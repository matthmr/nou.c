include(make/defaults.m4)dnl
define(makeflagdef, `$1:=M4FLAG_conf_$1')dnl
define(makeflagchange, `$1?=M4FLAG_conf_$1')dnl
dnl
makeflagdef(`CC')
makeflagdef(`CFLAGS')
makeflagchange(`CFLAGSADD')
makeflagdef(`MD2ROFF')
dnl
undefine(`makeflag')dnl
