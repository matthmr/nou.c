#!/usr/bin/env bash

case $1 in
	'-h'|'--help')
		printf 'Usage:       scripts/make-deps.sh
Description: Makes the dependencies for objects
Variables:   CC=[C compiler]
             M4=[m4-like command]
	     SED=[sed-like command]
Note:        Make sure to call this script from the repository root
'
		exit 1
		;;
esac

echo '[ .. ] Generating dependencies'

[[ -z $CC ]]  && CC=cc
[[ -z $M4 ]]  && M4=m4
[[ -z $SED ]] && SED=sed

echo "[ INFO ] CC = $CC"

# fix-up for local development branch: all test files have `-test' prefix
find . -name '*.c' -and -not -name '*-test*' -type f | xargs $CC -MM > make/Autodep.mk

# get object files
cat make/Autodep.mk | cut -d: -f1 | tr '\n' ' ' | $SED -e 's/^/OBJECTS:=&/g' -e 'a
'>> make/Autodep.mk

# set up future template for `make-makefile'
$SED -Ef make/sed/makefile.sed make/Autodep.mk \
     > make/Objects.m4

echo '[ .. ] Generating objects'
eval "$M4 $M4FLAGS make/Include.m4" | $SED 's/M4FLAG_include_[A-Za-z]*/ /g' \
    > make/Objects.mk
$M4 make/Targets.m4 > make/Targets.mk

echo '[ .. ] Generating targets'
cat make/Targets.mk | cut -d: -f1 | tr '\n' ' ' | $SED -e 's/^/TARGETS:=&/g' -e 'a 
'>> make/Targets.mk

echo '[ OK ] Done'
