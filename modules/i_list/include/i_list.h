
typedef struct intrusive_list{
    int location_nr;
    struct intrusive_list *next;
    struct intrusive_list *previous;
}intrusive_list_node;


void append(intrusive_list_node *new_node);

void connect_endpoint_with_the_rest(intrusive_list_node *node_ptr, int location_nr);

void disconnect_endpoint_from_the_rest(int location_nr, intrusive_list_node *node_ptr);

void delete_endpoint(int location_nr);

