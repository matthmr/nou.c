include(make/m4/defaults.m4)dnl
dnl
define(`header', `-DHEADER=$(HEADER)')dnl
define(`file', `define(`M4FLAG_include_$1', `$2')')dnl
dnl
file(`nou', header)dnl
undefine(`file')dnl
undefine(`header')dnl
include(make/Objects.m4)
