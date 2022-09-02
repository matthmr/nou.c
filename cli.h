#ifndef LOCK_CLI_COLOR
#  define LOCK_CLI_COLOR

#  define ESC "\x1b"
#  define __ESC__ "\x1b["
#  define __RESET__ __ESC__ "0m"

#  define __BOLD__ "1m"
#  define __LIGHT__ "2m"
#  define __ITALICS__ "3m"
#  define __UNDERLINE__ "4m"
#  define __INVERT__ "7m"
#  define BOLD(x) __ESC__ __BOLD__ x __RESET__
#  define LIGHT(x) __ESC__ __LIGHT__ x __RESET__
#  define ITALICS(x) __ESC__ __ITALICS__ x __RESET__
#  define UNDERLINE(x) __ESC__ __UNDERLINE__ x __RESET__
#  define INVERT(x) __ESC__ __INVERT__ x __RESET__
#  define RESET(x) __RESET__ x __RESET__

#  define STYLE(x,y) __ESC__ "0;" x y __RESET__

#  define GREY_FG(x) __ESC__ "30m" x __RESET__
#  define RED_FG(x) __ESC__ "31m" x __RESET__
#  define GREEN_FG(x) __ESC__ "32m" x __RESET__
#  define YELLOW_FG(x) __ESC__ "33m" x __RESET__
#  define BLUE_FG(x) __ESC__ "34m" x __RESET__
#  define PURPLE_FG(x) __ESC__ "35m" x __RESET__
#  define CYAN_FG(x) __ESC__ "36m" x __RESET__
#  define LGREY_FG(x) __ESC__ "37m" x __RESET__

#  define LIGHT_GREY_FG(x) __ESC__ "90m" x __RESET__
#  define LIGHT_RED_FG(x) __ESC__ "91m" x __RESET__
#  define LIGHT_GREEN_FG(x) __ESC__ "92m" x __RESET__
#  define LIGHT_YELLOW_FG(x) __ESC__ "93m" x __RESET__
#  define LIGHT_BLUE_FG(x) __ESC__ "94m" x  __RESET__
#  define LIGHT_PURPLE_FG(x) __ESC__ "95m" x __RESET__
#  define LIGHT_CYAN_FG(x) __ESC__ "96m" x __RESET__
#  define LIGHT_LGREY_FG(x) __ESC__ "97m" x __RESET__

#  define GREY_BG(x) __ESC__ "40m" x __RESET__
#  define RED_BG(x) __ESC__ "41m" x __RESET__
#  define GREEN_BG(x) __ESC__ "42m" x __RESET__
#  define YELLOW_BG(x) __ESC__ "43m" x __RESET__
#  define BLUE_BG(x) __ESC__ "44m" x __RESET__
#  define PURPLE_BG(x) __ESC__ "45m" x __RESET__
#  define CYAN_BG(x) __ESC__ "46m" x __RESET__
#  define LGREY_BG(x) __ESC__ "47m" x __RESET__

#  define LIGHT_GREY_BG(x) __ESC__ "100m" x __RESET__
#  define LIGHT_RED_BG(x) __ESC__ "101m" x __RESET__
#  define LIGHT_GREEN_BG(x) __ESC__ "102m" x __RESET__
#  define LIGHT_YELLOW_BG(x) __ESC__ "103m" x __RESET__
#  define LIGHT_BLUE_BG(x) __ESC__ "104m" x __RESET__
#  define LIGHT_PURPLE_BG(x) __ESC__ "105m" x __RESET__
#  define LIGHT_CYAN_BG(x) __ESC__ "106m" x __RESET__
#  define LIGHT_LGREY_BG(x) __ESC__ "107m" x __RESET__

#endif

#ifndef LOCK_CLI_CMOV
#  define LOCK_CLI_CMOV

#  define MOVSTART __ESC__ "H"
#  define MOVPOS(x,y) __ESC__ x ";" y "H"
#  define MOVUP(x) __ESC__ x "A"
#  define MOVDOWN(x) __ESC__ x "B"
#  define MOVRIGHT(x) __ESC__ x "C"
#  define MOVLEFT(x) __ESC__ x "D"
#  define MOVDOWN0(x) __ESC__ x "E"
#  define MOVUP0(x) __ESC__ x "F"
#  define MOVCOL(x) __ESC__ x "G"
#  define MOVUP1 ESC "M"
#  define MOVSAVEDEC ESC "7"
#  define MOVRESTOREDEC ESC "8"
#  define MOVSAVESOC __ESC__ "s"
#  define MOVRESTORESOC __ESC__ "u"
#  define MOVSTART __ESC__ "H"

#  define ERASEINDISPLAY __ESC__ "J"
#  define ERASE2ENDSCREEN __ESC__ "0J"
#  define ERASE2BEGINSCREEN __ESC__ "1J"
#  define ERASEALLSCREEN __ESC__ "2J"
#  define ERASESAVELINE __ESC__ "3J"
#  define ERASEINLINE __ESC__ "K"
#  define ERASE2ENDLINE __ESC__ "0K"
#  define ERASE2STARTLINE __ESC__ "1K"
#  define ERASEALLLINE __ESC__ "2K"

#endif
