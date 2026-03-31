from sys import exit, argv
from select import select
from time import sleep
from os import get_terminal_size, read, write, fsync
from signal import signal, SIGINT, SIGTERM
from termios import tcgetattr, tcsetattr, TCSADRAIN
from tty import setcbreak
from re import compile as compile_re
from hashlib import sha1
from traceback import format_exc
from string import hexdigits

import sys

REGEX_ANSI = compile_re(r'(?:[@-Z\\-_]|[\x80-\x9A\x9C-\x9F]|(?:\x1B\[|\x9B)[0-?]*[ -/]*[@-~])')

class InterpreterMessage(object):
    def __init__(self, file = "<string>", line = 0, col = 0, message = "test message", code = None, highlight = None):
        self.file = file
        self.line = line
        self.col = col
        self.message = message
        self.highligt = highlight
        self.code = code

        self.prefix = "info"
        self.color = "\033[1;36m"
        self.color_cancel = "\033[0m"
        self.highligt_style = ("^", "~")

    def __str__(self):
        generated_code = self.code
        generated_highligt = " " * (len(generated_code) if self.code else 0)

        if (self.highligt):
            for item in self.highligt:
                generated_code = generated_code[:item[0]] + self.color + generated_code[item[0]:item[1]] + self.color_cancel + generated_code[item[1]:]
                generated_highligt = generated_highligt[:item[0]] + self.color + self.highligt_style[0] + self.highligt_style[-1] * (len(generated_highligt[item[0]:item[1]]) - 1) + self.color_cancel + generated_highligt[item[1]:]
        return (
            f"{self.file}:{self.line}:{self.col}: {self.color}{self.prefix}{self.color_cancel}: {self.message}" +
            ('' if self.code is None else f"\n    {self.line} | {generated_code}") +
            ('' if self.highligt is None or self.code is None else f"\n    {' ' * len(str(self.line))} | {generated_highligt}")
        )

    def __repr__(self):
        return (self.__str__())

class InterpreterError(InterpreterMessage):
    def __init__(self, file="<string>", line=0, col=0, message="test message", code=None, highlight=None):
        super().__init__(file, line, col, message, code, highlight)

        self.prefix = "error"
        self.color = "\033[1;31m"
        self.color_cancel = "\033[0m"

class InterpreterWarning(InterpreterMessage):
    def __init__(self, file="<string>", line=0, col=0, message="test message", code=None, highlight=None):
        super().__init__(file, line, col, message, code, highlight)

        self.prefix = "warning"
        self.color = "\033[1;33m"
        self.color_cancel = "\033[0m"

class Token(object):
    def __init__(self):
        self.entries = []
        self.name = "token"

class TokenLoop(Token):
    def __init__(self, looping_over: str = None):
        super().__init__()

        self.looping_over: str = looping_over

    def __str__(self):
        return (f"<TLoop refered_by={self.looping_over} content={self.entries}>")

    def __repr__(self):
        return (self.__str__())

class TokenRef(Token):
    def __init__(self, refered_by: str):
        super().__init__()

        self.refered: str = refered_by

    def __str__(self):
        return (f"<TRef refered_by={self.refered} content={self.entries}>")

    def __repr__(self):
        return (self.__str__())

class TokenRelRef(Token):
    def __init__(self, refered_by: str, off_ref: str):
        super().__init__()

        self.refered: str = refered_by
        self.off_ref: str = off_ref

    def __str__(self):
        return (f"<TRef refered_by={self.refered}&{self.off_ref} content={self.entries}>")

    def __repr__(self):
        return (self.__str__())

class TokenEntry(Token):
    def __init__(self, size: str, name: str, qual: str):
        super().__init__()

        self.size: int = int(eval(size, {}, {})) if not size.startswith("DYN_") else size
        self.name: str = str(name)
        self.qual: str = str(qual)

    def __str__(self):
        return (f"<TEntry size={self.size} name={self.name} ref={self.qual}>")

    def __repr__(self):
        return (self.__str__())

