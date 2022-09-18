include(make/m4/defaults.m4)dnl
dnl
define(`header', `-DHEADER=$(HEADER)')dnl
define(`debug', `-DDEBUG=$(DEBUG)')dnl
define(`file', `define(`M4FLAG_include_$1', `$2')')dnl
dnl
file(`deck', debug)dnl
file(`draw', debug)dnl
file(`cmd', debug)dnl
file(`players', debug)dnl
file(`nou', header debug)dnl
undefine(`file')dnl
undefine(`header')dnl
include(make/Objects.m4)
