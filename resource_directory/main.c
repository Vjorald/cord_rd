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
    char location[20];
    char name[50];
    int lt;
    char ressources[100];
    Ptr next;
    Ptr previous;
} Endpoint;

Ptr head;

Endpoint list[100];

int number_registered_endpoints = 0;

static ssize_t _registration_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx);

static const coap_resource_t _resources[] = {

    { "/resourcedirectory/", COAP_GET | COAP_PUT | COAP_POST , _registration_handler, NULL },
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

    puts("======= Registration record infos: =========\n");
    printf("Endpoint: %s\n", list[number_registered_endpoints].name);
    printf("Lifetime: %d\n", list[number_registered_endpoints].lt);
    printf("Resources: %s\n", list[number_registered_endpoints].ressources);

    append(&list[number_registered_endpoints]);

    number_registered_endpoints++;

    puts("\n");
    puts("======= Registered Endpoints: ===========");

    printList(&list[number_registered_endpoints - 1]);



    gcoap_resp_init(pdu, buf, len, COAP_CODE_CREATED);

    coap_opt_add_string(pdu, COAP_OPT_LOCATION_PATH, location_str, ' ');
    
    size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_NONE);
    
    return resp_len + strlen(location_str);
   
}


int main(void)
{ 

    gcoap_register_listener(&_listener);
  
    return 0;
}