class TokenEnum(Token):
    def __init__(self, attached_to):
        super().__init__()

        self.attached_to = attached_to

class TokenColor(Token):
    pass

class PatternEntry(object):
    def __init__(self, size: int, name: str, value: bytes, ref = None, offset = None):
        self.value = value
        self.size = size
        self.name = name
        self.ref = ref
        self.offset = offset

    def __str__(self):
        return (f"${hex(self.offset).split('x')[1]} {''.join(map(lambda x: hex(x).split('x')[1].zfill(2), self.value))} | {self.name}")

    def __repr__(self):
        return (self.__str__())

class PatternFormat(object):
    def __init__(self, file_path):
        self._fp = open(file_path, 'r')

        self.entries = []
        self.messages = []
        self.tokens = self._parse()
        self.lexed_line = ""
        self.line = 0
        self.col = 0

    def _build_token(self, string, root = None):
        string = string.strip()
        token = string.split(' ')[0]

        if (token == 'ref_rel'):
            value = tuple(map(lambda x: x.strip(), 'ref_rel'.join(string.split('ref_rel')[1:]).strip().split(' ')))
            if (len(value) != 2):
                self.messages.append(InterpreterError(self._fp.name, self.line, self.col, f"Invalid argument count for ref_rel keyword, got: `{len(value)}` but excpected 2.", code=self.lexed_line, highlight = ((self.lexed_line.find("ref_rel"), len(self.lexed_line)),)))
                return TokenRelRef(None, None)
            base, rel = value
            rel_found = PatternFormat._find_token(lambda x: x.name == rel and hasattr(x, "qual") and x.qual == "REF", root)
            base_found = PatternFormat._find_token(lambda x: x.name == base and hasattr(x, "qual") and x.qual == "REF", root)

            if (not rel_found):
                self.messages.append(InterpreterError(self._fp.name, self.line, self.col, f"Relative reference not found: `{rel}`.", code=self.lexed_line, highlight = ((self.lexed_line.rfind(rel), self.lexed_line.rfind(rel) + len(rel)),)))
            if (not base_found):
                self.messages.append(InterpreterError(self._fp.name, self.line, self.col, f"Relative reference base not found: `{base}`.", code=self.lexed_line, highlight = ((self.lexed_line.find(base), self.lexed_line.find(base) + len(base)),)))
            return (TokenRelRef(rel_found, base_found))

        if (token == 'ref'):
            ref_name = 'ref'.join(string.split('ref')[1:]).strip()
            if (not ref_name):
                self.messages.append(InterpreterError(self._fp.name, self.line, self.col, f"Invalid argument count for ref keyword, got: `0` but excpected 1.", code=self.lexed_line, highlight = ((self.lexed_line.find("ref"), len(self.lexed_line)),)))
                return TokenRef(None)
            found = PatternFormat._find_token(lambda x: x.name == ref_name and hasattr(x, "qual") and x.qual == "REF", root)
            if (not found):
                self.messages.append(InterpreterError(self._fp.name, self.line, self.col, f"Reference not found: `{ref_name}`.", code=self.lexed_line, highlight = ((self.lexed_line.find(ref_name), self.lexed_line.find(ref_name) + len(ref_name)),)))
            return (TokenRef(found))

        if (token == 'loop'):
            if (not string[4:].strip()):
                self.messages.append(InterpreterWarning(self._fp.name, self.line, self.col, f"Infinite loop defined, this loop will run forever until it reach file EOF.", code=self.lexed_line, highlight = ((self.lexed_line.find(token), self.lexed_line.find(token) + len(token)),)))
            return (TokenLoop(None if (not string[4:].strip()) else string[4:].strip()))

        if (token == 'enum'):
            enum_name = 'enum'.join(string.split('enum')[1:]).strip()
            found = PatternFormat._find_token(lambda x: x.name == enum_name, root)
            if (not found):
                self.messages.append(InterpreterWarning(self._fp.name, self.line, self.col, f"Defined enum for a block that don't exist `{enum_name}`.", code=self.lexed_line, highlight = ((self.lexed_line.find(enum_name), self.lexed_line.find(enum_name) + len(enum_name)),)))

            return (TokenEnum(enum_name))

        if (token == 'colors'):
            return (TokenColor())

        string = tuple(filter(lambda x: x, map(lambda x: x.strip(), string.split(' '))))
        if (len(string) > 1 and len(string) < 4):
            if (string[0].startswith("DYN_") and not PatternFormat._find_token(lambda x: x.name == 'DYN_'.join(string[0].split('DYN_')[1:]) and hasattr(x, "qual") and x.qual == "LOC", root)):
                self.messages.append(InterpreterError(self._fp.name, self.line, self.col, f"DYN Size relation not found: `{string[0]}` for `{string[1]}`.", code=self.lexed_line, highlight = ((self.lexed_line.find(string[0]), self.lexed_line.find(string[0]) + len(string[0])),)))
            try:
                return TokenEntry(string[0], string[1], string[2] if len(string) > 2 else None)
            except Exception:
                self.messages.append(InterpreterError(self._fp.name, self.line, self.col, f"Invalid size: `{string[0]}` for `{string[1]}`.", code=self.lexed_line, highlight = ((self.lexed_line.find(string[0]), self.lexed_line.find(string[0]) + len(string[0])),)))
                return TokenEntry("0", string[1], string[2] if len(string) > 2 else None)

        self.messages.append(InterpreterError(self._fp.name, self.line, self.col, f"Invalid token: `{token}`.", code=self.lexed_line, highlight = ((self.lexed_line.find(token), self.lexed_line.find(token) + len(token)),)))
        return (Token())

    @staticmethod
    def _find_token(func, token_list, /, depht = 128):
        if (depht == -1):
            return (None)

        for item in token_list:
            if (not isinstance(item, Token)):
                continue
            if (func(item)):
                return (item)
            value = PatternFormat._find_token(func, item.entries, depht=depht-1)
            if (value):
                return (value)
        return (None)

    def _parse(self):
        value: str = self._fp.readline()
        self.lexed_line: str = value
        self.line: int = 1
        tokens: list = []

        while (value):
            self.col = len(value) - len(value.rstrip())
            value = value.split('#')[0].strip()

            if (value.endswith('\r\n')):
                value = value[:-2].strip()
            if (value.endswith('\n')):
                value = value[:-1].strip()

            self.lexed_line: str = value
            parent = tokens
            parent_type = None

            if (not value):
                value: str = self._fp.readline()
                continue

            depht: int = 0
            while (value.startswith('-')):
                value = value[1:].strip()
                if (parent and hasattr(parent[-1], "entries")):
                    parent_type = parent[-1].__class__
                    parent = parent[-1].entries
                    depht += 1
                else:
                    self.messages.append(InterpreterError(
                        self._fp.name, self.line, self.col,
                        f"Invalid tree depht, max depht hit `{depht}` inside token `{str(parent_type)}`.",
                        code=self.lexed_line, highlight = ((0, 0 + (len(self.lexed_line) - len(value))),)
                    ))

            if (value.startswith('/')):
                value = value[1:].strip()

            if (parent_type != TokenEnum and parent_type != TokenColor):
                parent.append(self._build_token(value, tokens))
            else:
                parent.append(tuple(map(lambda x: x.strip(), value.split(' '))))

            value: str = self._fp.readline()
            self.line += 1

        return (tokens)

    def close(self):
        if (not self._fp.closed):
            self._fp.close()

    def __del__(self):
        self.close()

