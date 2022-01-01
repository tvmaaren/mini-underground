#include <climits>

typedef struct {
	unsigned char r,g,b;
}COLOUR;
typedef struct{
	float x;
	float y;
}Point2d;

typedef enum {SQUARE=0, CIRCLE=1, TRIANGLE=2, PENTAGON=3, GEM=4} SHAPE;
const int shapes=5;
