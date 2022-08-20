define(default, `ifdef(`M4FLAG_conf_$1',, `define(`M4FLAG_conf_$1', `$2')')')dnl
dnl
default(`CC', `cc')dnl
default(`CFLAGS', `')dnl
default(`MD2ROFF', `marked-man')dnl
dnl
undefine(`default')dnl