class BinaryReader(object):
    def __init__(self, file_path, pattern: PatternFormat):
        self._fp = open(file_path, 'rb')
        self._fp.seek(0, 2)
        self.eof = self._fp.tell()
        self._fp.seek(0)

        self._pattern = pattern

        self._parse()

    def _read(self, size):
        position = self._fp.tell()
        if (position >= self.eof):
            return (b'\0' * size)
        # if (position + size >= self.eof):
        #     return self._fp.read(size - ((position + size) - self.eof)) + (b"\0" * ((position + size) - self.eof))
        return read(self._fp.fileno(), size)

    def _parse(self):
        offset = 0
        stack = [*tuple(reversed(self._pattern.tokens))]
        refs_use = {}
        refs = {}
        locs_use = {}
        locs = {}

        while (stack):
            item = stack.pop()
            if (isinstance(item, TokenEntry)):
                if (isinstance(item.size, int)):
                    self._pattern.entries.append(PatternEntry(item.size, item.name, self._read(item.size), None, offset))
                    if (item.qual == "REF"):
                        if (not item.name in refs):
                            refs_use[item.name] = 0
                            refs[item.name] = []
                        refs[item.name].append(int.from_bytes(self._pattern.entries[-1].value))
                    if (item.qual == "LOC"):
                        if (item.name not in locs):
                            locs_use[item.name] = 0
                            locs[item.name] = []
                        locs[item.name].append(int.from_bytes(self._pattern.entries[-1].value))
                    offset += item.size
                else:
                    self._pattern.entries.append(PatternEntry(locs['DYN_'.join(item.size.split('DYN_')[1:])][locs_use['DYN_'.join(item.size.split('DYN_')[1:])]], item.name, self._read(locs['DYN_'.join(item.size.split('DYN_')[1:])][locs_use['DYN_'.join(item.size.split('DYN_')[1:])]]), None, offset))
                    locs_use['DYN_'.join(item.size.split('DYN_')[1:])] += 1
                    if (item.qual == "REF"):
                        if (not item.name in refs):
                            refs[item.name] = []
                        refs[item.name].append(int.from_bytes(self._pattern.entries[-1].value))
                    if (item.qual == "LOC"):
                        if (item.name not in locs):
                            locs[item.name] = []
                        locs[item.name].append(int.from_bytes(self._pattern.entries[-1].value))
                    offset += len(self._pattern.entries[-1].value)
            if (isinstance(item, TokenLoop)):
                if (item.looping_over is None):
                    if (self.eof != self._fp.tell()):
                        stack.extend(tuple(reversed(item.entries)))
                        stack.insert(0, item)
                else:
                    for _ in range(locs[item.looping_over][locs_use[item.looping_over]]):
                        stack.extend(tuple(reversed(item.entries)))
            if (isinstance(item, TokenRelRef)):
                self._fp.seek(refs[item.off_ref.name][0] + refs[item.refered.name][refs_use[item.refered.name] if refs_use[item.refered.name] < len(refs[item.refered.name]) else -1])
                stack.extend(tuple(reversed(item.entries)))
                offset = refs[item.off_ref.name][0] + refs[item.refered.name][refs_use[item.refered.name] if refs_use[item.refered.name] < len(refs[item.refered.name]) else -1]
                refs_use[item.refered.name] += 1
            if (isinstance(item, TokenRef)):
                self._fp.seek(refs[item.refered.name][refs_use[item.refered.name] if refs_use[item.refered.name] < len(refs[item.refered.name]) else -1])
                stack.extend(tuple(reversed(item.entries)))
                offset = refs[item.refered.name][refs_use[item.refered.name] if refs_use[item.refered.name] < len(refs[item.refered.name]) else -1]
                refs_use[item.refered.name] += 1

    def close(self):
        if (not self._fp.closed):
            self._fp.close()

    def __del__(self):
        self.close()

