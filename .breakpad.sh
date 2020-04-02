#!/bin/sh
SRC="${1:-.}"
OPT="${2:-}"
echo "${SRC}/configure --disabled-shared $OPT"
${SRC}/configure --disable-shared $OPT || exit 1
make || exit 1

