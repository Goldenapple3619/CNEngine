<a href="https://github.com/Goldenapple3619/CNEngine">

<picture>
  <source media="(prefers-color-scheme: dark)" srcset="./assets/images/banner_dark_XL.png">
  <img alt="CNEngine" src="./assets/images/banner_XL.png">
</picture>

</a>


<div align="center">
  
[![GitHub contributors](https://img.shields.io/github/contributors/Goldenapple3619/CNEngine?style=for-the-badge)](https://github.com/Goldenapple3619/CNEngine/graphs/contributors) ![GitHub repo size](https://img.shields.io/github/repo-size/Goldenapple3619/CNEngine?style=for-the-badge) [![GitHub License](https://img.shields.io/github/license/Goldenapple3619/CNEngine?style=for-the-badge)](https://github.com/Goldenapple3619/CNEngine/blob/dev/LICENSE) [![GitHub Actions Status](https://img.shields.io/github/actions/workflow/status/Goldenapple3619/CNEngine/build.yml?style=for-the-badge)](https://github.com/Goldenapple3619/CNEngine/actions/workflows/build.yml) [![GitHub Actions Status](https://img.shields.io/github/actions/workflow/status/Goldenapple3619/CNEngine/documentation.yml?style=for-the-badge&label=Documentation)](https://github.com/Goldenapple3619/CNEngine/actions/workflows/documentation.yml)

</div>

## Table of content
<ol>
  <li><a href="#overview">Overview</a></li>
  <li><a href="#built-with">Built With</a></li>
  <li><a href="#projects-using-cnengine">Projects using CNEngine</a></li>
  <li><a href="#prerequisites">Prerequisites</a></li>
  <li>
    <a href="#how-to-build">How to build</a>
    <ul>
      <li><a href="#windows-using-msys">Windows using MSYS</a></li>
      <li><a href="#windows-using-wsl">Windows using WSL</a></li>
      <li><a href="#linux">Linux</a></li>
      <li><a href="#macos">MacOS</a></li>
    </ul>
  </li>
  <li><a href="#contributors">Contributors</a></li>
  <li><a href="#license">License</a></li>
  <li><a href="#faq">FAQ</a></li>
  <li><a href="#contact">Contact</a></li>
</ol>

## Overview
This project is a complete game engine programmed in C that allow 2d rendering, 3d rendering, GUI rendering.

Higly configurable on the back technology allow the usage of multiple graphics api (via SDL surface, SDL renderer, OpenGL context, ...) and to render using CPU or GPU.

## Built with
<div align="center">
  
![Static Badge](https://img.shields.io/badge/The%20C%20language-black?style=for-the-badge&logo=C)
![Static Badge](https://img.shields.io/badge/Python-black?style=for-the-badge&logo=python)
![Static Badge](https://img.shields.io/badge/SDL2-black?style=for-the-badge&logo=SDL)
![Static Badge](https://img.shields.io/badge/OpenGL-black?style=for-the-badge&logo=opengl)
![Static Badge](https://img.shields.io/badge/CMake-black?style=for-the-badge&logo=cmake)
![Static Badge](https://img.shields.io/badge/GCC-black?style=for-the-badge&logo=gnu)
  
</div>

## Projects using CNEngine
|Name|Description|By|Website|
|---|---|---|---|
|**Alice - Gunz & Arrests**|*Still in developement*|**CNStudio™**|*Not avaiable until end of developement of the game and the engine itself*|

## Prerequisites
- CMake
- Make
- GCC (mingw for Windows)
- LIBC
- LIBM
- python-jinja (jinja2)

## How to build

*Before installing make sure you have every prerequisites installed (libm, libc are most of the time install with the mingw / gcc package).*

### Windows using MSYS
```shell
  git clone https://github.com/Goldenapple3619/CNEngine.git && cd CNEngine
  
  # if you're executing outside of MSYS (make sure binaries are exposed in $PATH):
  cmd.exe /c scripts\build_windows-${ARCH}.bat

  # or if you're inside of MSYS
  sh scripts/build_windows-${ARCH}.sh
```

### Windows using WSL
```shell
  git clone https://github.com/Goldenapple3619/CNEngine.git && cd CNEngine

  sh scripts/build_wsl-${ARCH}.sh
```

### Linux
```shell
  git clone https://github.com/Goldenapple3619/CNEngine.git && cd CNEngine

  sh scripts/build_linux-${ARCH}.sh
```

### MacOS
```shell
  git clone https://github.com/Goldenapple3619/CNEngine.git && cd CNEngine

  sh scripts/build_macos-${ARCH}.sh
```

## Documentation
Code documentation available via the doxygen at [https://Goldenapple3619.github.io/CNEngine](https://Goldenapple3619.github.io/CNEngine/) or by compiling the doc with:
```shell
  doxygen doc/doxygen/doxydoc
```

## Contributors
<a href="https://github.com/Goldenapple3619/CNEngine/graphs/contributors">
  <img src="https://contrib.rocks/image?repo=Goldenapple3619/CNEngine" alt="contrib.rocks image" />
</a>

## License
See [LICENSE](https://github.com/Goldenapple3619/CNEngine/blob/dev/LICENSE) for more information.

## FAQ
- **Can I use this engine to release commercial products?**
> *No.*

- **Can I experiment using the engine and build personal project with it?**
> *Yes, but credit the engine and keep the license.*

- **Can I read the code to inspire when building my own engine?**
> *Yes, but don't do a copy/paste of the codebase itself.*

- **Can I contribute to this repo by doing a PR?**
> *If you're reading the FAQ section, there's 99.99% of chance that the anwser is no, but you can still give suggestion / report bugs.*

## Contact
For more infos, join the [Discord server](https://discord.gg/F34jV23hjd).
