#!/bin/sh
set -e

PARSER="${PARSER:-`pwd`/../build/breakdown-parser}"

DUMPS="report-Linux-2f5d2700-d191-9876-430fd7b8-3825119e report-Windows-44de0072-5ed8-49aa-9fd1-5f24b1c76acb"

for i in $DUMPS;do
  echo "Testing $i"
  rm -f $i.tmp || true
  $PARSER symbols $i.dmp > $i.tmp
  DIFF=`diff $i.txt $i.tmp`
  if [ ! -z "$DIFF" ]; then
    echo "$i: FAILED"
    echo "$DIFF"
    exit 1
  else
    echo "$i: PASSED"
  fi
  rm -f $i.tmp || true
done
