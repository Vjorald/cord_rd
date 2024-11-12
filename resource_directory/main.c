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

#define BASE_URI_MAX_LEN 64
#define ENDPOINT_NAME_MAX_LEN 50
#define RESOURCES_MAX_LEN 100
#define REGISTERED_ENDPOINTS_MAX_NUMBER 100
#define DELETED_ENDPOINTS_MAX_NUMBER 100
#define LOOKUP_RESULTS_MAX_LEN 100
#define INITIAL_NUMBER_REGISTERED_ENDPOINTS 0
#define INITIAL_NUMBER_DELETED_ENDPOINTS 0
#define LOCATION_STR_1_MAX_LEN 6
#define LOCATION_STR_2_MAX_LEN 2
#define LOCATION_NUMBER_STR_MAX_LEN 4
#define LOCATION_STR_MAX_LEN 12
#define RESOURCE_URI_MAX_NUMBER 10
#define RESOURCE_URI_MAX_LEN 100
#define QUERY_BUFFER_MAX_LEN 100
#define LIFETIME_STR_MAX_LEN 10
#define LOOKUP_RESULT_STR_MAX_LEN 1024

typedef struct intrusive_list{
    int location_nr;
    struct intrusive_list *next;
    struct intrusive_list *previous;
}intrusive_list_node;

typedef struct nodeelement{
    char base[BASE_URI_MAX_LEN];
    char name[ENDPOINT_NAME_MAX_LEN];
    int lt;
    char ressources[RESOURCES_MAX_LEN];
    intrusive_list_node node_management;
} Endpoint;

intrusive_list_node *head;

Endpoint list[REGISTERED_ENDPOINTS_MAX_NUMBER];

Endpoint deleted_registrations_list[DELETED_ENDPOINTS_MAX_NUMBER];

Endpoint lookup_result_list[LOOKUP_RESULTS_MAX_LEN];

int number_registered_endpoints = INITIAL_NUMBER_REGISTERED_ENDPOINTS;

int number_deleted_registrations = INITIAL_NUMBER_DELETED_ENDPOINTS;

static ssize_t _registration_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx);

static ssize_t _update_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx);

static ssize_t _endpoint_lookup_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx);

static ssize_t _resource_lookup_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx);

static const coap_resource_t _resources[] = {

    { "/resourcedirectory/", COAP_GET | COAP_PUT | COAP_POST , _registration_handler, NULL },
    { "/reg/",  COAP_PUT | COAP_POST | COAP_DELETE | COAP_MATCH_SUBTREE , _update_handler, NULL },
    { "/endpoint-lookup/",  COAP_GET | COAP_MATCH_SUBTREE , _endpoint_lookup_handler, NULL },
    { "/resource-lookup/",  COAP_GET | COAP_MATCH_SUBTREE , _resource_lookup_handler, NULL}
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
        if (strncmp(token, "ep=", 3) == 0) strcpy(ep, token + 3);
        else if (strncmp(token, "lt=", 3) == 0) strcpy(lt, token + 3);
        token = strtok(NULL, " ");
    }
}

static void append(intrusive_list_node *new_node)
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

static void build_location_string(int location_nr, char* location_str)
{
    char location_str_1[LOCATION_STR_1_MAX_LEN] = "/reg/";
    char location_str_2[LOCATION_STR_2_MAX_LEN] = "/";
    char number_str[LOCATION_NUMBER_STR_MAX_LEN];

    sprintf(number_str, "%d", location_nr);

    strcat(location_str, location_str_1);
    strcat(location_str, number_str);
    strcat(location_str, location_str_2);

}

static void build_base_uri_string(char* addr_str, char* base_uri){
    
    char* base_first = "coap://[";
    char* ending = "]";

    strcat(base_uri, base_first);
    strcat(base_uri, addr_str);
    strcat(base_uri, ending);
}

