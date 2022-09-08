define(`rec', `ifelse($1, `', ,`$1 rec(shift($@))')')dnl
dnl
define(`target', `define(`__target__', `$1')')dnl
define(`target_obj', `define(`__target_objects__', `rec($@)')')dnl
define(`target_gen', `dnl
__target__: __target_objects__')dnl
