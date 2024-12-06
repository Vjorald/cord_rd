
typedef struct intrusive_list{
    int location_nr;
    struct intrusive_list *next;
    struct intrusive_list *previous;
}intrusive_list_node;

void append_list_entry(intrusive_list_node **head, intrusive_list_node **new_node, intrusive_list_node **previous);

void add_list_entry_in_the_middle(intrusive_list_node **new_node, intrusive_list_node **previous_node);

void add_list_entry_at_the_beginning(intrusive_list_node **head, intrusive_list_node **new_node);

void remove_list_entry(intrusive_list_node **head,intrusive_list_node **node_ptr);