class ByteLookup(object):
    def __init__(self, pattern):
        self._pattern = pattern

        self.colors = {}
        self.enums = {}

        self._parse()

        sys.stdout.write("\033[?1049h\033[?25l\033[H")
        sys.stdout.flush()

        self.pos = 0

        temp = get_terminal_size(0)
        self.size = (temp.columns, temp.lines)

        signal(SIGINT, lambda *args, **kwargs: self.close())
        signal(SIGTERM, lambda *args, **kwargs: self.close())

        self.old_settings = tcgetattr(0)
        setcbreak(0)

        self.running = True

        self.command_mode = False
        self.command = ""

        self.buffer = bytearray()
        self.no_color = False

        self.last_written = None
        self.closed = False

        self.output = None

    def _parse(self):
        for item in self._pattern.tokens:
            if (isinstance(item, TokenColor)):
                for it in item.entries:
                    self.colors[it[0]] = eval(it[1], {}, {})

            if (isinstance(item, TokenEnum)):
                self.enums[item.attached_to] = {eval(it[0], {}, {}): it[1] for it in item.entries}

    def event_navigation_mode(self, values):
        if (values == b"\033[A"):
            if (self.pos - 16 > 0):
                self.pos -= 16
            else:
                self.pos = 0

        elif (values == b"\033[B"):
            max_val = sum(max(map(lambda x: (x.offset, x.size), self._pattern.entries), key=lambda x: x[0]))
            if (self.pos + 16 < max_val - 1):
                self.pos += 16
            else:
                self.pos = max_val - 1

        elif (values == b"\033[D"):
            if (self.pos - 1 > 0):
                self.pos -= 1
            else:
                self.pos = 0

        elif (values == b"\033[C"):
            max_val = sum(max(map(lambda x: (x.offset, x.size), self._pattern.entries), key=lambda x: x[0]))
            if (self.pos + 1 < max_val - 1):
                self.pos += 1
            else:
                self.pos = max_val - 1

        elif (b':' in values):
            self.command_mode = True

        else:
            if (b'c' in values):
                self.no_color = not self.no_color

    def handle_command(self):
        if (not self.command.strip()):
            return

        command, *args = tuple(filter(lambda x: x, map(lambda x: x.strip(), self.command.strip().split(' '))))

        if (command == "pos"):
            if (len(args) < 1):
                self.output = "pos: missing argument 'pos_addr'."
                return
            if (not args[0].isnumeric() and not all(map(lambda x: x in hexdigits, args[0].lower().split('x')[-1]))):
                self.output = "pos: argument 1 should be an int/(hex 0x format)."
                return
            if (args[0].isnumeric()):
                self.pos = int(args[0])
            else:
                self.pos = int(args[0].lower().split('x')[-1], 16)
            self.output = f"moved to {self.pos}."
            return

        if (command == "blk"):
            if (len(args) < 1):
                self.output = "blk: missing argument 'blk_name'."
                return
            for item in self._pattern.entries:
                if (item.name == args[0] and item.offset > self.pos):
                    self.pos = item.offset
                    self.output = f"moved to {item.offset}."
                    return

            for item in self._pattern.entries:
                if (item.name == args[0]):
                    self.pos = item.offset
                    self.output = f"moved to {item.offset}."
                    return
            self.output = f"block '{args[0]}' not found."
            return

        if (command == "eval"):
            if (len(args) < 1):
                self.output = "eval: missing arguments 'expression'."
                return
            try:
                self.output = f"eval: {eval(' '.join(args), {}, {})}"
            except Exception as e:
                self.output = f"eval: error: {e}"
            return

        self.output = f"command not found: '{command}'."

    def event_command_mode(self, values):
        if (values.startswith(b'\033[')):
            return

        for item in values.decode():
            if (item == "\033"):
                self.command_mode = False
                return

            if (ord(item) == 127):
                self.command = self.command[:-1]

            if (ord(item) == 10):
                self.handle_command()
                self.command = ""
                self.command_mode = False

            if (item.isprintable()):
                self.command += item

    def event(self):
        if (not sys.stdin.isatty()):
            return

        if (select([sys.stdin.fileno()], [], [], 0) != ([sys.stdin.fileno()], [], [])):
            return

        values = read(sys.stdin.fileno(), 3)

        if (not self.command_mode):
            self.event_navigation_mode(values)
        else:
            self.event_command_mode(values)

    def draw(self):
        temp = get_terminal_size(0)
        changed = False

        self.buffer = bytearray()

        if ((temp.columns, temp.lines) != self.size):
            self.size = (temp.columns, temp.lines)
            changed = True

        off = 0 if ((self.pos / 16.0) < 9) else (self.pos - (9 * 16) - (self.pos % 16))
        base = off

        self.buffer.extend(b"\033[H\033[0J")

        sorted_entries = tuple(sorted(self._pattern.entries, key=lambda x: x.offset))
        to_string_entries = bytearray()
        last_color = None
        selected = (None, 0)
        while off < sum(max(map(lambda x: (x.offset, x.size), self._pattern.entries), key=lambda x: x[0])):
            sorted_entries = tuple(filter(lambda x: x.offset + x.size - 1 >= off, sorted_entries))
            if (off != base and not (off % 4)):
                self.buffer.extend(b' ')
            if (off != base and not (off % 16)):
                self.buffer.extend(b"\033[0m")
                to_string_entries.extend(b"\033[0m")
                self.buffer.extend("│ ".encode())
                self.buffer.extend(to_string_entries)
                to_string_entries.clear()
                self.buffer.extend(b"\033[1B\r")
                if ((off - base) / 16.0 > 30):
                    break
            if ((off % 16) == 0):
                if (last_color and (not self.no_color)):
                    to_string_entries.extend(last_color.encode())
                self.buffer.extend(f"${hex(off).split('x')[1].zfill(8)} {last_color if last_color and (not self.no_color) else ''}".encode())
            if (off == self.pos):
                if (not self.command_mode):
                    self.buffer.extend(b"\033[7m")
                    to_string_entries.extend(b"\033[7m")
                selected = (None if off < sorted_entries[0].offset else sorted_entries[0], off)
            if (off < sorted_entries[0].offset or len(sorted_entries[0].value) <= off - sorted_entries[0].offset):
                if (not self.no_color):
                    self.buffer.extend(b"\033[38;2;79;79;79m??\033[0m")
                    to_string_entries.extend(b"\033[38;2;79;79;79m?\033[0m")
                else:
                    self.buffer.extend(b"??")
                    to_string_entries.extend(b"?")

                if (off == self.pos and not self.command_mode):
                    self.buffer.extend(b"\033[27m")
                    to_string_entries.extend(b"\033[27m")
                self.buffer.extend(b" ")
                off += 1
                last_color = None
                continue

            if ((sorted_entries[0].offset == off or off == base) and sorted_entries[0].name in self.colors):
                color = self.colors[sorted_entries[0].name]
                b,g,r = (color & 0x0000ff), (color & 0x00ff00) >> 8, (color & 0xff0000) >> 16
                last_color = f"\033[38;2;{r};{g};{b}m"
                if (not self.no_color):
                    to_string_entries.extend(last_color.encode())
                    self.buffer.extend(last_color.encode())

            self.buffer.extend(hex(sorted_entries[0].value[off - sorted_entries[0].offset]).split('x')[1].zfill(2).encode())
            to_string_entries.extend((chr(sorted_entries[0].value[off - sorted_entries[0].offset]) if chr(sorted_entries[0].value[off - sorted_entries[0].offset]).isprintable() else ".").encode())
            if (off == self.pos):
                self.buffer.extend(b"\033[27m")
                to_string_entries.extend(b"\033[27m")
            if (off == sorted_entries[0].offset + sorted_entries[0].size - 1):
                last_color = None
                to_string_entries.extend(b"\033[0m")
                self.buffer.extend(b"\033[0m")
            self.buffer.extend(b' ')
            off += 1
        sorted_entries = tuple(filter(lambda x: x.offset + x.size - 1 >= off, sorted_entries))

        if (not sorted_entries):
            self.buffer.extend(b"\033[62G")
            self.buffer.extend(" │ ".encode())
            self.buffer.extend(to_string_entries)

        self.buffer.extend(b"\033[0;82H")
        self.buffer.extend(("│\033[1B\b" * ((off - base) // 16 + (0 if sorted_entries or not (off - base) % 16 else 1))).encode())

        moved: int = 0

        self.buffer.extend(b"\033[0;84H")
        if (selected[0] and len(selected[0].value) > (self.pos - selected[0].offset)):
            self.buffer.extend(f"area name: {selected[0].name}\033[2;84H".encode())
            self.buffer.extend(f"size: {selected[0].size}\033[3;84H".encode())
            self.buffer.extend(f"offset: {selected[0].offset}\033[4;84H".encode())
            self.buffer.extend(f"hex offset: {hex(selected[0].offset).split('x')[1].zfill(8)}\033[5;84H".encode())
            self.buffer.extend(f"value: {'...' if len(selected[0].value) > 36 else str(int.from_bytes(selected[0].value))[:35]}\033[6;84H".encode())
            if (selected[0].name in self.enums):
                self.buffer.extend(f"enum entry: {self.enums[selected[0].name].get(int.from_bytes(selected[0].value), "UKNOWN") if len(selected[0].value) <= 36  else "DATA TOO LARGE"}\033[7;84H".encode())
                moved += 1
            self.buffer.extend(f"byte pos: {self.pos}\033[{7 + moved};84H".encode())
            self.buffer.extend(f"hex byte pos: {hex(self.pos).split('x')[1].zfill(8)}\033[{8 + moved};84H".encode())
            self.buffer.extend(f"byte value: {selected[0].value[self.pos - selected[0].offset]}".encode())
        else:
            self.buffer.extend(b"Unmapped area, no data found.")
            self.buffer.extend(b"\033[2;84H")
            self.buffer.extend(f"byte pos: {self.pos}".encode())
            self.buffer.extend(b"\033[3;84H")
            self.buffer.extend(f"hex byte pos: {hex(self.pos).split('x')[1].zfill(8)}".encode())

        # if (self.no_color):
        #     self.buffer = REGEX_ANSI.sub('', self.buffer.decode()).encode()

        if (self.command_mode):
            self.buffer.extend(b"\033[8;11H")
            self.buffer.extend(("╔" + ("═" * 48) + "╗").encode())
            self.buffer.extend(b"\033[9;11H")
            self.buffer.extend(("║" + self.command + (" " * (48 - len(self.command))) + "║").encode())
            self.buffer.extend(b"\033[10;11H")
            self.buffer.extend(("╚" + ("═" * 48) + "╝").encode())

        if (self.output):
            self.buffer.extend(f"\033[{(off - base) // 16 + (1 if sorted_entries else 2)};0H".encode())
            self.buffer.extend(self.output.encode())

        last_written = sha1(self.buffer, usedforsecurity=False).hexdigest()

        if (last_written != self.last_written or changed):
            self.last_written = last_written
            write(sys.stdout.fileno(), self.buffer)
            sys.stdout.flush()

        sleep(0.01)

    def __del__(self):
        self.close()

    def close(self):
        self.running = False

        if (not self.closed):
            self._pattern.close()

            sys.stdout.write("\033[?25h\033[H\033[?1049l")
            sys.stdout.flush()

            tcsetattr(0, TCSADRAIN, self.old_settings)
            self.closed = True

def main() -> int:
    has_error: bool = False

    if (len(argv) < 2):
        sys.stderr.write("No binary file specified.\n")
        return (1)
    if (len(argv) < 3):
        sys.stderr.write("No pattern file specified.\n")
        return (1)

    pf = PatternFormat(argv[2])

    for item in pf.messages:
        if (isinstance(item, InterpreterError)):
            has_error = True
        print(item)

    if (has_error):
        return (1)

    br = BinaryReader(argv[1], pf)
    br.close()

    if (not sys.stdout.isatty()):
        for item in pf.entries:
            print(item)
        pf.close()
        return (0)

    bl = ByteLookup(pf)

    try:
        while bl.running:
            bl.event()
            bl.draw()
    except Exception as e:
        pf.close()
        bl.close()
        sys.stderr.write(f"error: {format_exc()}.\n")
        return (1)

    return (0)

if (__name__ == "__main__"):
    exit(main())
