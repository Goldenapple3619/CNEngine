from sys import exit, argv
from xml.etree import ElementTree
from os.path import basename, dirname, join
import sys

ENDL: str = "\n"

class Source(object):
    SRC_C = 0x00
    SRC_CPP = 0x01

    def __init__(self, path, src_type):
        self.path = path
        self.type = src_type

    def type_to_str(self):
        if (self.type == self.SRC_C):
            return ("c")
        if (self.type == self.SRC_CPP):
            return ("cpp")
        return ("ukn")

    def __str__(self):
        return (f"<src type={self.type_to_str()} path={self.path}>")

    def __repr__(self):
        return (self.__str__())

class SourcesCompiler(object):
    def __init__(self, tree):
        self.tree = tree

        self.sources = []

    def __repr__(self):
        return (self.__str__())
    
    def __str__(self):
        return (f"""<Sources:{'|'.join(str(item) for item in self.sources)}>""")

    def _code_dat(self, elm):
        if (elm.tag == "c"):
            self.sources.append(Source(elm.text.strip(), Source.SRC_C))
            return
        if (elm.tag == "cpp"):
            self.sources.append(Source(elm.text.strip(), Source.SRC_CPP))
            return
        raise ValueError(f"invalid tag: '{elm.tag}' in '{self.tree.tag}'.")

    def load(self):
        if (self.tree.tag != "srcs"):
            raise ValueError("root should have tag 'srcs'")
        
        for item in self.tree:
            self._code_dat(item)

class StrEntry(object):
    def __init__(self, value: str, addr: int):
        self.addr: int = addr
        self.value: str = value

class Strndx(object):
    STR_LEN_SIZE = 0x04
    STR_ADDR_SIZE = 0x04

    def __init__(self, /, encoding: str = "utf8"):
        self._table: int = []
        self._encoding: str = encoding

    def find_str(self, string: str):
        if (isinstance(string, str)):
            string: bytes = string.encode(self._encoding)
        for item in self._table:
            if (item.value == string):
                return item
        return (None)

    def add_str(self, string: str) -> StrEntry:
        if (isinstance(string, str)):
            string: bytes = string.encode(self._encoding)
        entry: StrEntry = self.find_str(string)
        
        if (not entry):
            entry = StrEntry(string, (self._table[-1].addr + len(self._table[-1].value)) if len(self._table) else 0)
            self._table.append(entry)
        return (entry)

class DataSection(object):
    SECTION_TYPE_UKN = 0
    def __init__(self, name, section_type = SECTION_TYPE_UKN, section_flags = 0):
        self.name = name
        self.type = section_type
        self.flags = section_flags

    def get_size(self, str_table: Strndx) -> int:
        return (0)

    def get_bytes(self, str_table: Strndx) -> bytes:
        return (b'')

