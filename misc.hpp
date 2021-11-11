
void add_element(void** list, int *required, int *available, size_t element_size);
int inc_circ_count(int i, int max);//Make it loop round forwards
int dec_circ_count(int i, int max);//Make it loop round backwards

struct node;
typedef struct node node_t;

struct node{
	node_t* next;
	int value;
};

//Deletes the node that comes after the specified node from the linked list
void del_node(node_t* node);

//Adds a node after the specified node in the linked list and returns a pointer to that new node
node_t* add_node(node_t* node);


// vim: cc=100
