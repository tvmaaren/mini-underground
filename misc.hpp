
typedef struct node node_t;
struct node{
	node_t* links[2];
	int value;
	bool to_be_removed=false;
};

float distance2d(Point2d a, Point2d b);
float distance_line_to_point(Point2d point, Point2d line1, Point2d line2);
node_t* add_node_before(node_t* node);

node_t* add_node_after(node_t* node);
node_t* remove_node(node_t* node);
typedef enum {NEXT=1, PREV=0} LINK_DIRECTION;
SHAPE int_to_shape(int in);
