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



typedef struct nodeelement *Ptr;

typedef struct nodeelement{
    bool next_free_middle_position;
    char base[50];
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

Endpoint lookup_result_list[100];

int number_registered_endpoints = 0;

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
    printf("Base URI: %s\n", actual->base);
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
            printf("Base URI: %s\n", actual->base);
            puts("===\n"); 
        }
        else
        {
            return 0;
        }
        


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

    Ptr actual = head;

    if (strcmp(actual->base, pattern)  == 0)
    {
        lookup_result_list[last_element_index] = *actual;
        last_element_index++;
    }
    
    do
    {
        if(actual->next != NULL)
        {
            actual = actual->next;
           
            if(strcmp(actual->base, pattern)  == 0)
            {
                lookup_result_list[last_element_index] = *actual;
                last_element_index++;
            }
        }
        
        else
        {
            break;
        }
        


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

    Ptr actual = head;

    if (strcmp(actual->name, pattern)  == 0 || strcmp(actual->location, pattern) == 0)
    {
        return *actual;
    }
    
    do
    {
        if(actual->next != NULL)
        {
            actual = actual->next;
           
            if(strcmp(actual->name, pattern)  == 0 || strcmp(actual->location, pattern) == 0)
            {
                return *actual;
            }
        }
        
        else
        {
            return empty;
        }
        


    }while(actual->next != NULL);

    return empty;


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

        if(actual->location_nr > 1)
        {
            return 1; 
        }

        if (actual->next_free_middle_position == true)
        {return actual->location_nr + 1;} 

        do
        {
            if (actual->next != NULL)
            {
                actual = actual->next;

                if(actual->next_free_middle_position == true)
                {
                    return actual->location_nr + 1;
                }
            }
            
        } while (actual->next != NULL);
    }

        return 0;

}


