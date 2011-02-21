#!/bin/bash
if [ $# -lt 1 ]; then
	echo "Usage: $0 program"
	exit 1
fi

make
make -C tests/
rm store.file
util/tfstool create store.file 2048 disk1

MAINFILE=$1

while (( "$#" )); do
  util/tfstool write store.file tests/$1 $1
  shift
done

yams buenos initprog=[disk1]$MAINFILE
