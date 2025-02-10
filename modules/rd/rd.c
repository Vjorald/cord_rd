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

#include "rd.h"


i_list_node *head;

Endpoint registered_endpoints_list[REGISTERED_ENDPOINTS_MAX_NUMBER];

int deleted_registrations_list[DELETED_ENDPOINTS_MAX_NUMBER];

Endpoint lookup_result_list[LOOKUP_RESULTS_MAX_LEN];

ztimer_t lifetime_expiries[REGISTERED_ENDPOINTS_MAX_NUMBER];


int number_registered_endpoints = INITIAL_NUMBER_REGISTERED_ENDPOINTS;

int number_deleted_registrations = INITIAL_NUMBER_DELETED_ENDPOINTS;


ssize_t _registration_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx);

ssize_t _simple_registration_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx);

ssize_t _update_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx);

ssize_t _endpoint_lookup_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx);

ssize_t _resource_lookup_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx);

const coap_resource_t _resources[] = {

    
    { "/resourcedirectory/", COAP_GET | COAP_PUT | COAP_POST | COAP_MATCH_SUBTREE, _registration_handler, NULL },
    { "/.well-known/rd", COAP_GET | COAP_PUT | COAP_POST | COAP_MATCH_SUBTREE, _simple_registration_handler, NULL },
    { "/reg/",  COAP_PUT | COAP_POST | COAP_DELETE | COAP_MATCH_SUBTREE , _update_handler, NULL },
    { "/endpoint-lookup/",  COAP_GET | COAP_MATCH_SUBTREE , _endpoint_lookup_handler, NULL },
    { "/resource-lookup/",  COAP_GET | COAP_MATCH_SUBTREE , _resource_lookup_handler, NULL}
};

gcoap_listener_t _listener = {
    _resources,
    ARRAY_SIZE(_resources),
    GCOAP_SOCKET_TYPE_UDP,
    NULL,
    NULL,
    NULL
};

void resource_directory_init(void){

    gcoap_register_listener(&_listener);
    
}


void append_endpoint(i_list_node *new_node){

    i_list_node* previous = NULL;

    if (head != NULL)
    {
        int previous_location = get_previous_endpoint_location(new_node->location_nr);
        previous = &registered_endpoints_list[previous_location - 1].node_management;
    }

    append_list_entry(&head, &new_node, &previous);

}

void connect_endpoint_with_the_rest(i_list_node *node_ptr, int location_nr)
{
    if (number_deleted_registrations == INITIAL_NUMBER_DELETED_ENDPOINTS) append_endpoint(node_ptr);
    else
    {
        if (location_nr > 1 && location_nr < number_registered_endpoints)
        {
            int previous_endpoint_location = get_previous_endpoint_location(node_ptr->location_nr);
            i_list_node *previous_node = &registered_endpoints_list[previous_endpoint_location - 1].node_management;

            add_list_entry_in_the_middle(&node_ptr, &previous_node);
        }
        else if (location_nr == 1)
        {
            add_list_entry_at_the_beginning(&head, &node_ptr);
        }
    }
}

void disconnect_endpoint_from_the_rest(int location_nr, i_list_node *node_ptr){

    (void) location_nr;

    remove_list_entry(&head, &node_ptr);
}

void delete_endpoint(int location_nr) {

    if (number_deleted_registrations < DELETED_ENDPOINTS_MAX_NUMBER)
    {
        number_deleted_registrations++;
        deleted_registrations_list[number_deleted_registrations - 1] = location_nr;

        Endpoint empty = { 0 };
        registered_endpoints_list[location_nr - 1] = empty;

        if (location_nr == number_registered_endpoints) number_registered_endpoints--;
    }

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

    i_list_node *actual = head;
    Endpoint *endpoint_ptr = container_of(actual, Endpoint, node_management);
    char location_str[LOCATION_STR_MAX_LEN] = "";
    build_location_string(actual->location_nr, location_str);

    printf("Endpoint: %s\n", endpoint_ptr->name);
    printf("Lifetime: %d\n", endpoint_ptr->lt);
    printf("Resources: %s\n", endpoint_ptr->resources);
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
            printf("Resources: %s\n", endpoint_ptr->resources);
            printf("Location: %s\n", location_str);
            printf("Base URI: %s\n", endpoint_ptr->base);
            puts("===\n"); 
            
        }
        else return 0;

    }while(actual->next != NULL);

    return 0;

}

