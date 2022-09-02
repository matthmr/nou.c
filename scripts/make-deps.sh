#!/usr/bin/env bash

case $1 in
	'-h'|'--help')
		printf 'Usage:       scripts/make-dep.sh
Description: Makes the dependencies for objects
Variables:   CC=[C compiler]
             M4=[m4-like command]
Note:        Make sure to call this script from the repository root
'
		exit 1
		;;
esac

echo '[ .. ] Generating dependencies'

[[ -z $CC ]] && CC=cc
[[ -z $M4 ]] && M4=m4

echo "[ INFO ] CC = $CC"

# fix-up for local development branch: all test files have `-test' prefix
find . -name '*.c' -and -not -name '*-test*' -type f | xargs $CC -MM > make/Objects.mk

cat make/Objects.mk | cut -d: -f1 | tr '\n' ' ' | sed -e 's/^/OBJECTS:=&/g' -e 'a' >> make/Objects.mk

echo '[ .. ] Generating targets'
$M4 make/Targets.m4 > make/Targets.mk
cat make/Targets.mk | cut -d: -f1 | tr '\n' ' ' | sed -e 's/^/TARGETS:=&/g' -e 'a'>> make/Targets.mk

echo '[ OK ] Done'
