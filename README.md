# Raylib-Inventory-System
A 2D inventory system built with raylib and C++. Features drag &amp; drop, item stacking, selling mechanics, tooltip system and lazy texture loading.

## Run on Windows

The committed `build/Adv_InventorySys.exe` is a 64-bit Windows executable. Keep the repository's `resources` folder alongside `build`; the game loads its images and sounds from there. You can launch the executable from any working directory.

The executable is Windows-only and is not a standalone file without the `resources` folder. Other operating systems need their own build.

## Build from source

Install CMake and a C++17 compiler, then run:

```sh
cmake -S . -B build
cmake --build build --config Release
```

CMake downloads raylib during configuration, so an internet connection is required the first time you build.