void parse_query_buffer(unsigned char *query_buffer, char *ep, char *lt, char *et, char *sector, char *base) {
   
    char *token = strtok((char *)query_buffer, " ");
    
    while (token != NULL) {
        if (strncmp(token, "ep=", 3) == 0) strcpy(ep, token + 3);
        else if (strncmp(token, "lt=", 3) == 0) strcpy(lt, token + 3);
        else if (strncmp(token, "et=", 3) == 0) strcpy(et, token + 3);
        else if (strncmp(token, "d=", 2) == 0) strcpy(sector, token + 2);
        else if (strncmp(token, "base=", 5) == 0) strcpy(base, token + 5);
        token = strtok(NULL, " ");
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

    strcpy(base_uri, base_first);
    strcat(base_uri, addr_str);
    strcat(base_uri, ending);
}

void get_all_registered_endpoints(void){

    memset(lookup_result_list, 0, sizeof(lookup_result_list));
    int last_element_index = 0;

    if (head == NULL)
    {
        puts("There are no registered endpoints.");
        return ;
    }

    i_list_node *actual = head;
    Endpoint *endpoint_ptr = container_of(actual, Endpoint, node_management);


    lookup_result_list[last_element_index] = *endpoint_ptr;
    last_element_index++;
    
    do
    {
        if(actual->next != NULL)
        {
            actual = actual->next;
            endpoint_ptr = container_of(actual, Endpoint, node_management);
           
            lookup_result_list[last_element_index] = *endpoint_ptr;
            last_element_index++;
            
        }
        else break; 

    }while(actual->next != NULL);

    return ;
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

    i_list_node *actual = head;
    Endpoint *endpoint_ptr = container_of(actual, Endpoint, node_management);

    if (strcmp(endpoint_ptr->base, pattern)  == 0 || strcmp(endpoint_ptr->et, pattern)  == 0)
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
           
            if(strcmp(endpoint_ptr->base, pattern)  == 0 || strcmp(endpoint_ptr->et, pattern)  == 0)
            {
                lookup_result_list[last_element_index] = *endpoint_ptr;
                last_element_index++;
            }
        }
        else break; 

    }while(actual->next != NULL);

    return ;

}

Endpoint *find_endpoint_by_pattern(char* pattern)
{

    char base_uri[BASE_URI_MAX_LEN] = { 0 };

    if (head == NULL)
    {
        puts("There are no registered endpoints.");
        return NULL;
    }


    i_list_node *actual = head;
    Endpoint *endpoint_ptr = container_of(actual, Endpoint, node_management);
    char location_str[LOCATION_STR_MAX_LEN] = "";
    build_location_string(actual->location_nr, location_str);

    if(endpoint_ptr->epsim == true){
        memset(base_uri, 0, sizeof(base_uri));
        
        build_base_uri_string(pattern, base_uri);
        if(strcmp(base_uri, endpoint_ptr->base)  == 0){
            return endpoint_ptr;
        }
    }

    if (strcmp(endpoint_ptr->name, pattern)  == 0 || strcmp(location_str, pattern) == 0) return endpoint_ptr;
    
    do
    {
        if(actual->next != NULL)
        {
            actual = actual->next;
            endpoint_ptr = container_of(actual, Endpoint, node_management);
            memset(location_str, 0, sizeof(location_str));
            build_location_string(actual->location_nr, location_str);

            if(endpoint_ptr->epsim == true){
                memset(base_uri, 0, sizeof(base_uri));
                
                build_base_uri_string(pattern, base_uri);
                if(strcmp(base_uri, endpoint_ptr->base)  == 0){
                    return endpoint_ptr;
                }
            }
            
            if (strcmp(endpoint_ptr->name, pattern)  == 0 || strcmp(location_str, pattern) == 0) return endpoint_ptr;
        }
        else return NULL;
        
    }while(actual->next != NULL);

    return NULL;

}

void lifetime_callback(void *argument)
{
    i_list_node *node_ptr = &registered_endpoints_list[*(int*)argument - 1].node_management;

    printf("Location nummer: %d\n", *(int*)argument);

    disconnect_endpoint_from_the_rest(node_ptr->location_nr, node_ptr);
    delete_endpoint(node_ptr->location_nr);

    Endpoint *endpoint_ptr = container_of(node_ptr, Endpoint, node_management);

    printList(endpoint_ptr);
}


