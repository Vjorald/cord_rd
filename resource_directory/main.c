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



typedef struct nodeelem{
    char location[20];
    char name[50];
    int lt;
    char ressources[100];
} Endpoint;

static int number_registered_endpoints = 0;

Endpoint registered_endpoints[100];

/*
// Define a node in the linked list
typedef struct {
    Endpoint endpoint;
    struct Node* next;
} Node;
*/

//Node* head = NULL;



static ssize_t _registration_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx);


/*
// Function to create a new node
Node* create_node(Endpoint endpoint) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    if (new_node == NULL) {
        printf("Memory allocation failed!\n");
        exit(1); 
    }
    new_node->endpoint = endpoint;
    new_node->next = NULL;
    return new_node;
}
*/
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

/*
void append_node(Node** head, Endpoint endpoint) {
    Node* new_node = create_node(endpoint);

    // If the list is empty, set new_node as the head
    if (*head == NULL) {
        *head = new_node;
        return;
    }

    // Traverse to the end of the list
    Node* current = *head;
    while (current->next != NULL) {
        current = current->next;
    }

    // Append the new node
    current->next = new_node;

    puts("Registration finished!");
}
*/
/*
// Function to remove a node by ID from the list
int remove_node(Node** head, int id) {
    Node* current = *head;
    Node* previous = NULL;

    // Traverse the list to find the node with the given id
    while (current != NULL && current->data.id != id) {
        previous = current;
        current = current->next;
    }

    // If the node wasn't found, return 0
    if (current == NULL) {
        return 0;  // Node with the given id not found
    }

    // If the node to be removed is the head node
    if (previous == NULL) {
        *head = current->next;  // Move the head to the next node
    } else {
        // Otherwise, bypass the node to be removed
        previous->next = current->next;
    }

    free(current);  // Free memory of the removed node
    return 1;  // Success
}
*/
/*
void append(Endpoint ep) {
    // Allocate memory for the new node
    Ptr newNode = (Ptr)malloc(sizeof(Endpoint));
    if (newNode == NULL) {
        perror("Failed to allocate memory");
        return;
    }
    
    // Copy data into the new node
    strcpy(newNode->name, ep.name);
    newNode->lt = ep.lt;
    strcpy(newNode->ressources, ep.ressources);
    newNode->next = NULL; // New node will point to NULL

    // If the list is empty, set head to the new node
    if (head == NULL) {
        head = newNode;
    } else {
        // Traverse to the last node
        Ptr current = head;
        while (current->next != NULL) {
            current = current->next;
        }
        // Link the last node to the new node
        current->next = newNode;
    }
}
*/
/*
void print_list() {
    
    
    Ptr current = head;
    while (current != NULL) {
        printf("Name: %s, LT: %d, Resources: %s\n", current->name, current->lt, current->ressources);
        current = current->next;
    }
}
*/


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

static ssize_t _registration_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx)
{
    (void)ctx;

    //char *query = (char*)coap_find_option(pdu, );

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
    Endpoint ep; 

    char location_str[20] = "";
    char location_str_1[6] = "/reg/";
    char location_str_2[2] = "/";
    char number_str[6];

    sprintf(number_str, "%d", number_registered_endpoints + 1);

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

    registered_endpoints[number_registered_endpoints] = ep;
    number_registered_endpoints++;

    puts("\n");
    puts("======= Registered Endpoints: ===========");
    for (int i = 0; i < number_registered_endpoints; i++)
    {
        printf("Endpoint: %s\n", registered_endpoints[i].name);
        printf("Lifetime: %d\n", registered_endpoints[i].lt);
        printf("Resources: %s\n", registered_endpoints[i].ressources);
        printf("Location: %s\n", registered_endpoints[i].location);
        puts("--\n");
    }


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