
typedef struct i_list{
    int location_nr;
    struct i_list *next;
    struct i_list *previous;
}i_list_node;

void append_list_entry(i_list_node **head, i_list_node **new_node, i_list_node **previous);

void add_list_entry_in_the_middle(i_list_node **new_node, i_list_node **previous_node);

void add_list_entry_at_the_beginning(i_list_node **head, i_list_node **new_node);

void remove_list_entry(i_list_node **head,i_list_node **node_ptr);