void initialize_endpoint(char *lifetime, char *endpoint_name, Endpoint *endpoint_ptr, i_list_node *node_ptr, char *base_uri, char *payload, int *payload_len, char* location_str, int location_nr, char *et, char *sector)
{
    build_location_string(location_nr, location_str);

    endpoint_ptr->lt = atoi(lifetime); 

    if (*payload_len > 0){
        strncpy((char*)endpoint_ptr->resources, payload, *payload_len);
    }

    strncpy((char*)endpoint_ptr->name, endpoint_name, sizeof(endpoint_ptr->name) - 1);

    node_ptr->location_nr = location_nr;

    if(strlen(et) > 0)
    {
        strncpy((char*)endpoint_ptr->et, et, sizeof(endpoint_ptr->et) - 1);
    }
    else
    {
        strncpy((char*)endpoint_ptr->et, "core.rd-ep", sizeof(endpoint_ptr->et) - 1);
    }

    if (strlen(sector) > 0)
    {
        strncpy((char*)endpoint_ptr->sector, sector, sizeof(endpoint_ptr->sector) - 1);
    }
    else
    {
        strncpy((char*)endpoint_ptr->sector, "default", sizeof(endpoint_ptr->sector) - 1);
    }

    strncpy((char*)endpoint_ptr->base, base_uri, sizeof(endpoint_ptr->base) - 1);

    if(endpoint_ptr->epsim == false){

        lifetime_expiries[location_nr - 1].callback = lifetime_callback; 
        lifetime_expiries[location_nr - 1].arg = &(node_ptr->location_nr);             
        ztimer_set(ZTIMER_SEC, &lifetime_expiries[location_nr - 1], endpoint_ptr->lt);

    }
    
}


int get_next_empty_location(void)
{

    int location_nr = deleted_registrations_list[0];

    for (int i = 0; i < number_deleted_registrations; i++) deleted_registrations_list[i] = deleted_registrations_list[i + 1];
 
    deleted_registrations_list[number_deleted_registrations - 1] = 0;
    number_deleted_registrations--; 

    return location_nr;

}

