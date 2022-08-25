#!/usr/bin/awk -f

BEGIN {
	found = 0
	size = 0
	FS="\""
}

/const char\* errmsg\[\]/ { found = 1 }
/\};/ { found = 0 }

{
	if (found) {
		if ($0 ~ /\[E.*\] = ".*",/) {
			_size = length($2)
			size = (_size > size)? _size: size;
		}
	}
}

END {print size}
