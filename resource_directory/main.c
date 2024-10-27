#include <stdio.h>
 
#include "net/sock/udp.h"
#include "net/ipv6.h"
#include <netdev_tap.h>
#include "net/sock/util.h"
#include "net/gcoap.h"
#include <ctype.h>
#include "net/cord/ep.h"
 
#include "shell.h"
#include "msg.h"



typedef struct nodeelement *Ptr;

typedef struct nodeelement{
    bool next_free_middle_position;
    char location[20];
    char name[50];
    int lt;
    char ressources[100];
    int location_nr;
    Ptr next;
    Ptr previous;
} Endpoint;


bool free_positions_in_the_middle = false;

Ptr head;

Endpoint list[100];

int number_registered_endpoints = 0;

static ssize_t _registration_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx);

static ssize_t _update_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx);

static const coap_resource_t _resources[] = {

    { "/resourcedirectory/", COAP_GET | COAP_PUT | COAP_POST , _registration_handler, NULL },
    { "/reg/",  COAP_PUT | COAP_POST | COAP_DELETE | COAP_MATCH_SUBTREE , _update_handler, NULL },
};

static gcoap_listener_t _listener = {
    _resources,
    ARRAY_SIZE(_resources),
    GCOAP_SOCKET_TYPE_UDP,
    NULL,
    NULL,
    NULL
};


void parse_query_buffer(unsigned char *query_buffer, char *ep, char *lt) {
   
    char *token = strtok((char *)query_buffer, " ");
    
    while (token != NULL) {
        
        if (strncmp(token, "ep=", 3) == 0) {
            strcpy(ep, token + 3);
        }
        
        else if (strncmp(token, "lt=", 3) == 0) {
            strcpy(lt, token + 3);
        }
        
        token = strtok(NULL, " ");
    }
}

static void append(Endpoint *new_node)
{
    Ptr actual = NULL;

    if(head == NULL)
    {
        new_node->previous = NULL;
        new_node->next = NULL;
        head = new_node;
    }
    else
    {   
        actual = head;
        do
        {
            if (actual->next != NULL)
            {
                actual = actual->next;
            }
            else
            {
                break;
            }
            
        } while (actual->next != NULL);

        new_node->previous = actual;
        new_node->next = NULL;
        actual->next = new_node;
        
    }

}


static int printList(Endpoint* endpoint)
{
    if (head == NULL)
    {
        puts("There are no registered endpoints.");
        return 0;
    }

    printf("I'm being printed: %d \n", endpoint->lt);
    puts("===\n");

    Ptr actual = head;
    
    printf("Endpoint: %s\n", actual->name);
    printf("Lifetime: %d\n", actual->lt);
    printf("Resources: %s\n", actual->ressources);
    printf("Location: %s\n", actual->location);
    puts("===\n");

    do
    {
        if(actual->next != NULL)
        {
            actual = actual->next;
            printf("Endpoint: %s\n", actual->name);
            printf("Lifetime: %d\n", actual->lt);
            printf("Resources: %s\n", actual->ressources);
            printf("Location: %s\n", actual->location);
            puts("===\n"); 
        }
        else
        {
            return 0;
        }
        


    }while(actual->next != NULL);

    return 0;

}

static int find_next_empty(Endpoint* endpoint)
{
    (void) endpoint;

    if (head == NULL)
        {
            puts("There are no registered endpoints.");
            return 0;
        }
    else
    {
        Ptr actual = head;

        if (actual->next_free_middle_position == true)
        {return actual->location_nr;} 

        do
        {
            if (actual->next != NULL)
            {
                actual = actual->next;

                if(actual->next_free_middle_position == true)
                {
                    return actual->location_nr;
                }
            }
            
        } while (actual->next != NULL);
    }

        return 0;

}


