# Breakdown

A library for parsing Google Breakpad crash dumps.

Requires CMake (3.0+), a c++11 compatible compiler (only GCC tested) and libzip.

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