static int printList(Endpoint* endpoint)
{
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

static void find_endpoints_by_pattern(char* pattern)
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

static Endpoint find_endpoint_by_pattern(char* pattern)
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

static void initialize_endpoint(char *lifetime, char *endpoint_name, Endpoint *endpoint_ptr, intrusive_list_node *node_ptr, char *base_uri, coap_pkt_t *pdu, char* location_str, int location_nr)
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


static int get_next_empty_location(Endpoint* deleted_list)
{
    (void) deleted_list;

    int location_nr = deleted_registrations_list[0].node_management.location_nr;

    for (int i = 0; i < number_deleted_registrations; i++) deleted_registrations_list[i] = deleted_registrations_list[i + 1];
 
    Endpoint empty = { 0 }; 
    deleted_registrations_list[number_deleted_registrations - 1] = empty;
    number_deleted_registrations--; 

    return location_nr;

}

static int get_previous_endpoint_location(int actual_location)
{
    for (int i = actual_location - 2; i >= 0; i--) if(strlen(list[i].name) > 0) return list[i].node_management.location_nr;

    return -1;
}

static void connect_endpoint_with_the_rest(intrusive_list_node *node_ptr, int location_nr)
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

static void build_result_string(char* lookup_result, char* first_bracket, char* second_href_bracket, char* ep_key, char* base, char* rt, Endpoint* endpoint){

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

static void build_resource_string(int number_sensors, char extracted_sensor_uris[RESOURCE_URI_MAX_NUMBER][RESOURCE_URI_MAX_LEN], char* lookup_result, Endpoint* endpoint)
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

static size_t send_blockwise_response(coap_pkt_t *pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx, char* lookup_result)
{
    (void) ctx;

    coap_block1_t req_block;

    if (coap_get_block2(pdu, &req_block) == 0) {
        
        req_block.blknum = 0;
        req_block.szx = COAP_BLOCKSIZE_64; 
    
    }

    coap_block_slicer_t resp_block;
    coap_block_slicer_init(&resp_block, req_block.blknum, 1 << (req_block.szx + 4));

    size_t block_size = (1 << (req_block.szx + 4)); 
    size_t offset =  req_block.blknum * block_size;
    size_t chunk_len =  (offset + block_size < strlen(lookup_result)) ? block_size : (strlen(lookup_result) - offset);


    if (offset >= strlen(lookup_result)) return COAP_CODE_404;

    bool more = (offset + chunk_len < strlen(lookup_result));

    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    req_block.more = more;
    coap_opt_add_block2(pdu, &resp_block, more);
    size_t result = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);
    memcpy(pdu->payload, lookup_result + offset, chunk_len);

    return result + chunk_len;
}

static ssize_t _registration_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx)
{

    char addr_str[IPV6_ADDR_MAX_STR_LEN];
    sock_udp_ep_t* remote = ctx->remote;
    ipv6_addr_to_str(addr_str, (ipv6_addr_t *)remote->addr.ipv6, IPV6_ADDR_MAX_STR_LEN);

    char base_uri[BASE_URI_MAX_LEN];
    build_base_uri_string(addr_str, base_uri);
    
    unsigned char query_buffer[QUERY_BUFFER_MAX_LEN];
    int result = coap_opt_get_string(pdu, COAP_OPT_URI_QUERY, query_buffer, QUERY_BUFFER_MAX_LEN, ' ');

    intrusive_list_node *node_ptr;
    Endpoint *endpoint_ptr;

    printf("%s, result: %d \n", query_buffer, result);
    char endpoint_name[ENDPOINT_NAME_MAX_LEN];
    char lifetime[LIFETIME_STR_MAX_LEN];
    parse_query_buffer(query_buffer, endpoint_name, lifetime);
    puts("======== Request infos: =======\n");
    printf("Endpoint: %s\n", endpoint_name);
    printf("Lifetime: %s\n", lifetime);
    printf("Resources: %s\n", pdu->payload);
    puts("\n");
    

    char location_str[LOCATION_STR_MAX_LEN] = "";
    int location_nr = -1;

    if (number_deleted_registrations == INITIAL_NUMBER_DELETED_ENDPOINTS)
    {
        node_ptr = &list[number_registered_endpoints].node_management;
        endpoint_ptr = container_of(node_ptr, Endpoint, node_management);
        number_registered_endpoints++;
        location_nr = number_registered_endpoints;
    }
    else
    {
        location_nr = get_next_empty_location(deleted_registrations_list);
        node_ptr = &list[location_nr - 1].node_management;
        endpoint_ptr = container_of(node_ptr, Endpoint, node_management);
    }

    memset(location_str, 0, sizeof(location_str));
    initialize_endpoint(lifetime, endpoint_name, endpoint_ptr, node_ptr, base_uri, pdu, location_str, location_nr);
    connect_endpoint_with_the_rest(node_ptr, node_ptr->location_nr);
    printList(&list[number_registered_endpoints - 1]);

    gcoap_resp_init(pdu, buf, len, COAP_CODE_CREATED);
    coap_opt_add_string(pdu, COAP_OPT_LOCATION_PATH, location_str, ' ');
    size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_NONE);
    
    return resp_len + strlen(location_str);
   
}

