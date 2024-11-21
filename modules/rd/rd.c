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
#include "i_list.h"


ssize_t _registration_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx);

ssize_t _update_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx);

ssize_t _endpoint_lookup_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx);

ssize_t _resource_lookup_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx);

const coap_resource_t _resources[] = {

    { "/resourcedirectory/", COAP_GET | COAP_PUT | COAP_POST , _registration_handler, NULL },
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

ssize_t _registration_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx)
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

ssize_t _update_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx)
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

ssize_t _resource_lookup_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx)
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


ssize_t _endpoint_lookup_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx)
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