static ssize_t _registration_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx)
{

    char addr_str[IPV6_ADDR_MAX_STR_LEN];

    sock_udp_ep_t* remote = ctx->remote;

    
    ipv6_addr_to_str(addr_str, (ipv6_addr_t *)remote->addr.ipv6, IPV6_ADDR_MAX_STR_LEN);


    char base_uri[50];
    char* base_first = "coap://[";
    char* ending = "]";

    strcat(base_uri, base_first);
    strcat(base_uri, addr_str);
    strcat(base_uri, ending);
   
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
    char number_str[15];

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
     strncpy((char*)list[number_registered_endpoints].base, base_uri, sizeof(list[number_registered_endpoints].base) - 1);
    list[number_registered_endpoints].base[sizeof(list[number_registered_endpoints].name) - 1] = '\0';

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

        if (location > 1 && location < number_registered_endpoints)
        {
            sprintf(number_str, "%d", location);

            strcat(location_str, location_str_1);
            strcat(location_str, number_str);
            strcat(location_str, location_str_2);

            list[location - 1].lt = atoi(lifetime);
            strncpy((char*)list[location - 1].ressources, (char*)pdu->payload, sizeof(list[location - 1].ressources) - 1);
            list[location - 1].ressources[sizeof(list[location - 1].ressources) - 1] = '\0';
            strncpy((char*)list[location - 1].name, endpoint_name, sizeof(list[location - 1].name) - 1);
            list[location - 1].name[sizeof(list[location - 1].name) - 1] = '\0';
            strncpy((char*)list[location - 1].location, location_str, sizeof(list[location - 1].location) - 1);
            list[location - 1].location[sizeof(list[location - 1].location) - 1] = '\0';
            list[location - 1].location_nr = location;  
            strncpy((char*)list[number_registered_endpoints].base, base_uri, sizeof(list[number_registered_endpoints].base) - 1);
            list[number_registered_endpoints].base[sizeof(list[number_registered_endpoints].name) - 1] = '\0';

            list[location - 1].next = list[location - 2].next;
            list[location - 1].previous = &list[location - 2];
            list[location - 2].next = &list[location - 1];

            if (list[location - 1].next->location_nr > location + 1)
            {
                list[location - 1].next_free_middle_position = true;
                free_positions_in_the_middle = true;
            }
            else
            {
                list[location - 1].next_free_middle_position = false;

                if (find_next_empty(&list[location - 1]) > 0)
                {
                    free_positions_in_the_middle = true;
                }
                else 
                {
                    free_positions_in_the_middle = false;
                }

            }

            list[location - 2].next_free_middle_position = false;

        }
        else if (location == 1)
        {
            head = &list[location - 1];

            sprintf(number_str, "%d", location);

            strcat(location_str, location_str_1);
            strcat(location_str, number_str);
            strcat(location_str, location_str_2);

            list[location - 1].lt = atoi(lifetime);
            strncpy((char*)list[location - 1].ressources, (char*)pdu->payload, sizeof(list[location - 1].ressources) - 1);
            list[location - 1].ressources[sizeof(list[location - 1].ressources) - 1] = '\0';
            strncpy((char*)list[location - 1].name, endpoint_name, sizeof(list[location - 1].name) - 1);
            list[location - 1].name[sizeof(list[location - 1].name) - 1] = '\0';
            strncpy((char*)list[location - 1].location, location_str, sizeof(list[location - 1].location) - 1);
            list[location - 1].location[sizeof(list[location - 1].location) - 1] = '\0';
            list[location - 1].location_nr = location;
            strncpy((char*)list[number_registered_endpoints].base, base_uri, sizeof(list[number_registered_endpoints].base) - 1);
            list[number_registered_endpoints].base[sizeof(list[number_registered_endpoints].name) - 1] = '\0';

            list[location - 1].previous = NULL;
            list[location - 1].next = head;
            if (head->location_nr > location + 1)
            {
                list[location - 1].next_free_middle_position = true;
                free_positions_in_the_middle = true;
                
            }
            else 
            {
                list[location - 1].next_free_middle_position = false;

                 if (find_next_empty(&list[location - 1]) > 0)
                {
                    free_positions_in_the_middle = true;
                }
                else 
                {
                    free_positions_in_the_middle = false;
                }
            }

            head = &list[location - 1];

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

void extract_value_from_query(const char *input, char *href_value, char* pref) {
    const char *prefix = pref;
    const char *start = strstr(input, prefix);

    if (start) {
        start += strlen(prefix); 
        
        
        const char *end = strchr(start, ' ');
        if (!end) {
            end = input + strlen(input);
        }
        
        
        size_t length = end - start;
        strncpy(href_value, start, length);
        href_value[length] = '\0'; 
    } else {
        href_value[0] = '\0';
    }
}


static ssize_t _update_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx)
{
    (void)ctx;

    char uri[CONFIG_NANOCOAP_URI_MAX] = { 0 };

    coap_get_uri_path(pdu, (uint8_t *)uri);

    printf("Received: %s\n", uri);

    unsigned method = coap_method2flag(coap_get_code_detail(pdu));

    int location_nr = extract_number_from_location(uri);

    if (method == COAP_POST)
    {
        if (location_nr > 0 && location_nr < number_registered_endpoints)
        {
            char addr_str[IPV6_ADDR_MAX_STR_LEN];

            sock_udp_ep_t* remote = ctx->remote;

    
            ipv6_addr_to_str(addr_str, (ipv6_addr_t *)remote->addr.ipv6, IPV6_ADDR_MAX_STR_LEN);


            char base_uri[50];
            char* base_first = "coap://[";
            char* ending = "]";

            strcat(base_uri, base_first);
            strcat(base_uri, addr_str);
            strcat(base_uri, ending);

            list[location_nr - 1].lt = 86400;

            strncpy((char*)list[location_nr - 1].base, base_uri, sizeof(list[location_nr - 1].base) - 1);
            list[location_nr - 1].base[sizeof(list[location_nr - 1].name) - 1] = '\0';

        }
    }

    if (method == COAP_DELETE)
    {
        

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
                free_positions_in_the_middle = true;
            }
        }

        Endpoint empty = { 0 };

        list[location_nr - 1] = empty;

    }

    gcoap_resp_init(pdu, buf, len, COAP_CODE_CHANGED);

    
    coap_opt_add_format(pdu, COAP_FORMAT_LINK);

   
    size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_NONE);
    
    return resp_len;
}


static void build_result_string(char* lookup_result, char* first_bracket, char* second_href_bracket, char* ep_key, char* base, char* rt, Endpoint* endpoint){

    strcat(lookup_result, first_bracket);
    strcat(lookup_result, (*endpoint).location);
    strcat(lookup_result, second_href_bracket);
    strcat(lookup_result, ep_key);
    strcat(lookup_result, (*endpoint).name);
    strcat(lookup_result, base);
    strcat(lookup_result, (*endpoint).base);
    strcat(lookup_result, rt);

}

static void build_resource_string(int number_sensors, char extracted_sensor_uris[10][100], char* lookup_result, Endpoint* endpoint)
{

    for (int i = 0; i < number_sensors; i++)
    {
        strcat(lookup_result, "<");
        strcat(lookup_result, endpoint->base);
        strcat(lookup_result, extracted_sensor_uris[i]);
        strcat(lookup_result, ">");

        if(i < number_sensors - 1)
        {
            strcat(lookup_result, ",");
        }
    }
    
}