static ssize_t _update_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx)
{
    (void)ctx;

    char uri[CONFIG_NANOCOAP_URI_MAX] = { 0 };
    coap_get_uri_path(pdu, (uint8_t *)uri);

    printf("Received: %s\n", uri);

    unsigned method = coap_method2flag(coap_get_code_detail(pdu));
    int location_nr = extract_number_from_location(uri);

    intrusive_list_node *node_ptr = &list[location_nr - 1].node_management;
    Endpoint *endpoint_ptr = container_of(node_ptr, Endpoint, node_management);

    if (method == COAP_POST)
    {
        if (location_nr > 0 && location_nr < number_registered_endpoints)
        {
            char addr_str[IPV6_ADDR_MAX_STR_LEN];
            sock_udp_ep_t* remote = ctx->remote;
            ipv6_addr_to_str(addr_str, (ipv6_addr_t *)remote->addr.ipv6, IPV6_ADDR_MAX_STR_LEN);

            endpoint_ptr->lt = 86400;

            char base_uri[BASE_URI_MAX_LEN];
            build_base_uri_string(addr_str, base_uri);
            strncpy((char*)endpoint_ptr->base, base_uri, sizeof(endpoint_ptr->base) - 1);
            endpoint_ptr->base[sizeof(endpoint_ptr->base) - 1] = '\0';
        }
    }
    if (method == COAP_DELETE)
    {

        if (location_nr > 0 && location_nr < number_registered_endpoints)
        {
            disconnect_endpoint_from_the_rest(location_nr, node_ptr);
            delete_endpoint(location_nr);
        }

    }

    gcoap_resp_init(pdu, buf, len, COAP_CODE_CHANGED);
    coap_opt_add_format(pdu, COAP_FORMAT_LINK);
    size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_NONE);
    
    return resp_len;
}

static ssize_t _resource_lookup_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx)
{
    (void)ctx;

    uint8_t uri_query[CONFIG_NANOCOAP_URI_MAX] = { 0 };
    coap_opt_get_string(pdu, COAP_OPT_URI_QUERY, uri_query, CONFIG_NANOCOAP_URI_MAX, ',');

    static char lookup_result[LOOKUP_RESULT_STR_MAX_LEN];
    char relative_uris[RESOURCE_URI_MAX_NUMBER][RESOURCE_URI_MAX_LEN];
    int resource_number = 0;
    memset(lookup_result, 0, sizeof(lookup_result));
    memset(relative_uris, 0, sizeof(relative_uris));


    if (strstr((char*)uri_query, "href") != NULL)
    {

        char path[LOCATION_STR_MAX_LEN];
        extract_value_from_query((char*)uri_query, path, "href=");
        int location_nr = extract_number_from_location(path);

        if (location_nr <= number_registered_endpoints && location_nr > 0)
        {
            intrusive_list_node *node_ptr = &list[location_nr - 1].node_management;
            Endpoint *endpoint_ptr = container_of(node_ptr, Endpoint, node_management);
            resource_number = extract_resource_uris(endpoint_ptr->ressources, relative_uris);
            build_resource_string(resource_number, relative_uris, lookup_result, endpoint_ptr);
        }

    }
    else if(strstr((char*)uri_query, "ep") != NULL)
    {
        char ep_name[ENDPOINT_NAME_MAX_LEN];
        extract_value_from_query((char*)uri_query, ep_name, "ep=");
        Endpoint ep = find_endpoint_by_pattern(ep_name);
        resource_number = extract_resource_uris(ep.ressources, relative_uris);
        build_resource_string(resource_number, relative_uris, lookup_result, &ep);
    }
    else if(strstr((char*)uri_query, "base") != NULL)
    {
        char base_value[BASE_URI_MAX_LEN];
        extract_value_from_query((char*)uri_query, base_value, "base=");
        find_endpoints_by_pattern(base_value);
        
        for(unsigned int i = 0; i<sizeof(lookup_result_list); i++)
        {   
            if(strlen(lookup_result_list[i].name) == 0)
            {
                puts("Breaking...\n");
                break;
            }

            resource_number = extract_resource_uris(lookup_result_list[i].ressources, relative_uris);
            build_resource_string(resource_number, relative_uris, lookup_result, &lookup_result_list[i]);

        }

        return send_blockwise_response(pdu, buf, len, ctx, lookup_result);
        
    }
    else
    {
        
        intrusive_list_node *actual = head;
        Endpoint *endpoint_ptr = container_of(actual, Endpoint, node_management);
        resource_number = extract_resource_uris(endpoint_ptr->ressources, relative_uris);
        build_resource_string(resource_number, relative_uris, lookup_result, endpoint_ptr);

        do{
            if(actual->next != NULL)
            {
                strcat(lookup_result, ",");
                actual = actual->next;
                endpoint_ptr = container_of(actual, Endpoint, node_management);
                memset(relative_uris, 0, sizeof(relative_uris));
                resource_number = extract_resource_uris(endpoint_ptr->ressources, relative_uris);
                build_resource_string(resource_number, relative_uris, lookup_result, endpoint_ptr);
                
            }
            else break;
            
        }while(actual->next != NULL);

        return send_blockwise_response(pdu, buf, len, ctx, lookup_result);

    }

    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
    coap_opt_add_format(pdu, COAP_FORMAT_LINK);
    size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);

    if (pdu->payload_len >= strlen(lookup_result)) {
        memcpy(pdu->payload, lookup_result, strlen(lookup_result));
        return resp_len + strlen(lookup_result);
    }
    else {
        puts("gcoap_cli: msg buffer too small");
        return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);
    }
}


