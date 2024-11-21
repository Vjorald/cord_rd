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


intrusive_list_node *head;

Endpoint list[REGISTERED_ENDPOINTS_MAX_NUMBER];

Endpoint deleted_registrations_list[DELETED_ENDPOINTS_MAX_NUMBER];

Endpoint lookup_result_list[LOOKUP_RESULTS_MAX_LEN];

int number_registered_endpoints = INITIAL_NUMBER_REGISTERED_ENDPOINTS;

int number_deleted_registrations = INITIAL_NUMBER_DELETED_ENDPOINTS;


void parse_query_buffer(unsigned char *query_buffer, char *ep, char *lt) {
   
    char *token = strtok((char *)query_buffer, " ");
    
    while (token != NULL) {
        if (strncmp(token, "ep=", 3) == 0) strcpy(ep, token + 3);
        else if (strncmp(token, "lt=", 3) == 0) strcpy(lt, token + 3);
        token = strtok(NULL, " ");
    }
}

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

void build_location_string(int location_nr, char* location_str)
{
    char location_str_1[LOCATION_STR_1_MAX_LEN] = "/reg/";
    char location_str_2[LOCATION_STR_2_MAX_LEN] = "/";
    char number_str[LOCATION_NUMBER_STR_MAX_LEN];

    sprintf(number_str, "%d", location_nr);

    strcat(location_str, location_str_1);
    strcat(location_str, number_str);
    strcat(location_str, location_str_2);

}

void build_base_uri_string(char* addr_str, char* base_uri){
    
    char* base_first = "coap://[";
    char* ending = "]";

    strcat(base_uri, base_first);
    strcat(base_uri, addr_str);
    strcat(base_uri, ending);
}

int printList(Endpoint* endpoint)
{

    (void) endpoint;

    if (head == NULL)
    {
        puts("There are no registered endpoints.");
        return 0;
    }

    puts("\n");
    puts("======= Registered Endpoints: ===========\n");

    intrusive_list_node *actual = head;
    Endpoint *endpoint_ptr = container_of(actual, Endpoint, node_management);
    char location_str[LOCATION_STR_MAX_LEN] = "";
    build_location_string(actual->location_nr, location_str);

    printf("Endpoint: %s\n", endpoint_ptr->name);
    printf("Lifetime: %d\n", endpoint_ptr->lt);
    printf("Resources: %s\n", endpoint_ptr->ressources);
    printf("Location: %s\n", location_str);
    printf("Base URI: %s\n", endpoint_ptr->base);
    puts("===\n");

    do
    {
        if(actual->next != NULL)
        {
            actual = actual->next;
            endpoint_ptr = container_of(actual, Endpoint, node_management);
            memset(location_str, 0, sizeof(location_str));
            build_location_string(actual->location_nr, location_str);

            printf("Endpoint: %s\n", endpoint_ptr->name);
            printf("Lifetime: %d\n", endpoint_ptr->lt);
            printf("Resources: %s\n", endpoint_ptr->ressources);
            printf("Location: %s\n", location_str);
            printf("Base URI: %s\n", endpoint_ptr->base);
            puts("===\n"); 
        }
        else return 0;

    }while(actual->next != NULL);

    return 0;

}

void find_endpoints_by_pattern(char* pattern)
{
    memset(lookup_result_list, 0, sizeof(lookup_result_list));
    int last_element_index = 0;

    if (head == NULL)
    {
        puts("There are no registered endpoints.");
        return ;
    }

    intrusive_list_node *actual = head;
    Endpoint *endpoint_ptr = container_of(actual, Endpoint, node_management);

    if (strcmp(endpoint_ptr->base, pattern)  == 0)
    {
        lookup_result_list[last_element_index] = *endpoint_ptr;
        last_element_index++;
    }
    
    do
    {
        if(actual->next != NULL)
        {
            actual = actual->next;
            endpoint_ptr = container_of(actual, Endpoint, node_management);
           
            if(strcmp(endpoint_ptr->base, pattern)  == 0)
            {
                lookup_result_list[last_element_index] = *endpoint_ptr;
                last_element_index++;
            }
        }
        else break; 

    }while(actual->next != NULL);

    return ;

}

Endpoint find_endpoint_by_pattern(char* pattern)
{
    Endpoint empty = { 0 };

    if (head == NULL)
    {
        puts("There are no registered endpoints.");
        return empty;
    }


    intrusive_list_node *actual = head;
    Endpoint *endpoint_ptr = container_of(actual, Endpoint, node_management);
    char location_str[LOCATION_STR_MAX_LEN] = "";
    build_location_string(actual->location_nr, location_str);

    if (strcmp(endpoint_ptr->name, pattern)  == 0 || strcmp(location_str, pattern) == 0) return *endpoint_ptr;
    
    do
    {
        if(actual->next != NULL)
        {
            actual = actual->next;
            endpoint_ptr = container_of(actual, Endpoint, node_management);
            memset(location_str, 0, sizeof(location_str));
            build_location_string(actual->location_nr, location_str);

            if (strcmp(endpoint_ptr->name, pattern)  == 0 || strcmp(location_str, pattern) == 0) return *endpoint_ptr;
        }
        else return empty;
        
    }while(actual->next != NULL);

    return empty;

}