int get_previous_endpoint_location(int actual_location)
{
    for (int i = actual_location - 2; i >= 0; i--) if(strlen(registered_endpoints_list[i].name) > 0) return registered_endpoints_list[i].node_management.location_nr;

    return -1;
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

void extract_value_from_query(const char *input, char *query_value, char* pref) {
    const char *prefix = pref;
    const char *start = strstr(input, prefix);

    if (start) {
        start += strlen(prefix); 
        
        const char *end = strchr(start, ' ');

        if (!end) end = input + strlen(input);
        
        size_t length = end - start;
        strncpy(query_value, start, length);
        query_value[length] = '\0'; 
    } 
    else query_value[0] = '\0';
}

int check_existing_endpoint(char *ep_name, char* sector){


    if (head == NULL)
    {
        puts("There are no registered endpoints.");
        return -1;
    }


    i_list_node *actual = head;
    Endpoint *endpoint_ptr = container_of(actual, Endpoint, node_management);

    if (strcmp(endpoint_ptr->name, ep_name) == 0 && strcmp(endpoint_ptr->sector, sector) == 0) return actual->location_nr;
    
    do
    {
        if(actual->next != NULL)
        {
            actual = actual->next;
            endpoint_ptr = container_of(actual, Endpoint, node_management);

            if (strcmp(endpoint_ptr->name, ep_name) == 0 && strcmp(endpoint_ptr->sector, sector) == 0) return actual->location_nr;
        }
        else return -1;
        
    }while(actual->next != NULL);

    return -1;
}

void build_result_string(char* lookup_result, char* first_bracket, char* second_href_bracket, char* ep_key, char* base, char* rt, Endpoint* endpoint, char *et){

    char location_str[LOCATION_STR_MAX_LEN] = "";

    build_location_string(endpoint->node_management.location_nr, location_str);

    strcat(lookup_result, first_bracket);
    strcat(lookup_result, location_str);
    strcat(lookup_result, second_href_bracket);
    strcat(lookup_result, ep_key);
    strcat(lookup_result, (*endpoint).name);
    strcat(lookup_result, base);
    strcat(lookup_result, (*endpoint).base);
    if (strlen(et) > 0)
    {
        strcat(lookup_result, rt);
        strcat(lookup_result, et);
        strcat(lookup_result, "\",");
    }
    

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

void build_whole_result_string(uint8_t *uri_query, char *lookup_result, char *first_bracket, char *second_href_bracket, char *ep_key,
                                char *base, char *rt, char relative_uris[RESOURCE_URI_MAX_NUMBER][RESOURCE_URI_MAX_LEN], int *resource_number){
    if(strstr((char*)uri_query, "page") != NULL){

        if(strstr((char*)uri_query, "count") == NULL)
        {
            return ;
        }

        char page_str[MAX_PAGE_DIGIT_NUMBER] = { 0 };
        char count_str[MAX_COUNT_DIGIT_NUMBER] = { 0 };
        extract_value_from_query((char*)uri_query, page_str, "page=");
        extract_value_from_query((char*)uri_query, count_str, "count="); 

        int page_value = atoi(page_str);
        int count_value = atoi(count_str);

        int first_value = page_value * count_value;
        int last_value = count_value * (page_value + 1);

        for(int i = first_value; i < last_value; i++){

            if(strlen(lookup_result_list[i].name) == 0)
            {
                puts("Breaking...\n");
                break;
            }

            if(relative_uris){
                *resource_number = extract_resource_uris(lookup_result_list[i].resources, relative_uris);
                build_resource_string(*resource_number, relative_uris, lookup_result, &lookup_result_list[i]);
            }
            else{
                build_result_string(lookup_result, first_bracket, second_href_bracket, ep_key, base, rt, &lookup_result_list[i], lookup_result_list[i].et);
            }
           
        }
    }
    else{

        for(unsigned int i = 0; i<sizeof(lookup_result_list); i++)
        {   
            if(strlen(lookup_result_list[i].name) == 0)
            {
                puts("Breaking...\n");
                break;
            }

            if(relative_uris){
                *resource_number = extract_resource_uris(lookup_result_list[i].resources, relative_uris);
                build_resource_string(*resource_number, relative_uris, lookup_result, &lookup_result_list[i]);
            }
            else{
                build_result_string(lookup_result, first_bracket, second_href_bracket, ep_key, base, rt, &lookup_result_list[i], lookup_result_list[i].et);
            }

        }
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

size_t send_blockwise_response(coap_pkt_t *pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx, char* lookup_result)
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

void epsim_get_request_callback(void* argument){ 

    int location_nr = *(int *)argument;

    i_list_node *node_ptr = &registered_endpoints_list[location_nr - 1].node_management;
    Endpoint *endpoint_ptr = container_of(node_ptr, Endpoint, node_management);

    bool cache_expired = nanocoap_cache_entry_is_stale(endpoint_ptr->cache, ztimer_now(ZTIMER_SEC));

    if(endpoint_ptr->epsim == true && cache_expired == true){
        send_get_request(endpoint_ptr);
    }
    else{

        lifetime_expiries[location_nr - 1].callback = epsim_get_request_callback; 
        lifetime_expiries[location_nr - 1].arg = &(node_ptr->location_nr);             
        ztimer_set(ZTIMER_SEC, &lifetime_expiries[location_nr - 1], 0.75*endpoint_ptr->lt);
    }

}

void _resp_handler(const gcoap_request_memo_t *memo,
                          coap_pkt_t *pdu,
                          const sock_udp_ep_t *remote) {
    (void)memo; 

    char addr_str[IPV6_ADDR_MAX_STR_LEN] = { 0 };
    ipv6_addr_to_str(addr_str, (ipv6_addr_t *)remote->addr.ipv6, IPV6_ADDR_MAX_STR_LEN);

    printf("Remote address %s\n", addr_str);

    printf("Received: %s\n", pdu->payload);

    Endpoint *endpoint_ptr = find_endpoint_by_pattern(addr_str);

    nanocoap_cache_entry_t *cache = nanocoap_cache_add_by_req(&endpoint_ptr->epsim_pkt, pdu, pdu->options_len + pdu->payload_len);

    printf("Cache buf: %s\n", cache->response_buf);
    
    strncpy((char*)endpoint_ptr->resources, (char*)pdu->payload, sizeof(endpoint_ptr->resources) - 1);

    lifetime_expiries[endpoint_ptr->node_management.location_nr - 1].callback = epsim_get_request_callback;
    lifetime_expiries[endpoint_ptr->node_management.location_nr - 1].arg      = &endpoint_ptr->node_management.location_nr;
    ztimer_set(ZTIMER_SEC, &lifetime_expiries[endpoint_ptr->node_management.location_nr - 1], 0.75*endpoint_ptr->lt);

    printList(&registered_endpoints_list[number_registered_endpoints - 1]);

    sock_udp_close(&endpoint_ptr->epsim_sock);

    ztimer_sleep(ZTIMER_SEC, 1);

}

void send_get_request(Endpoint *endpoint_ptr) {

    sock_udp_ep_t local = { .family = AF_INET6 };
    coap_request_ctx_t ctx = { .remote = endpoint_ptr->remote };

    uint8_t _epsim_req_buf[CONFIG_GCOAP_PDU_BUF_SIZE] = { 0 };

    if (sock_udp_str2ep(&local, "[fe80::cafe:cafe:cafe:1]:5683") < 0) {
        puts("Unable to parse destination address");
    }

    if(sock_udp_create(&endpoint_ptr->epsim_sock, &local, NULL, 0) < 0) {
        printf("Sock creation unsuccessful!");
    }


    gcoap_req_init(&endpoint_ptr->epsim_pkt, _epsim_req_buf, CONFIG_GCOAP_PDU_BUF_SIZE, COAP_METHOD_GET, "/.well-known/core");
    coap_hdr_set_type(endpoint_ptr->epsim_pkt.hdr, COAP_TYPE_CON);
    int len = coap_opt_finish(&endpoint_ptr->epsim_pkt, COAP_OPT_FINISH_PAYLOAD);

    memset(endpoint_ptr->epsim_pkt.payload, 0, endpoint_ptr->epsim_pkt.payload_len);
    
    int result = gcoap_req_send(_epsim_req_buf, len, endpoint_ptr->remote, &local, _resp_handler, &ctx, GCOAP_SOCKET_TYPE_UDP);
    
    if (result <= 0) {
        puts("Failed to send request");
        printf("Result: %d\n", result);
    } else {
        puts("GET request sent successfully\n");
        printf("Result: %d\n", result);
    }

    ztimer_sleep(ZTIMER_SEC, 1);

}

void update_endpoint(char *payload, int *payload_len, unsigned char *query_buffer, Endpoint *endpoint_ptr, char *addr_str){

    char endpoint_name[ENDPOINT_NAME_MAX_LEN] = { 0 };
    char lifetime[LIFETIME_STR_MAX_LEN] = { 0 };
    char resources[RESOURCES_MAX_LEN] = { 0 };
    char endpoint_type[ENDPOINT_TYPE_MAX_LEN] = { 0 };
    char sector[SECTOR_NAME_MAX_LEN] = { 0 };
    char base_uri[BASE_URI_MAX_LEN] = { 0 };
    strncpy(resources, payload, *payload_len);

    unsigned char rep_query_buffer[QUERY_BUFFER_MAX_LEN] = { 0 };
    memcpy(rep_query_buffer, query_buffer, QUERY_BUFFER_MAX_LEN);

    parse_query_buffer(rep_query_buffer, endpoint_name, lifetime, endpoint_type, sector, base_uri);

    if(strlen(endpoint_name) > 0){
        
        memset(endpoint_ptr->name, 0, ENDPOINT_NAME_MAX_LEN);
        strncpy(endpoint_ptr->name, endpoint_name, strlen(endpoint_name));
    }

    if (strlen(lifetime) > 0){
        endpoint_ptr->lt = atoi(lifetime);

    }
    else{
        endpoint_ptr->lt = 90000;

    }

    if (endpoint_ptr->epsim == true){
        lifetime_expiries[endpoint_ptr->node_management.location_nr - 1].callback = epsim_get_request_callback;
        lifetime_expiries[endpoint_ptr->node_management.location_nr - 1].arg      = &endpoint_ptr->node_management.location_nr;
        ztimer_set(ZTIMER_SEC, &lifetime_expiries[endpoint_ptr->node_management.location_nr - 1], 0.75*endpoint_ptr->lt);
    }
    else{
        lifetime_expiries[endpoint_ptr->node_management.location_nr - 1].callback = lifetime_callback;
        lifetime_expiries[endpoint_ptr->node_management.location_nr - 1].arg      = &endpoint_ptr->node_management.location_nr;
        ztimer_set(ZTIMER_SEC, &lifetime_expiries[endpoint_ptr->node_management.location_nr - 1], endpoint_ptr->lt);
    }

    if (*payload_len > 0){

        memset(endpoint_ptr->resources, 0, RESOURCES_MAX_LEN);
        strncpy(endpoint_ptr->resources, resources, strlen(resources));
    }

    if (strlen(endpoint_type) > 0){

        memset(endpoint_ptr->et, 0, ENDPOINT_TYPE_MAX_LEN);
        strncpy(endpoint_ptr->et, endpoint_type, strlen(endpoint_type));
    }

    if (strlen(sector) > 0){

        memset(endpoint_ptr->sector, 0, SECTOR_NAME_MAX_LEN);
        strncpy(endpoint_ptr->sector, sector, strlen(sector));
    }

    if (strlen(base_uri) > 0){

        memset(endpoint_ptr->base, 0, BASE_URI_MAX_LEN);
        strncpy(endpoint_ptr->base, base_uri, strlen(base_uri));
    }
    else{
        
        memset(endpoint_ptr->base, 0, BASE_URI_MAX_LEN);
        build_base_uri_string(addr_str, base_uri);
        strncpy(endpoint_ptr->base, base_uri, strlen(base_uri));
    }
}

int register_endpoint(char *addr_str, unsigned char *query_buffer, char *location_str, char *payload, int *payload_len){

    i_list_node *node_ptr;
    Endpoint *endpoint_ptr;

    char endpoint_name[ENDPOINT_NAME_MAX_LEN] = { 0 };
    char lifetime[LIFETIME_STR_MAX_LEN] = { 0 };
    char resources[RESOURCES_MAX_LEN] = { 0 };
    char endpoint_type[ENDPOINT_TYPE_MAX_LEN] = { 0 };
    char sector[SECTOR_NAME_MAX_LEN] = { 0 };
    char base_uri[BASE_URI_MAX_LEN] = { 0 };
    strncpy(resources, payload, *payload_len);

    unsigned char rep_query_buffer[QUERY_BUFFER_MAX_LEN] = { 0 };
    memcpy(rep_query_buffer, query_buffer, QUERY_BUFFER_MAX_LEN);

    parse_query_buffer(rep_query_buffer, endpoint_name, lifetime, endpoint_type, sector, base_uri);

    if (strlen(base_uri) == 0){
        build_base_uri_string(addr_str, base_uri);
    }
    
    puts("======== Request infos: =======\n");
    printf("Endpoint: %s\n", endpoint_name);
    printf("Lifetime: %s\n", lifetime);
    printf("Resources: %s\n", resources);
    puts("\n");
    
    int location_nr = -1;
    int location_existing = -1;
    
    if(strlen(sector) == 0){
        location_existing = check_existing_endpoint(endpoint_name, "default");
    }
    else{
        location_existing = check_existing_endpoint(endpoint_name, sector);
    }

    if (location_existing != -1){
        node_ptr = &registered_endpoints_list[location_existing - 1].node_management;
        endpoint_ptr = container_of(node_ptr, Endpoint, node_management);
        location_nr = location_existing;
    }
    else{
        if (number_deleted_registrations == INITIAL_NUMBER_DELETED_ENDPOINTS)
        {
            node_ptr = &registered_endpoints_list[number_registered_endpoints].node_management;
            endpoint_ptr = container_of(node_ptr, Endpoint, node_management);
            number_registered_endpoints++;
            location_nr = number_registered_endpoints;
        }
        else
        {
            location_nr = get_next_empty_location();
            node_ptr = &registered_endpoints_list[location_nr - 1].node_management;
            endpoint_ptr = container_of(node_ptr, Endpoint, node_management);
        }
    }

    memset(location_str, 0, LOCATION_STR_MAX_LEN);
    initialize_endpoint(lifetime, endpoint_name, endpoint_ptr, node_ptr, base_uri, payload, payload_len, location_str, location_nr, endpoint_type, sector);
    if (location_existing == -1){
        connect_endpoint_with_the_rest(node_ptr, node_ptr->location_nr);
    }
    

    return location_nr;
}

ssize_t _registration_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx)
{

    char location_str[LOCATION_STR_MAX_LEN] = { 0 };

    char addr_str[IPV6_ADDR_MAX_STR_LEN] = { 0 };
    sock_udp_ep_t* remote = ctx->remote;
    ipv6_addr_to_str(addr_str, (ipv6_addr_t *)remote->addr.ipv6, IPV6_ADDR_MAX_STR_LEN);

    unsigned char query_buffer[QUERY_BUFFER_MAX_LEN] = { 0 };
    int result = coap_opt_get_string(pdu, COAP_OPT_URI_QUERY, query_buffer, QUERY_BUFFER_MAX_LEN, ' ');
    printf("Options: %s\n", query_buffer);

    (void) result;

    int payload_len = pdu->payload_len;
    
    register_endpoint(addr_str, query_buffer, location_str, (char *)pdu->payload, &payload_len);

    printList(&registered_endpoints_list[number_registered_endpoints - 1]);
    
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CREATED);
    coap_opt_add_string(pdu, COAP_OPT_LOCATION_PATH, location_str, ' ');
    size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);

    memset(pdu->payload, 0, pdu->payload_len);
    
    return resp_len + strlen(location_str);
   
}


ssize_t _simple_registration_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx){
   
    char location_str[LOCATION_STR_MAX_LEN] = { 0 };

    char addr_str[IPV6_ADDR_MAX_STR_LEN] = { 0 };
    sock_udp_ep_t* remote = ctx->remote;
    ipv6_addr_to_str(addr_str, (ipv6_addr_t *)remote->addr.ipv6, IPV6_ADDR_MAX_STR_LEN);

    unsigned char query_buffer[QUERY_BUFFER_MAX_LEN] = { 0 };
    int result = coap_opt_get_string(pdu, COAP_OPT_URI_QUERY, query_buffer, QUERY_BUFFER_MAX_LEN, ' ');
    printf("Options: %s\n", query_buffer);

    (void) result;

    int payload_len = pdu->payload_len;

    int location_nr = register_endpoint(addr_str, query_buffer, location_str, (char *)pdu->payload, &payload_len);

    i_list_node *node_ptr = &registered_endpoints_list[location_nr - 1].node_management;
    Endpoint *endpoint_ptr = container_of(node_ptr, Endpoint, node_management);

    endpoint_ptr->epsim = true;
    endpoint_ptr->remote = remote;

    gcoap_resp_init(pdu, buf, len, COAP_CODE_CHANGED);
    coap_opt_add_accept(pdu, COAP_FORMAT_LINK);
    size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);

    memset(pdu->payload, 0, pdu->payload_len);

    send_get_request(endpoint_ptr);

    if (strlen(endpoint_ptr->resources) == 0){
        disconnect_endpoint_from_the_rest(endpoint_ptr->node_management.location_nr, &endpoint_ptr->node_management);
        delete_endpoint(endpoint_ptr->node_management.location_nr);

        return gcoap_response(pdu, buf, len, COAP_CODE_REQUEST_ENTITY_INCOMPLETE);
    }

    return resp_len;

}


