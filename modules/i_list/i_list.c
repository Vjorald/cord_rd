#include <stdio.h>
 
#include "net/sock/udp.h"
#include "net/ipv6.h"
#include <netdev_tap.h>
#include "net/sock/util.h"
#include "net/gcoap.h"
#include <ctype.h>
#include "net/cord/ep.h"
#include "string.h"
#include <math.h>
 
#include "shell.h"
#include "msg.h"

#include "i_list.h"


void append(intrusive_list_node *new_node)
{
    intrusive_list_node* previous = NULL;

    if(head == NULL)
    {
        new_node->previous = NULL;
        new_node->next = NULL;
        head = new_node;
    }
    else
    {   
        previous = &list[new_node->location_nr - 2].node_management;

        new_node->previous = previous;
        new_node->next = NULL;
        previous->next = new_node;
    }

}

void connect_endpoint_with_the_rest(intrusive_list_node *node_ptr, int location_nr)
{
    if (number_deleted_registrations == INITIAL_NUMBER_DELETED_ENDPOINTS) append(node_ptr);
    else
    {
        if (location_nr > 1 && location_nr < number_registered_endpoints)
        {
            int previous_endpoint_location = get_previous_endpoint_location(node_ptr->location_nr);

            node_ptr->next = list[previous_endpoint_location - 1].node_management.next;
            node_ptr->previous = &list[previous_endpoint_location - 1].node_management;
            list[previous_endpoint_location - 1].node_management.next = node_ptr;
        }
        else if (location_nr == 1)
        {
            node_ptr->previous = NULL;
            node_ptr->next = head;
            head = node_ptr;
        }
    }
}

void disconnect_endpoint_from_the_rest(int location_nr, intrusive_list_node *node_ptr){

    if (location_nr < number_registered_endpoints && location_nr > 1)
    {
        
        if (node_ptr->previous != NULL)
        {
            node_ptr->previous->next = node_ptr->next;
            node_ptr->next->previous = node_ptr->previous;
        }
        else
        {
            head = node_ptr->next;
            node_ptr->next->previous = NULL;
        }
    }
    else if(location_nr == number_registered_endpoints) node_ptr->previous->next = NULL;
    else if(location_nr == 1)
    {
        if (number_registered_endpoints == 1)  head = NULL;
        else
        {
            head = node_ptr->next;
            node_ptr->next->previous = NULL;
        }
    }

    node_ptr->next = NULL;
    node_ptr->previous = NULL;
}

void delete_endpoint(int location_nr) {

    if (number_deleted_registrations < DELETED_ENDPOINTS_MAX_NUMBER)
    {
        number_deleted_registrations++;
        deleted_registrations_list[number_deleted_registrations - 1] = list[location_nr - 1];

        Endpoint empty = { 0 };
        list[location_nr - 1] = empty;

        if (location_nr == number_registered_endpoints) number_registered_endpoints--;
        if (location_nr == number_registered_endpoints && number_registered_endpoints == 1) number_registered_endpoints--;
    }

}

