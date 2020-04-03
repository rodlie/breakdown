# Breakdown

A library and example application for parsing Google Breakpad crash dumps on Windows (MinGW-only), macOS (untested) and Linux.

This library uses a [fork](https://github.com/NatronGitHub/breakpad) of Google Breakpad.

## Build and install
Requires autotools, pkg-config, CMake (3.0+), a c++11 compatible compiler (only GCC tested) and libzip.

```
git clone https://github.com/rodlie/breakdown
cd breakdown
git submodule update -i --recursive
mkdir build && cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr .. && make
make DESTDIR=<PACKAGE_LOCATION> install
```
```
<PACKAGE_LOCATION>
└── usr
    ├── include
    │   └── Breakdown
    │       └── breakdown.h
    ├── lib64
    │   ├── libBreakdown.so -> libBreakdown.so.1
    │   ├── libBreakdown.so.1 -> libBreakdown.so.1.0.0
    │   ├── libBreakdown.so.1.0.0
    │   └── pkgconfig
    │       └── breakdown.pc
    └── share
        └── doc
            └── Breakdown-1.0.0
                ├── LICENSE
                └── README.md
```

## Symbols Storage

Symbols are stored using the following directory structure:

* ``<SYMBOLS_FOLDER>``
  * ``<FILENAME_FOLDER>`` *(example: ``Natron-bin``)*
    * ``<SYMBOL_ID_FOLDER>`` *(example: ``69CDA01A0F236F7C71CD19E5A20A21AC0``)*
      * ``<ZIP_FILE>`` *(example: ``Natron-bin.sym.zip``)*

``<FILENAME_FOLDER>`` and ``<SYMBOL_ID_FOLDER>`` must match line 1 in the symbol file.

Example : ``MODULE Linux x86_64 69CDA01A0F236F7C71CD19E5A20A21AC0 Natron-bin``

**Note that Breakdown only supports zipped symbols (``filename.sym.zip``)**

## Creating Symbols

Example:
```

dump_syms Natron-bin > Natron-bin.sym
export SYMBOL_BIN=`head -1 Natron-bin.sym | awk '{print $5}'`
export SYMBOL_ID=`head -1 Natron-bin.sym | awk '{print $4}'`
mkdir -p symbols/$SYMBOL_BIN/$SYMBOL_ID
zip -9 Natron-bin.sym.zip Natron-bin.sym
mv Natron-bin.sym.zip symbols/$SYMBOL_BIN/$SYMBOL_ID/
```
```
symbols
└── Natron-bin
    └── 69CDA01A0F236F7C71CD19E5A20A21AC0
        └── Natron-bin.sym.zip

2 directories, 1 file
```

## Usage Example (library)

```
#include <breakdown.h>

std::string filename = "crash.dmp";
std::vector<std::string> storage;
storage.push_back("symbols");
std::string result = Breakdown::convertDumpToString(filename, storage);
```

## Usage Example (application)

```
stackwalker symbols crash.dmp > result.txt
```
```
OS       : Linux (0.0.0 Linux 4.4.172 #2 SMP Wed Jan 30 17:11:07 CST 2019 x86_64)
TYPE     : SIGSEGV /0x00000000

MODULE   : Natron-bin
FUNCTION : Natron::crash_application()
SOURCE   : Settings.cpp:2228

...
```