void initialize_endpoint(char *lifetime, char *endpoint_name, Endpoint *endpoint_ptr, intrusive_list_node *node_ptr, char *base_uri, coap_pkt_t *pdu, char* location_str, int location_nr)
{
    build_location_string(location_nr, location_str);

    endpoint_ptr->lt = atoi(lifetime); 

    strncpy((char*)endpoint_ptr->ressources, (char*)pdu->payload, sizeof(endpoint_ptr->ressources) - 1);
    endpoint_ptr->ressources[sizeof(endpoint_ptr->ressources) - 1] = '\0';

    strncpy((char*)endpoint_ptr->name, endpoint_name, sizeof(endpoint_ptr->name) - 1);
    endpoint_ptr->name[sizeof(endpoint_ptr->name) - 1] = '\0';

    node_ptr->location_nr = location_nr;

    strncpy((char*)endpoint_ptr->base, base_uri, sizeof(endpoint_ptr->base) - 1);
    endpoint_ptr->base[sizeof(endpoint_ptr->base) - 1] = '\0';
}


int get_next_empty_location(Endpoint* deleted_list)
{
    (void) deleted_list;

    int location_nr = deleted_registrations_list[0].node_management.location_nr;

    for (int i = 0; i < number_deleted_registrations; i++) deleted_registrations_list[i] = deleted_registrations_list[i + 1];
 
    Endpoint empty = { 0 }; 
    deleted_registrations_list[number_deleted_registrations - 1] = empty;
    number_deleted_registrations--; 

    return location_nr;

}

int get_previous_endpoint_location(int actual_location)
{
    for (int i = actual_location - 2; i >= 0; i--) if(strlen(list[i].name) > 0) return list[i].node_management.location_nr;

    return -1;
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

int extract_number_from_location(char *path) {
    int number = -1; 

    const char *start = strstr(path, "/reg/");
    if (start) {
        start += 5;
        number = atoi(start);
    }
    
    return number;
}

void extract_value_from_query(const char *input, char *href_value, char* pref) {
    const char *prefix = pref;
    const char *start = strstr(input, prefix);

    if (start) {
        start += strlen(prefix); 
        
        const char *end = strchr(start, ' ');

        if (!end) end = input + strlen(input);
        
        size_t length = end - start;
        strncpy(href_value, start, length);
        href_value[length] = '\0'; 
    } 
    else href_value[0] = '\0';
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

void build_result_string(char* lookup_result, char* first_bracket, char* second_href_bracket, char* ep_key, char* base, char* rt, Endpoint* endpoint){

    char location_str[LOCATION_STR_MAX_LEN] = "";

    build_location_string(endpoint->node_management.location_nr, location_str);

    strcat(lookup_result, first_bracket);
    strcat(lookup_result, location_str);
    strcat(lookup_result, second_href_bracket);
    strcat(lookup_result, ep_key);
    strcat(lookup_result, (*endpoint).name);
    strcat(lookup_result, base);
    strcat(lookup_result, (*endpoint).base);
    strcat(lookup_result, rt);

}

void build_resource_string(int number_sensors, char extracted_sensor_uris[RESOURCE_URI_MAX_NUMBER][RESOURCE_URI_MAX_LEN], char* lookup_result, Endpoint* endpoint)
{

    for (int i = 0; i < number_sensors; i++)
    {
        strcat(lookup_result, "<");
        strcat(lookup_result, endpoint->base);
        strcat(lookup_result, extracted_sensor_uris[i]);
        strcat(lookup_result, ">");

        if(i < number_sensors - 1) strcat(lookup_result, ",");
    }
    
}

int extract_resource_uris(const char *input, char uris[RESOURCE_URI_MAX_NUMBER][RESOURCE_URI_MAX_LEN]) {
    int uri_count = 0;
    const char *start = input;
    const char *end;

    while ((start = strchr(start, '<')) != NULL && uri_count < RESOURCE_URI_MAX_NUMBER) {
        start++; 
        end = strchr(start, '>');
        if (end == NULL) break; 

        int length = end - start;
        if (length >= RESOURCE_URI_MAX_LEN) length = length - 1;

        
        strncpy(uris[uri_count], start, length);
        uris[uri_count][length] = '\0'; 

        uri_count++;
        start = end + 1; 
    }

    return uri_count;
}
