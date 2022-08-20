#ifndef LOCK_DRAW
#  define LOCK_DRAW

#  define HOLLOW(x) ((x) + (SPADES+1))

typedef char wchar[4];
typedef char schar[2];

typedef struct draw {
	char _lpad;
	wchar suit;
	schar number;
} CardDraw;

extern const schar numpm[];
extern const wchar suitpm[];

typedef struct {
	char* dis;
} Display;

extern Display display;

void update_display (void);

#endif