int extract_resource_uris(const char *input, char uris[10][100]) {
    int uri_count = 0;
    const char *start = input;
    const char *end;

    while ((start = strchr(start, '<')) != NULL && uri_count < 10) {
        start++; 
        end = strchr(start, '>');
        if (end == NULL) break; 

        int length = end - start;
        if (length >= 100) length = 99;

        
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


        if (offset >= strlen(lookup_result)) 
        {
            return COAP_CODE_404;
        }

        bool more = (offset + chunk_len < strlen(lookup_result));
        
        gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);

        req_block.more = more;

        coap_opt_add_block2(pdu, &resp_block, more);

        size_t result = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);

        memcpy(pdu->payload, lookup_result + offset, chunk_len);

        return result + chunk_len;
}

static ssize_t _resource_lookup_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx)
{
    (void)ctx;

    uint8_t uri_query[CONFIG_NANOCOAP_URI_MAX] = { 0 };

    coap_opt_get_string(pdu, COAP_OPT_URI_QUERY, uri_query, CONFIG_NANOCOAP_URI_MAX, ',');

    static char lookup_result[1024];

    char relative_uris[10][100];

    int resource_number = 0;
    
    memset(lookup_result, 0, sizeof(lookup_result));

    memset(relative_uris, 0, sizeof(relative_uris));


    if (strstr((char*)uri_query, "href") != NULL)
    {

        char path[50];

        extract_value_from_query((char*)uri_query, path, "href=");
        
        int location_nr = extract_number_from_location(path);

        if (location_nr <= number_registered_endpoints && location_nr > 0)
        {
            Endpoint ep = list[location_nr - 1];

            resource_number = extract_resource_uris(ep.ressources, relative_uris);

            build_resource_string(resource_number, relative_uris, lookup_result, &ep);
            
        }

    }
    else if(strstr((char*)uri_query, "ep") != NULL)
    {
        char ep_name[60];

        extract_value_from_query((char*)uri_query, ep_name, "ep=");

        Endpoint ep = find_endpoint_by_pattern(ep_name);

        resource_number = extract_resource_uris(ep.ressources, relative_uris);

        build_resource_string(resource_number, relative_uris, lookup_result, &ep);


    }
    else if(strstr((char*)uri_query, "base") != NULL)
    {
        char base_value[60];

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
        
        Ptr actual = head;

        resource_number = extract_resource_uris(actual->ressources, relative_uris);

        build_resource_string(resource_number, relative_uris, lookup_result, actual);

        do{
            if(actual->next != NULL)
            {
                strcat(lookup_result, ",");
                actual = actual->next;

                memset(relative_uris, 0, sizeof(relative_uris));

                resource_number = extract_resource_uris(actual->ressources, relative_uris);

                build_resource_string(resource_number, relative_uris, lookup_result, actual);
                
            }
            else
            {
                break;
            }


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

    static char lookup_result[1024];

    memset(lookup_result, 0, sizeof(lookup_result));

    char* first_bracket = "<";
    char* second_href_bracket = ">;";
    char* ep_key = "ep=\"";
    char* base = "\";base=\"";
    char* rt = "\";rt=\"core.rd-ep\"";


    if (strstr((char*)uri_query, "href") != NULL)
    {

        char path[50];

        extract_value_from_query((char*)uri_query, path, "href=");
        
        int location_nr = extract_number_from_location(path);

        if (location_nr <= number_registered_endpoints && location_nr > 0)
        {
            Endpoint ep = list[location_nr - 1];
            
            build_result_string(lookup_result, first_bracket, second_href_bracket, ep_key, base, rt, &ep);

        }

    }
    else if(strstr((char*)uri_query, "ep") != NULL)
    {
        char ep_name[60];

        extract_value_from_query((char*)uri_query, ep_name, "ep=");

        Endpoint ep = find_endpoint_by_pattern(ep_name);

        build_result_string(lookup_result, first_bracket, second_href_bracket, ep_key, base, rt, &ep);

    }
    else if(strstr((char*)uri_query, "base") != NULL)
    {
        char base_value[60];

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
        
        Ptr actual = head;
        
        build_result_string(lookup_result, first_bracket, second_href_bracket, ep_key, base, rt, actual);


        do{
            if(actual->next != NULL)
            {
                strcat(lookup_result, ",");
                actual = actual->next;
                
                build_result_string(lookup_result, first_bracket, second_href_bracket, ep_key, base, rt, actual);
            }
            else
            {
                break;
            }


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