static ssize_t _registration_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx)
{
    (void)ctx;


    unsigned char query_buffer[100];

    int result = coap_opt_get_string(pdu, COAP_OPT_URI_QUERY, query_buffer, 100, ' ');

    printf("%s, result: %d \n", query_buffer, result);
    char endpoint_name[60];
    char lifetime[10];
    parse_query_buffer(query_buffer, endpoint_name, lifetime);
    puts("======== Request infos: =======\n");
    printf("Endpoint: %s\n", endpoint_name);
    printf("Lifetime: %s\n", lifetime);
    printf("Resources: %s\n", pdu->payload);
    puts("\n");
    

    char location_str[20] = "";
    char location_str_1[6] = "/reg/";
    char location_str_2[2] = "/";
    char number_str[6];

    if (free_positions_in_the_middle == false)
    {
    sprintf(number_str, "%d", number_registered_endpoints + 1);

    strcat(location_str, location_str_1);
    strcat(location_str, number_str);
    strcat(location_str, location_str_2);

    list[number_registered_endpoints].lt = atoi(lifetime); 
    strncpy((char*)list[number_registered_endpoints].ressources, (char*)pdu->payload, sizeof(list[number_registered_endpoints].ressources) - 1);
    list[number_registered_endpoints].ressources[sizeof(list[number_registered_endpoints].ressources) - 1] = '\0'; 
    strncpy((char*)list[number_registered_endpoints].name, endpoint_name, sizeof(list[number_registered_endpoints].name) - 1);
    list[number_registered_endpoints].name[sizeof(list[number_registered_endpoints].name) - 1] = '\0';
    strncpy((char*)list[number_registered_endpoints].location, location_str, sizeof(list[number_registered_endpoints].location) - 1);
    list[number_registered_endpoints].location[sizeof(list[number_registered_endpoints].location) - 1] = '\0';
    list[number_registered_endpoints].location_nr = number_registered_endpoints + 1;

    puts("======= Registration record infos: =========\n");
    printf("Endpoint: %s\n", list[number_registered_endpoints].name);
    printf("Lifetime: %d\n", list[number_registered_endpoints].lt);
    printf("Resources: %s\n", list[number_registered_endpoints].ressources);

    
        list[number_registered_endpoints].next_free_middle_position = false;

        append(&list[number_registered_endpoints]);

        number_registered_endpoints++;
    }
    else
    {
        int location = find_next_empty(&list[number_registered_endpoints]);

        if (location > 0 && location < number_registered_endpoints)
        {
            sprintf(number_str, "%d", location + 1);

            strcat(location_str, location_str_1);
            strcat(location_str, number_str);
            strcat(location_str, location_str_2);

            list[location].lt = atoi(lifetime);
            strncpy((char*)list[location].ressources, (char*)pdu->payload, sizeof(list[location].ressources) - 1);
            list[location].ressources[sizeof(list[location].ressources) - 1] = '\0';
            strncpy((char*)list[location].name, endpoint_name, sizeof(list[location].name) - 1);
            list[location].name[sizeof(list[location].name) - 1] = '\0';
            strncpy((char*)list[location].location, location_str, sizeof(list[location].location) - 1);
            list[location].location[sizeof(list[location].location) - 1] = '\0';
            list[location].location_nr = location + 1;

            list[location].next = list[location - 1].next;
            list[location].previous = &list[location - 1];
            list[location - 1].next = &list[location];

            if (list[location].next->location_nr > location + 1)
            {
                list[location].next_free_middle_position = true;
            }
            else
            {
                list[location].next_free_middle_position = false;
            }

            list[location - 1].next_free_middle_position = false;

        }
    }


    puts("\n");
    puts("======= Registered Endpoints: ===========");

    printList(&list[number_registered_endpoints - 1]);



    gcoap_resp_init(pdu, buf, len, COAP_CODE_CREATED);

    coap_opt_add_string(pdu, COAP_OPT_LOCATION_PATH, location_str, ' ');
    
    size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_NONE);
    
    return resp_len + strlen(location_str);
   
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


static ssize_t _update_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx)
{
    (void)ctx;

    char uri[CONFIG_NANOCOAP_URI_MAX] = { 0 };

    coap_get_uri_path(pdu, (uint8_t *)uri);

    printf("Received: %s\n", uri);

    unsigned method = coap_method2flag(coap_get_code_detail(pdu));



    if (method == COAP_POST)
    {
        puts("Post Method!!\n");
    }

    if (method == COAP_DELETE)
    {
        int location_nr = extract_number_from_location(uri);

        if (location_nr < number_registered_endpoints && location_nr > 1)
        {
            if (list[location_nr - 1].previous != NULL)
            {
                list[location_nr - 1].previous->next = list[location_nr - 1].next;
                list[location_nr - 1].previous->next_free_middle_position = true;
                free_positions_in_the_middle = true;
                list[location_nr - 1].next->previous = list[location_nr - 1].previous;
            }
            else
            {
                head = list[location_nr - 1].next;
                free_positions_in_the_middle = true;
                list[location_nr - 1].next->previous = NULL;
            }
            
        }
        else if(location_nr == number_registered_endpoints)
        {
            list[location_nr - 1].previous->next = NULL;
            number_registered_endpoints--;
        }
        else if(location_nr == 1)
        {
            if (number_registered_endpoints == 1)
            {
                head = NULL;
                number_registered_endpoints--;
            }
            else
            {
                head = list[location_nr - 1].next;
                list[location_nr - 1].next->previous = NULL;
            }
        }

        Endpoint empty;

        list[location_nr - 1] = empty;

    }

    gcoap_resp_init(pdu, buf, len, COAP_CODE_CHANGED);

    
    coap_opt_add_format(pdu, COAP_FORMAT_LINK);

   
    size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_NONE);
    
    return resp_len;
}


int main(void)
{ 

    gcoap_register_listener(&_listener);
  
    return 0;
}