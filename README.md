# Metoxid
Metoxid - mediafile metadata editor, written in C++

# Dependencies
- ncurses
- Exiv2

# Building on Windows
## Install MSYS2
For compiling metoxid you need to install MSYS2 first: https://www.msys2.org/

## Install packages
```bash
pacman -S cmake make mingw-w64-x86_64-ncurses ncurses-devel
```

## Update MSYS2 `PATH` variable
You have to update `PATH` variable to make CMake able to locate MinGW compiler
```bash
export PATH="/mingw64/bin:$PATH"
```