ssize_t _update_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx)
{

    char uri[CONFIG_NANOCOAP_URI_MAX] = { 0 };
    coap_get_uri_path(pdu, (uint8_t *)uri);

    printf("Received: %s\n", uri);

    unsigned method = coap_method2flag(coap_get_code_detail(pdu));
    int location_nr = extract_number_from_location(uri);

    i_list_node *node_ptr = &registered_endpoints_list[location_nr - 1].node_management;
    Endpoint *endpoint_ptr = container_of(node_ptr, Endpoint, node_management);

    if (method == COAP_POST)
    {
        if (location_nr > 0 && location_nr < number_registered_endpoints)
        {
            char addr_str[IPV6_ADDR_MAX_STR_LEN] = { 0 };
            sock_udp_ep_t* remote = ctx->remote;
            ipv6_addr_to_str(addr_str, (ipv6_addr_t *)remote->addr.ipv6, IPV6_ADDR_MAX_STR_LEN);

            unsigned char query_buffer[QUERY_BUFFER_MAX_LEN] = { 0 };
            int result = coap_opt_get_string(pdu, COAP_OPT_URI_QUERY, query_buffer, QUERY_BUFFER_MAX_LEN, ' ');
            printf("Options: %s\n", query_buffer);

            (void) result;

            int payload_len = pdu->payload_len;

            update_endpoint((char *)pdu->payload, &payload_len, query_buffer, endpoint_ptr, addr_str);
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

ssize_t _resource_lookup_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx)
{
    (void)ctx;

    uint8_t uri_query[CONFIG_NANOCOAP_URI_MAX] = { 0 };
    coap_opt_get_string(pdu, COAP_OPT_URI_QUERY, uri_query, CONFIG_NANOCOAP_URI_MAX, ' ');

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
            i_list_node *node_ptr = &registered_endpoints_list[location_nr - 1].node_management;
            Endpoint *endpoint_ptr = container_of(node_ptr, Endpoint, node_management);
            resource_number = extract_resource_uris(endpoint_ptr->resources, relative_uris);
            build_resource_string(resource_number, relative_uris, lookup_result, endpoint_ptr);
        }

    }
    else if(strstr((char*)uri_query, "ep") != NULL)
    {
        char ep_name[ENDPOINT_NAME_MAX_LEN];
        extract_value_from_query((char*)uri_query, ep_name, "ep=");
        Endpoint *ep = find_endpoint_by_pattern(ep_name);
        resource_number = extract_resource_uris(ep->resources, relative_uris);
        build_resource_string(resource_number, relative_uris, lookup_result, ep);
    }
    else if(strstr((char*)uri_query, "base") != NULL)
    {
        char base_value[BASE_URI_MAX_LEN];
        extract_value_from_query((char*)uri_query, base_value, "base=");
        find_endpoints_by_pattern(base_value);
        
        build_whole_result_string(uri_query, lookup_result, NULL, NULL, NULL, NULL,
                                    NULL, relative_uris, &resource_number);

        return send_blockwise_response(pdu, buf, len, ctx, lookup_result);
        
    }
    else if(strstr((char*)uri_query, "et") != NULL)
    {
        char endpoint_type[ENDPOINT_TYPE_MAX_LEN] = { 0 };
        extract_value_from_query((char*)uri_query, endpoint_type, "et=");
        find_endpoints_by_pattern(endpoint_type);

        build_whole_result_string(uri_query, lookup_result, NULL, NULL, NULL, NULL,
                                    NULL, relative_uris, &resource_number);

        return send_blockwise_response(pdu, buf, len, ctx, lookup_result);

    }
    else
    {
        get_all_registered_endpoints();

        build_whole_result_string(uri_query, lookup_result, NULL, NULL, NULL, NULL,
                                    NULL, relative_uris, &resource_number);
 
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


ssize_t _endpoint_lookup_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx)
{
    (void)ctx;

    uint8_t uri_query[CONFIG_NANOCOAP_URI_MAX] = { 0 };
    coap_opt_get_string(pdu, COAP_OPT_URI_QUERY, uri_query, CONFIG_NANOCOAP_URI_MAX, ' ');

    static char lookup_result[LOOKUP_RESULT_STR_MAX_LEN];
    memset(lookup_result, 0, sizeof(lookup_result));

    char* first_bracket = "<";
    char* second_href_bracket = ">;";
    char* ep_key = "ep=\"";
    char* base = "\";base=\"";
    char* rt = "\";rt=\"";


    if (strstr((char*)uri_query, "href") != NULL)
    {
        char path[LOCATION_STR_MAX_LEN];
        extract_value_from_query((char*)uri_query, path, "href=");
        int location_nr = extract_number_from_location(path);

        if (location_nr <= number_registered_endpoints && location_nr > 0)
        {
            i_list_node *node_ptr = &registered_endpoints_list[location_nr - 1].node_management;
            Endpoint *endpoint_ptr = container_of(node_ptr, Endpoint, node_management);
            build_result_string(lookup_result, first_bracket, second_href_bracket, ep_key, base, rt, endpoint_ptr, endpoint_ptr->et);
        }

    }
    else if(strstr((char*)uri_query, "ep") != NULL)
    {
        char ep_name[ENDPOINT_NAME_MAX_LEN];
        extract_value_from_query((char*)uri_query, ep_name, "ep=");
        Endpoint *ep = find_endpoint_by_pattern(ep_name);
        build_result_string(lookup_result, first_bracket, second_href_bracket, ep_key, base, rt, ep, ep->et);
    }
    else if(strstr((char*)uri_query, "base") != NULL)
    {
        char base_value[BASE_URI_MAX_LEN];
        extract_value_from_query((char*)uri_query, base_value, "base=");
        find_endpoints_by_pattern(base_value);

        build_whole_result_string(uri_query, lookup_result, first_bracket, second_href_bracket, ep_key, base,
                                    rt, NULL, NULL);

        return send_blockwise_response(pdu, buf, len, ctx, lookup_result);
        
    }
    else if(strstr((char*)uri_query, "et") != NULL)
    {
        char endpoint_type[ENDPOINT_TYPE_MAX_LEN] = { 0 };
        extract_value_from_query((char*)uri_query, endpoint_type, "et=");
        find_endpoints_by_pattern(endpoint_type);

        build_whole_result_string(uri_query, lookup_result, first_bracket, second_href_bracket, ep_key, base,
                                    rt, NULL, NULL);

        return send_blockwise_response(pdu, buf, len, ctx, lookup_result);

    }
    else
    {
        
        get_all_registered_endpoints();

        build_whole_result_string(uri_query, lookup_result, first_bracket, second_href_bracket, ep_key, base,
                                    rt, NULL, NULL);
           

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