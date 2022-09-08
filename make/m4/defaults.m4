define(`default', `ifdef(`M4FLAG_conf_$1',, `define(`M4FLAG_conf_$1', `$2')')')dnl
define(`incdefault', `ifdef(`M4FLAG_include_$1',, `define(`M4FLAG_include_$1', `$2')')')dnl
dnl
default(`CC', `cc')dnl
default(`CFLAGS', `')dnl
default(`CFLAGSADD', `')dnl
default(`MD2ROFF', `marked-man')dnl
incdefault(`HEADER', `1')dnl
dnl
undefine(`default')dnl
