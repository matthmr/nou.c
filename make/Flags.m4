include(make/defaults.m4)dnl
define(makeflag, `$1:=M4FLAG_conf_$1')dnl
dnl
makeflag(`CC')
makeflag(`CFLAGS')
makeflag(`MD2ROFF')
dnl
undefine(`makeflag')dnl