static ssize_t _endpoint_lookup_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx)
{
    (void)ctx;

    uint8_t uri_query[CONFIG_NANOCOAP_URI_MAX] = { 0 };
    coap_opt_get_string(pdu, COAP_OPT_URI_QUERY, uri_query, CONFIG_NANOCOAP_URI_MAX, ',');

    static char lookup_result[LOOKUP_RESULT_STR_MAX_LEN];
    memset(lookup_result, 0, sizeof(lookup_result));

    char* first_bracket = "<";
    char* second_href_bracket = ">;";
    char* ep_key = "ep=\"";
    char* base = "\";base=\"";
    char* rt = "\";rt=\"core.rd-ep\"";


    if (strstr((char*)uri_query, "href") != NULL)
    {
        char path[LOCATION_STR_MAX_LEN];
        extract_value_from_query((char*)uri_query, path, "href=");
        int location_nr = extract_number_from_location(path);

        if (location_nr <= number_registered_endpoints && location_nr > 0)
        {
            intrusive_list_node *node_ptr = &list[location_nr - 1].node_management;
            Endpoint *endpoint_ptr = container_of(node_ptr, Endpoint, node_management);
            build_result_string(lookup_result, first_bracket, second_href_bracket, ep_key, base, rt, endpoint_ptr);
        }

    }
    else if(strstr((char*)uri_query, "ep") != NULL)
    {
        char ep_name[ENDPOINT_NAME_MAX_LEN];
        extract_value_from_query((char*)uri_query, ep_name, "ep=");
        Endpoint ep = find_endpoint_by_pattern(ep_name);
        build_result_string(lookup_result, first_bracket, second_href_bracket, ep_key, base, rt, &ep);
    }
    else if(strstr((char*)uri_query, "base") != NULL)
    {
        char base_value[BASE_URI_MAX_LEN];
        extract_value_from_query((char*)uri_query, base_value, "base=");
        find_endpoints_by_pattern(base_value);
        
        for(unsigned int i = 0; i<sizeof(lookup_result_list); i++)
        {   
            if(strlen(lookup_result_list[i].name) == 0)
            {
                puts("Breaking...\n");
                break;
            }

            build_result_string(lookup_result, first_bracket, second_href_bracket, ep_key, base, rt, &lookup_result_list[i]);

        }

        return send_blockwise_response(pdu, buf, len, ctx, lookup_result);
        
    }
    else
    {
        
        intrusive_list_node *actual = head;
        Endpoint *endpoint_ptr = container_of(actual, Endpoint, node_management);
        build_result_string(lookup_result, first_bracket, second_href_bracket, ep_key, base, rt, endpoint_ptr);

        do{
            if(actual->next != NULL)
            {
                strcat(lookup_result, ",");
                actual = actual->next;
                endpoint_ptr = container_of(actual, Endpoint, node_management);
                build_result_string(lookup_result, first_bracket, second_href_bracket, ep_key, base, rt, endpoint_ptr);
            }
            else break;

        }while(actual->next != NULL);

        return send_blockwise_response(pdu, buf, len, ctx, lookup_result);

    }
        gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);
        coap_opt_add_format(pdu, COAP_FORMAT_LINK);
        size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);

        if (pdu->payload_len >= strlen(lookup_result)) {
            memcpy(pdu->payload, lookup_result, strlen(lookup_result));
            return resp_len + strlen(lookup_result);
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