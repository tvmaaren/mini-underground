#include <climits>

/*typedef struct {
	unsigned char r,g,b;
}COLOUR;*/
typedef struct{
	float x;
	float y;
}Point2d;

typedef enum{GAME_OVER, PLAYING} PLAY_STATUS;

typedef struct{
	int points;
	PLAY_STATUS play_status;
}STATUS;

typedef enum {SQUARE=0, CIRCLE=1, TRIANGLE=2, PENTAGON=3, GEM=4} SHAPE;
typedef enum {AT_STATION, ON_LINE, ON_REMOVED_LINE} LOCATION_TYPE;
const int shapes=5;
