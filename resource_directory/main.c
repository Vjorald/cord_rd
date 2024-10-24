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
#include "clist.h"


typedef struct list_node_t{
    char location[20];
    char name[50];
    int lt;
    char ressources[100];
    clist_node_t* next;
} clist_node_t;

clist_node_t list;

static ssize_t _registration_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx);

static const coap_resource_t _resources[] = {

    { "/resourcedirectory/", COAP_GET | COAP_PUT | COAP_POST | COAP_MATCH_SUBTREE, _registration_handler, NULL },
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

static int _print_node(clist_node_t *node) { printf("0x%08x ", (unsigned)node); return 0; }

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
    clist_node_t ep; 

    char location_str[20] = "";
    char location_str_1[6] = "/reg/";
    char location_str_2[2] = "/";
    char number_str[6];

   // sprintf(number_str, "%d", number_registered_endpoints + 1);

    strcat(location_str, location_str_1);
    strcat(location_str, number_str);
    strcat(location_str, location_str_2);

    ep.lt = atoi(lifetime); 
    strncpy((char*)ep.ressources, (char*)pdu->payload, sizeof(ep.ressources) - 1);
    ep.ressources[sizeof(ep.ressources) - 1] = '\0'; 
    strncpy((char*)ep.name, endpoint_name, sizeof(ep.name) - 1);
    ep.name[sizeof(ep.name) - 1] = '\0';
    strncpy((char*)ep.location, location_str, sizeof(ep.location) - 1);
    ep.location[sizeof(ep.location) - 1] = '\0';

    puts("======= Registration record infos: =========\n");
    printf("Endpoint: %s\n", ep.name);
    printf("Lifetime: %d\n", ep.lt);
    printf("Resources: %s\n", ep.ressources);

    clist_rpush(&list, &ep);

    puts("\n");
    puts("======= Registered Endpoints: ===========");

    clist_foreach(&list, _print_node);
    
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CREATED);

    
    coap_opt_add_format(pdu, COAP_FORMAT_LINK);
    
   
    size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);
    
    const char *response = location_str;
    

    if (pdu->payload_len >= strlen(response)) {
        memcpy(pdu->payload, response, strlen(response));
        return resp_len + strlen(response);
    }
    else {
        
        puts("gcoap_cli: msg buffer too small");
        return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);
    }
}


int main(void)
{  

    gcoap_register_listener(&_listener);
  
    return 0;
}