class ObjectCompiler(object):
    MAGIC = b"\x08\x75\xC4\xE3"
    FLAGS_SIZE = 0x04 
    ENDIAN_SIZE = 0x01

    ADDR_SIZE = 0x08

    SECTION_HEADER_SIZE_SIZE = 0x08
    SECTION_NUMBER_SIZE = 0x08
    SECTION_TYPE_SIZE = 0x04
    SECTION_FLAGS_SIZE = 0x04
    SECTION_SIZE_SIZE = 0x08
    def __init__(self, tree):
        self.tree = tree

        self.name = "unkown"
        self.base = self.tree.attrib.get("base", "__builtin_scene_object")
        self.sources = None
        self.methods = []

    def __repr__(self):
        return (self.__str__())

    def __str__(self):
        return f"""<Object name={self.name} base={self.base} srcs={self.sources}>"""

    def _object_dat(self, elm):
        if (elm.tag == "name"):
            self.name = elm.text.strip()
        if (elm.tag == "srcs"):
            self.sources = SourcesCompiler(elm)
        if (elm.tag == "methods"):
            for method in elm:
                if (method.tag != "symbol"):
                    raise ValueError(f"invalid method tag '{method.tag}'")
                self.methods.append((method.attrib["name"], method.text.strip()))
        if (elm.tag == "attrs"):
            pass # todo: implement attrs system

    def load(self):
        if (self.tree.tag != "object"):
            raise ValueError("root should have tag 'object'")
        
        for item in self.tree:
            self._object_dat(item)

        if (self.sources):
            self.sources.load()

    def generate_code(self):
        real_base = self.base.replace("__builtin_", "", 1) if \
            self.base.startswith("__builtin_") else "__generated_" + real_base
        with open(join(dirname(__file__), "patterns", "cbase_obj"), 'r') as fp:
            return (fp.read()
                .replace("${PARENT_BUILDER_SYMBOL}", "new_" + real_base + "()")
                .replace("${SYMBOL_NAME}", "__generated_new_" + self.name)
                .replace("${METHODS}", f"{ENDL}".join(f"    CREATE_METHOD_CLASS_BUILD(obj, \"{item[0]}\", &{item[1]});" for item in self.methods))
            )
        
    def generate_bytes(self, flags = 0, /, encoding = "utf8", endian = "big"):
        itoa = lambda i, sz: int.to_bytes(i, sz, byteorder=endian, signed=False)
        header: bytearray = bytearray()
        data_sections_header: bytearray = bytearray()

        _str_table = Strndx(encoding)
        _sections = []

        header_size = (
            len(self.MAGIC) +
            self.ENDIAN_SIZE +
            self.FLAGS_SIZE +
            self.ADDR_SIZE +
            self.ADDR_SIZE +
            _str_table.STR_ADDR_SIZE
        )
        
        section_header_size = (
            self.SECTION_HEADER_SIZE_SIZE +
            self.SECTION_NUMBER_SIZE +
            len(_sections) * (
                _str_table.STR_ADDR_SIZE +
                self.SECTION_TYPE_SIZE + 
                self.SECTION_FLAGS_SIZE +
                self.SECTION_SIZE_SIZE +
                self.ADDR_SIZE
            )
        )
        
        all_section_size = sum(map(lambda x: x.get_size(_str_table), _sections))
        
        # magic
        header.extend(self.MAGIC)

        # endianness
        header.extend(itoa(1 if endian == "little" else 0, self.ENDIAN_SIZE))

        # flags
        header.extend(itoa(header_size, self.FLAGS_SIZE))

        # data sections header position
        header.extend(itoa(header_size, self.ADDR_SIZE))

        # strndx position
        header.extend(itoa(header_size + section_header_size + all_section_size, self.ADDR_SIZE))
        
        # object name
        header.extend(itoa(_str_table.add_str(self.name).addr, _str_table.STR_ADDR_SIZE))


        # sh size
        data_sections_header.extend(itoa(section_header_size, self.SECTION_HEADER_SIZE_SIZE))

        # sh cnt
        data_sections_header.extend(itoa(len(_sections), self.SECTION_NUMBER_SIZE))

        offset = header_size + section_header_size

        # sh sections
        for item in _sections:
            # s name
            data_sections_header.extend(itoa(_str_table.add_str(item.name).addr, _str_table.STR_ADDR_SIZE))

            # s type
            data_sections_header.extend(itoa(item.type, self.SECTION_TYPE_SIZE))

            # s flags
            data_sections_header.extend(itoa(item.flags, self.SECTION_FLAGS_SIZE))

            # s size
            temp_sz = item.get_size(_str_table)
            data_sections_header.extend(itoa(temp_sz, self.SECTION_SIZE_SIZE))

            # s off
            data_sections_header.extend(itoa(offset, self.ADDR_SIZE))

            offset += temp_sz

        yield bytes(header)

        #todo: put alignement here

        yield bytes(data_sections_header)

        #todo: put alignement here

        offset = header_size + section_header_size

        for i, item in enumerate(_sections):
            if (i):
                #todo: put alignement here
                pass

            generated = item.to_bytes(_str_table)
    
            yield generated

            offset += len(generated)

        #todo: put alignement here

        for item in _str_table._table:
            yield itoa(len(item.value), _str_table.STR_LEN_SIZE)
            yield item.value

def main() -> int:
    if (len(argv) < 2):
        sys.stderr.write(f"{argv[0]}: no file given as input.{ENDL}")
        return (1)
    
    if (len(argv) < 3):
        output = "."
    else:
        output = argv[2]
    
    try:
        tree = ElementTree.parse(argv[1])
    except Exception as e:
        sys.stderr.write(f"{argv[0]}: {argv[1]}: failed to parse. ({e}){ENDL}")
        return (1)

    root = tree.getroot()

    oc  = ObjectCompiler(root)
    try:
        oc.load()
    except Exception as e:
        sys.stderr.write(f"{argv[0]}: {argv[1]}: failed to load. ({e}){ENDL}")
        return (1)
    
    try:
        with open(join(output, f"__generated_{oc.name}.c"), "w+") as fp:
            fp.write(oc.generate_code())
        with open(join(output, f"__generated_{oc.name}.cobj"), 'wb+') as fp:
            fp.write(b''.join(oc.generate_bytes()))
    except Exception as e:
        sys.stderr.write(f"{argv[0]}: {argv[1]}: failed to generate. ({e}){ENDL}")
        return (1)


    return (0)

if (__name__ == "__main__"):
    exit(main())
