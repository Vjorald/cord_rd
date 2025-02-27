#include <stdio.h>
#include "i_list.h"


void add_list_entry_at_the_beginning(i_list_node **head, i_list_node **new_node){

    (*new_node)->previous = NULL;
    (*new_node)->next = *head;
    *head = *new_node;
}

void append_list_entry(i_list_node **head, i_list_node **new_node, i_list_node **previous){

     if(*head == NULL)
    {
        add_list_entry_at_the_beginning(head, new_node);
    }
    else
    {   

        (*new_node)->previous = *previous;
        (*new_node)->next = NULL;
        (*previous)->next = *new_node;
    }
}

void add_list_entry_in_the_middle(i_list_node **new_node, i_list_node **previous_node){

    (*new_node)->next = (*previous_node)->next;
    (*new_node)->previous = *previous_node;
    (*previous_node)->next = *new_node;
    (*new_node)->next->previous = *new_node;
}

void remove_list_entry(i_list_node **head, i_list_node **node_ptr){

     if ((*node_ptr)->previous != NULL)
    {
        (*node_ptr)->previous->next = (*node_ptr)->next;

        if ((*node_ptr)->next != NULL){
            (*node_ptr)->next->previous = (*node_ptr)->previous;
        }
    }
    else
    {
        *head = (*node_ptr)->next;
        
        if ((*node_ptr)->next != NULL){
            (*node_ptr)->next->previous = NULL;
        }
        
    }

    (*node_ptr)->next = NULL;
    (*node_ptr)->previous = NULL;

}

