#include "time_benchmark.h"

const char *base_strings[] = {" lt=570 ", " lt=800 et=oic.r.glucose.medication ", " lt=1200 d=special "};

size_t base_count = sizeof(base_strings) / sizeof(base_strings[0]);

unsigned char query_list[QUERY_LIST_SIZE][QUERY_BUFFER_MAX_LEN] = { 0 };

char IPv6_address_list[IPV6_LIST_SIZE][IPV6_ADDR_MAX_STR_LEN] = { 0 };

static char lookup_result[LOOKUP_RESULT_STR_MAX_LEN] = { 0 };

char relative_uris[RESOURCE_URI_MAX_NUMBER][RESOURCE_URI_MAX_LEN] = { 0 };

char location_str[LOCATION_STR_MAX_LEN] = { 0 };

char addr_str[IPV6_ADDR_MAX_STR_LEN] = "fe80::cafe:cafe:cafe:5";

uint8_t *payload = (uint8_t*)"<resource-link-1>,<resource-link-2>";

uint16_t payload_len = sizeof((char *)payload);

int runs = NUMBER_CYCLES_BENCHMARK;

int start, end;

int total_time = 0;

i_list_node *node_ptr, *next_expiring_node;

Endpoint *endpoint_ptr, *next_expiring_endpoint;


void benchmark_sequential_registrations(void){

    memset(registered_endpoints_list, 0, sizeof(registered_endpoints_list));
    memset(deleted_registrations_list, 0, sizeof(deleted_registrations_list));
    memset(lookup_result, 0, sizeof(lookup_result));
    memset(relative_uris, 0, sizeof(relative_uris));
    memset(query_list, 0, sizeof(query_list));
    memset(location_str, 0, sizeof(location_str));
    head = NULL;
    number_registered_endpoints = 0;
    number_deleted_registrations = 0;
    
    random_init(0); // Initialize RIOT's random system
    generate_query_list(query_list, base_strings, base_count);

    total_time = 0;

    for (int i = 0; i < runs; i++) {
        
        start = ztimer_now(ZTIMER_USEC);

        register_endpoint(addr_str, query_list[i], location_str, payload, &payload_len);

        end   = ztimer_now(ZTIMER_USEC);

        if (runs % (REGISTERED_ENDPOINTS_MAX_NUMBER - 1) == 0){
            memset(registered_endpoints_list, 0, sizeof(registered_endpoints_list));
            memset(deleted_registrations_list, 0, sizeof(deleted_registrations_list));
            head = NULL;
            number_registered_endpoints = 0;
            number_deleted_registrations = 0;
        }

        memset(location_str, 0, sizeof(location_str));

        total_time += (end - start);
    }

    printf("Average execution time for sequential registrations: %d microseconds\n", total_time / runs);

}

void benchmark_idempotent_last_registration_existing(void){

    memset(registered_endpoints_list, 0, sizeof(registered_endpoints_list));
    memset(deleted_registrations_list, 0, sizeof(deleted_registrations_list));
    memset(lookup_result, 0, sizeof(lookup_result));
    memset(relative_uris, 0, sizeof(relative_uris));
    memset(query_list, 0, sizeof(query_list));
    memset(location_str, 0, sizeof(location_str));
    head = NULL;
    number_registered_endpoints = 0;
    number_deleted_registrations = 0;
    
    random_init(0); 
    generate_query_list(query_list, base_strings, base_count);

    for (int i = 0; i < REGISTERED_ENDPOINTS_MAX_NUMBER; i++) {

        register_endpoint(addr_str, query_list[i], location_str, payload, &payload_len);

        memset(location_str, 0, sizeof(location_str));
    }


    for (int i = 0; i < runs; i++) {
        
        start = ztimer_now(ZTIMER_USEC);

        register_endpoint(addr_str, query_list[REGISTERED_ENDPOINTS_MAX_NUMBER - 1], location_str, payload, &payload_len);

        end   = ztimer_now(ZTIMER_USEC);

        total_time += (end - start);
    }

    printf("Average execution time idempotent case: %d microseconds\n", total_time / runs);

}

void benchmark_registration_deleted_middle_elements(void){

    memset(registered_endpoints_list, 0, sizeof(registered_endpoints_list));
    memset(deleted_registrations_list, 0, sizeof(deleted_registrations_list));
    memset(lookup_result, 0, sizeof(lookup_result));
    memset(relative_uris, 0, sizeof(relative_uris));
    memset(query_list, 0, sizeof(query_list));
    memset(location_str, 0, sizeof(location_str));
    head = NULL;
    number_registered_endpoints = 0;
    number_deleted_registrations = 0;
    
    random_init(0); // Initialize RIOT's random system
    generate_query_list(query_list, base_strings, base_count);


    for (int i = 0; i < REGISTERED_ENDPOINTS_MAX_NUMBER; i++) {

        register_endpoint(addr_str, query_list[i], location_str, payload, &payload_len);

        memset(location_str, 0, sizeof(location_str));
    }


    /* Delete the EP registrations in the middle */
    for (int i = 1; i < REGISTERED_ENDPOINTS_MAX_NUMBER; i++) {

        node_ptr = &registered_endpoints_list[i].node_management;

        next_expiring_node = find_next_expiring_endpoint();
        next_expiring_endpoint = container_of(next_expiring_node, Endpoint, node_management);

        int expired_lifetime = 0;

        if(next_expiring_node->location_nr == node_ptr->location_nr){

            expired_lifetime = next_expiring_endpoint->lt;
        }

        if (node_ptr->location_nr > 0 && node_ptr->location_nr < number_registered_endpoints)
        {
            disconnect_endpoint_from_the_rest(node_ptr->location_nr, node_ptr);
            delete_endpoint(node_ptr->location_nr);
            
            if(expired_lifetime > 0){

                update_registration_lifetimes(expired_lifetime);

                next_expiring_node = find_next_expiring_endpoint();
                next_expiring_endpoint = container_of(next_expiring_node, Endpoint, node_management);

                lifetime_expiry.callback = lifetime_callback; 
                lifetime_expiry.arg = &(next_expiring_node->location_nr);             
                ztimer_set(ZTIMER_SEC, &lifetime_expiry, next_expiring_endpoint->lt);

                if (next_expiring_endpoint->epsim == true){

                    epsim_get_request.callback = epsim_get_request_callback; 
                    epsim_get_request.arg = &next_expiring_endpoint->node_management.location_nr;      
                    ztimer_set(ZTIMER_SEC, &epsim_get_request, 0.75*next_expiring_endpoint->lt);
            
                }
            }
        }

    }

    int location_value = 0;

    for (int i = 0; i < runs; i++) {
        
        start = ztimer_now(ZTIMER_USEC);

        location_value = register_endpoint(addr_str, query_list[REGISTERED_ENDPOINTS_MAX_NUMBER - 1], location_str, payload, &payload_len);

        end   = ztimer_now(ZTIMER_USEC);

        total_time += (end - start);

        memset(location_str, 0, sizeof(location_str));

        node_ptr = &registered_endpoints_list[location_value - 1].node_management;

        next_expiring_node = find_next_expiring_endpoint();
        next_expiring_endpoint = container_of(next_expiring_node, Endpoint, node_management);

        int expired_lifetime = 0;

        if(next_expiring_node->location_nr == node_ptr->location_nr){

            expired_lifetime = next_expiring_endpoint->lt;
        }

        if (node_ptr->location_nr > 0 && node_ptr->location_nr < number_registered_endpoints)
        {
            disconnect_endpoint_from_the_rest(node_ptr->location_nr, node_ptr);
            delete_endpoint(node_ptr->location_nr);
            
            if(expired_lifetime > 0){

                update_registration_lifetimes(expired_lifetime);

                next_expiring_node = find_next_expiring_endpoint();
                next_expiring_endpoint = container_of(next_expiring_node, Endpoint, node_management);

                lifetime_expiry.callback = lifetime_callback; 
                lifetime_expiry.arg = &(next_expiring_node->location_nr);             
                ztimer_set(ZTIMER_SEC, &lifetime_expiry, next_expiring_endpoint->lt);

                if (next_expiring_endpoint->epsim == true){

                    epsim_get_request.callback = epsim_get_request_callback; 
                    epsim_get_request.arg = &next_expiring_endpoint->node_management.location_nr;      
                    ztimer_set(ZTIMER_SEC, &epsim_get_request, 0.75*next_expiring_endpoint->lt);
            
                }
            }
        }

    }

    printf("Average execution time registration in the middle: %d microseconds\n", total_time / runs);

}

void benchmark_sequential_epsim_registrations(void){

    memset(registered_endpoints_list, 0, sizeof(registered_endpoints_list));
    memset(deleted_registrations_list, 0, sizeof(deleted_registrations_list));
    memset(lookup_result, 0, sizeof(lookup_result));
    memset(relative_uris, 0, sizeof(relative_uris));
    memset(query_list, 0, sizeof(query_list));
    memset(location_str, 0, sizeof(location_str));
    head = NULL;
    number_registered_endpoints = 0;
    number_deleted_registrations = 0;

    random_init(0); // Initialize RIOT's random system
    generate_query_list(query_list, base_strings, base_count);

    generate_ipv6_list(IPv6_address_list);

    total_time = 0;

    for (int i = 0; i < runs; i++) {
        
        start = ztimer_now(ZTIMER_USEC);

        register_endpoint(IPv6_address_list[i], query_list[i], location_str, payload, &payload_len);

        /* Fetching the resources */

        ztimer_sleep(ZTIMER_MSEC, 250);

        endpoint_ptr = find_endpoint_by_pattern(IPv6_address_list[i]);


        if (next_expiring_node->location_nr == endpoint_ptr->node_management.location_nr){

            lifetime_expiry.callback = lifetime_callback;
            lifetime_expiry.arg      = &endpoint_ptr->node_management.location_nr;
            ztimer_set(ZTIMER_SEC, &lifetime_expiry, endpoint_ptr->lt);

        }

        memcpy((char*)endpoint_ptr->resources, "<resource-link-1>,<resource-link-2>", sizeof("<resource-link-1>,<resource-link-2>"));

        endpoint_ptr->epsim = true;

        end   = ztimer_now(ZTIMER_USEC);

        if (runs % (REGISTERED_ENDPOINTS_MAX_NUMBER - 1) == 0){
            memset(registered_endpoints_list, 0, sizeof(registered_endpoints_list));
            memset(deleted_registrations_list, 0, sizeof(deleted_registrations_list));
            head = NULL;
            number_registered_endpoints = 0;
            number_deleted_registrations = 0;
        }

        memset(location_str, 0, sizeof(location_str));

        total_time += (end - start);
    }

    printf("Average execution time sequential epsim: %d microseconds\n", total_time / runs);

}

void benchmark_idempotent_last_epsim_registration_existing(void){

    memset(registered_endpoints_list, 0, sizeof(registered_endpoints_list));
    memset(deleted_registrations_list, 0, sizeof(deleted_registrations_list));
    memset(lookup_result, 0, sizeof(lookup_result));
    memset(relative_uris, 0, sizeof(relative_uris));
    memset(query_list, 0, sizeof(query_list));
    memset(location_str, 0, sizeof(location_str));
    head = NULL;
    number_registered_endpoints = 0;
    number_deleted_registrations = 0;

    random_init(0); // Initialize RIOT's random system
    generate_query_list(query_list, base_strings, base_count);

    generate_ipv6_list(IPv6_address_list);

    uint8_t *payload_epsim = (uint8_t*)"";

    uint16_t payload_len_epsim = strlen((char *)payload);

    for (int i = 0; i < REGISTERED_ENDPOINTS_MAX_NUMBER; i++) {

        register_endpoint(IPv6_address_list[i], query_list[i], location_str, payload_epsim, &payload_len_epsim);

        memset(location_str, 0, sizeof(location_str));
    }


    total_time = 0;

    for (int i = 0; i < runs; i++) {
        
        start = ztimer_now(ZTIMER_USEC);

        register_endpoint(IPv6_address_list[REGISTERED_ENDPOINTS_MAX_NUMBER - 1], query_list[REGISTERED_ENDPOINTS_MAX_NUMBER - 1], location_str, payload, &payload_len);

        /* Fetching the resources */

        ztimer_sleep(ZTIMER_MSEC, 250);

        endpoint_ptr = find_endpoint_by_pattern(IPv6_address_list[REGISTERED_ENDPOINTS_MAX_NUMBER - 1]);

        next_expiring_node = find_next_expiring_endpoint();

        if (next_expiring_node->location_nr == endpoint_ptr->node_management.location_nr){

            lifetime_expiry.callback = lifetime_callback;
            lifetime_expiry.arg      = &endpoint_ptr->node_management.location_nr;
            ztimer_set(ZTIMER_SEC, &lifetime_expiry, endpoint_ptr->lt);

        }

        memcpy((char*)endpoint_ptr->resources, "<resource-link-1>,<resource-link-2>", sizeof("<resource-link-1>,<resource-link-2>"));

        endpoint_ptr->epsim = true;

        end   = ztimer_now(ZTIMER_USEC);

        total_time += (end - start);
    }

    printf("Average execution time last epsim idempotent: %d microseconds\n", total_time / runs);

}

void benchmark_epsim_registration_deleted_middle_elements(void){

    memset(registered_endpoints_list, 0, sizeof(registered_endpoints_list));
    memset(deleted_registrations_list, 0, sizeof(deleted_registrations_list));
    memset(lookup_result, 0, sizeof(lookup_result));
    memset(relative_uris, 0, sizeof(relative_uris));
    memset(query_list, 0, sizeof(query_list));
    memset(location_str, 0, sizeof(location_str));
    head = NULL;
    number_registered_endpoints = 0;
    number_deleted_registrations = 0;

    
    random_init(0); // Initialize RIOT's random system
    generate_query_list(query_list, base_strings, base_count);

    generate_ipv6_list(IPv6_address_list);

    for (int i = 0; i < REGISTERED_ENDPOINTS_MAX_NUMBER; i++) {

        register_endpoint(IPv6_address_list[i], query_list[i], location_str, payload, &payload_len);

        memset(location_str, 0, sizeof(location_str));
    }

    /* Delete the EP registrations in the middle */
    for (int i = 1; i < REGISTERED_ENDPOINTS_MAX_NUMBER; i++) {

        node_ptr = &registered_endpoints_list[i].node_management;

        next_expiring_node = find_next_expiring_endpoint();
        next_expiring_endpoint = container_of(next_expiring_node, Endpoint, node_management);

        int expired_lifetime = 0;

        if(next_expiring_node->location_nr == node_ptr->location_nr){

            expired_lifetime = next_expiring_endpoint->lt;
        }

        if (node_ptr->location_nr > 0 && node_ptr->location_nr < number_registered_endpoints)
        {
            disconnect_endpoint_from_the_rest(node_ptr->location_nr, node_ptr);
            delete_endpoint(node_ptr->location_nr);
            
            if(expired_lifetime > 0){

                update_registration_lifetimes(expired_lifetime);

                next_expiring_node = find_next_expiring_endpoint();
                next_expiring_endpoint = container_of(next_expiring_node, Endpoint, node_management);

                lifetime_expiry.callback = lifetime_callback; 
                lifetime_expiry.arg = &(next_expiring_node->location_nr);             
                ztimer_set(ZTIMER_SEC, &lifetime_expiry, next_expiring_endpoint->lt);

                if (next_expiring_endpoint->epsim == true){

                    epsim_get_request.callback = epsim_get_request_callback; 
                    epsim_get_request.arg = &next_expiring_endpoint->node_management.location_nr;      
                    ztimer_set(ZTIMER_SEC, &epsim_get_request, 0.75*next_expiring_endpoint->lt);
            
                }
            }
        }

    }

    total_time = 0;

    int location_value = 0;

    for (int i = 0; i < runs; i++) {
        
        start = ztimer_now(ZTIMER_USEC);

        location_value = register_endpoint(IPv6_address_list[REGISTERED_ENDPOINTS_MAX_NUMBER - 1], query_list[REGISTERED_ENDPOINTS_MAX_NUMBER - 1], location_str, payload, &payload_len);

        ztimer_sleep(ZTIMER_MSEC, 250);

        endpoint_ptr = find_endpoint_by_pattern(IPv6_address_list[REGISTERED_ENDPOINTS_MAX_NUMBER - 1]);

        next_expiring_node = find_next_expiring_endpoint();

        if (next_expiring_node->location_nr == endpoint_ptr->node_management.location_nr){

            lifetime_expiry.callback = lifetime_callback;
            lifetime_expiry.arg      = &endpoint_ptr->node_management.location_nr;
            ztimer_set(ZTIMER_SEC, &lifetime_expiry, endpoint_ptr->lt);

        }

        memcpy((char*)endpoint_ptr->resources, "<resource-link-1>,<resource-link-2>", sizeof("<resource-link-1>,<resource-link-2>"));

        endpoint_ptr->epsim = true;

        end   = ztimer_now(ZTIMER_USEC);

        total_time += (end - start);

        memset(location_str, 0, sizeof(location_str));

        node_ptr = &registered_endpoints_list[location_value - 1].node_management;

        next_expiring_node = find_next_expiring_endpoint();
        next_expiring_endpoint = container_of(next_expiring_node, Endpoint, node_management);

        int expired_lifetime = 0;

        if(next_expiring_node->location_nr == node_ptr->location_nr){

            expired_lifetime = next_expiring_endpoint->lt;
        }

        if (node_ptr->location_nr > 0 && node_ptr->location_nr < number_registered_endpoints)
        {
            disconnect_endpoint_from_the_rest(node_ptr->location_nr, node_ptr);
            delete_endpoint(node_ptr->location_nr);
            
            if(expired_lifetime > 0){

                update_registration_lifetimes(expired_lifetime);

                next_expiring_node = find_next_expiring_endpoint();
                next_expiring_endpoint = container_of(next_expiring_node, Endpoint, node_management);

                lifetime_expiry.callback = lifetime_callback; 
                lifetime_expiry.arg = &(next_expiring_node->location_nr);             
                ztimer_set(ZTIMER_SEC, &lifetime_expiry, next_expiring_endpoint->lt);

                if (next_expiring_endpoint->epsim == true){

                    epsim_get_request.callback = epsim_get_request_callback; 
                    epsim_get_request.arg = &next_expiring_endpoint->node_management.location_nr;      
                    ztimer_set(ZTIMER_SEC, &epsim_get_request, 0.75*next_expiring_endpoint->lt);
            
                }
            }
        }

    }

    printf("Average execution time epsim in the middle: %d microseconds\n", total_time / runs);

}

void benchmark_update_last_element(void){

    memset(registered_endpoints_list, 0, sizeof(registered_endpoints_list));
    memset(deleted_registrations_list, 0, sizeof(deleted_registrations_list));
    memset(lookup_result, 0, sizeof(lookup_result));
    memset(relative_uris, 0, sizeof(relative_uris));
    memset(query_list, 0, sizeof(query_list));
    memset(location_str, 0, sizeof(location_str));
    head = NULL;
    number_registered_endpoints = 0;
    number_deleted_registrations = 0;

    random_init(0); // Initialize RIOT's random system
    generate_query_list(query_list, base_strings, base_count);


    for (int i = 0; i < REGISTERED_ENDPOINTS_MAX_NUMBER; i++) {

        register_endpoint(addr_str, query_list[i], location_str, payload, &payload_len);

        memset(location_str, 0, sizeof(location_str));
    }

    node_ptr = &registered_endpoints_list[REGISTERED_ENDPOINTS_MAX_NUMBER - 1].node_management;
    endpoint_ptr = container_of(node_ptr, Endpoint, node_management);

    endpoint_ptr->lt = 30;

    lifetime_expiry.callback = lifetime_callback; 
    lifetime_expiry.arg = &(node_ptr->location_nr);             
    ztimer_set(ZTIMER_SEC, &lifetime_expiry, endpoint_ptr->lt);

    if (endpoint_ptr->epsim == true){

        epsim_get_request.callback = epsim_get_request_callback; 
        epsim_get_request.arg = &node_ptr->location_nr;      
        ztimer_set(ZTIMER_SEC, &epsim_get_request, 0.75*endpoint_ptr->lt);

    }


    int payload_len_update = strlen((char *)payload);

    for (int i = 0; i < runs; i++) {
        
        start = ztimer_now(ZTIMER_USEC);

        update_endpoint((char *)payload, &payload_len_update, query_list[QUERY_LIST_SIZE - 1], endpoint_ptr, addr_str);

        end   = ztimer_now(ZTIMER_USEC);

        total_time += (end - start);
    }

    printf("Average execution time update last element: %d microseconds\n", total_time / runs);

}

void benchmark_delete_last_element(void){

    memset(registered_endpoints_list, 0, sizeof(registered_endpoints_list));
    memset(deleted_registrations_list, 0, sizeof(deleted_registrations_list));
    memset(lookup_result, 0, sizeof(lookup_result));
    memset(relative_uris, 0, sizeof(relative_uris));
    memset(query_list, 0, sizeof(query_list));
    memset(location_str, 0, sizeof(location_str));
    head = NULL;
    number_registered_endpoints = 0;
    number_deleted_registrations = 0;

    random_init(0); // Initialize RIOT's random system
    generate_query_list(query_list, base_strings, base_count);


    for (int i = 0; i < REGISTERED_ENDPOINTS_MAX_NUMBER; i++) {

        register_endpoint(addr_str, query_list[i], location_str, payload, &payload_len);

        memset(location_str, 0, sizeof(location_str));
    }

    node_ptr = &registered_endpoints_list[REGISTERED_ENDPOINTS_MAX_NUMBER - 1].node_management;
    endpoint_ptr = container_of(node_ptr, Endpoint, node_management);

    endpoint_ptr->lt = 30;

    lifetime_expiry.callback = lifetime_callback; 
    lifetime_expiry.arg = &(node_ptr->location_nr);             
    ztimer_set(ZTIMER_SEC, &lifetime_expiry, endpoint_ptr->lt);

    if (endpoint_ptr->epsim == true){

        epsim_get_request.callback = epsim_get_request_callback; 
        epsim_get_request.arg = &node_ptr->location_nr;      
        ztimer_set(ZTIMER_SEC, &epsim_get_request, 0.75*endpoint_ptr->lt);

    }

    total_time = 0;

    for (int i = 0; i < runs; i++) {
        
        start = ztimer_now(ZTIMER_USEC);

        next_expiring_node = find_next_expiring_endpoint();
        next_expiring_endpoint = container_of(next_expiring_node, Endpoint, node_management);

        int expired_lifetime = 0;

        if(next_expiring_node->location_nr == node_ptr->location_nr){

            expired_lifetime = next_expiring_endpoint->lt;
        }

        if (node_ptr->location_nr > 0 && node_ptr->location_nr < number_registered_endpoints)
        {
            disconnect_endpoint_from_the_rest(node_ptr->location_nr, node_ptr);
            delete_endpoint(node_ptr->location_nr);
            
            if(expired_lifetime > 0){

                update_registration_lifetimes(expired_lifetime);

                next_expiring_node = find_next_expiring_endpoint();
                next_expiring_endpoint = container_of(next_expiring_node, Endpoint, node_management);

                lifetime_expiry.callback = lifetime_callback; 
                lifetime_expiry.arg = &(next_expiring_node->location_nr);             
                ztimer_set(ZTIMER_SEC, &lifetime_expiry, next_expiring_endpoint->lt);

                if (next_expiring_endpoint->epsim == true){

                    epsim_get_request.callback = epsim_get_request_callback; 
                    epsim_get_request.arg = &next_expiring_endpoint->node_management.location_nr;      
                    ztimer_set(ZTIMER_SEC, &epsim_get_request, 0.75*next_expiring_endpoint->lt);
            
                }
            }
        }

        end   = ztimer_now(ZTIMER_USEC);

        total_time += (end - start);

        register_endpoint(addr_str, query_list[REGISTERED_ENDPOINTS_MAX_NUMBER - 1], location_str, payload, &payload_len);
    }

    printf("Average execution time delete last element: %d microseconds\n", total_time / runs);

}

void benchmark_resource_lookup_href(void){

    memset(registered_endpoints_list, 0, sizeof(registered_endpoints_list));
    memset(deleted_registrations_list, 0, sizeof(deleted_registrations_list));
    memset(lookup_result, 0, sizeof(lookup_result));
    memset(relative_uris, 0, sizeof(relative_uris));
    memset(query_list, 0, sizeof(query_list));
    memset(location_str, 0, sizeof(location_str));
    head = NULL;
    number_registered_endpoints = 0;
    number_deleted_registrations = 0;

    random_init(0); // Initialize RIOT's random system
    generate_query_list(query_list, base_strings, base_count);

    generate_ipv6_list(IPv6_address_list);

    for (int i = 0; i < REGISTERED_ENDPOINTS_MAX_NUMBER; i++) {

        register_endpoint(IPv6_address_list[i], query_list[i], location_str, payload, &payload_len);

        memset(location_str, 0, sizeof(location_str));
    }


    uint8_t uri_query[] = " href=/reg/6 ";

    int resource_number;

    total_time = 0;

    for (int i = 0; i < runs; i++) {
    
        start = ztimer_now(ZTIMER_USEC);

        resource_number = 0;
        memset(lookup_result, 0, sizeof(lookup_result));
        memset(relative_uris, 0, sizeof(relative_uris));


        if (strstr((char*)uri_query, "href") != NULL)
        {

            char path[LOCATION_STR_MAX_LEN];
            extract_value_from_query((char*)uri_query, path, "href=");
            int location_nr = extract_number_from_location(path);

            if (location_nr <= number_registered_endpoints && location_nr > 0)
            {
                node_ptr = &registered_endpoints_list[location_nr - 1].node_management;
                endpoint_ptr = container_of(node_ptr, Endpoint, node_management);
                resource_number = extract_resource_uris(endpoint_ptr->resources, relative_uris);
                build_resource_string(resource_number, relative_uris, lookup_result, endpoint_ptr);
            }

        }

        end   = ztimer_now(ZTIMER_USEC);

        total_time += (end - start);

    }

    printf("Average execution time resource lookup href: %d microseconds\n", total_time / runs);
}

void benchmark_resource_lookup_ep_name(void){

    memset(registered_endpoints_list, 0, sizeof(registered_endpoints_list));
    memset(deleted_registrations_list, 0, sizeof(deleted_registrations_list));
    memset(lookup_result, 0, sizeof(lookup_result));
    memset(relative_uris, 0, sizeof(relative_uris));
    memset(query_list, 0, sizeof(query_list));
    memset(location_str, 0, sizeof(location_str));
    head = NULL;
    number_registered_endpoints = 0;
    number_deleted_registrations = 0;

    random_init(0); // Initialize RIOT's random system
    generate_query_list(query_list, base_strings, base_count);


    generate_ipv6_list(IPv6_address_list);


    for (int i = 0; i < REGISTERED_ENDPOINTS_MAX_NUMBER; i++) {

        register_endpoint(IPv6_address_list[i], query_list[i], location_str, payload, &payload_len);

        memset(location_str, 0, sizeof(location_str));
    }


    node_ptr = &registered_endpoints_list[REGISTERED_ENDPOINTS_MAX_NUMBER - 1].node_management;
    endpoint_ptr = container_of(node_ptr, Endpoint, node_management);

    uint8_t uri_query[CONFIG_NANOCOAP_URI_MAX] = " ep=";

    strcat((char *)uri_query, endpoint_ptr->name);
    strcat((char *)uri_query, " ");


    int resource_number;

    total_time = 0;

    for (int i = 0; i < runs; i++) {
    
        start = ztimer_now(ZTIMER_USEC);

        resource_number = 0;
        memset(lookup_result, 0, sizeof(lookup_result));
        memset(relative_uris, 0, sizeof(relative_uris));


        if(strstr((char*)uri_query, "ep") != NULL)
        {
            char ep_name[ENDPOINT_NAME_MAX_LEN];
            extract_value_from_query((char*)uri_query, ep_name, "ep=");
            Endpoint *ep = find_endpoint_by_pattern(ep_name);
            resource_number = extract_resource_uris(ep->resources, relative_uris);
            build_resource_string(resource_number, relative_uris, lookup_result, ep);
        }

        end   = ztimer_now(ZTIMER_USEC);

        total_time += (end - start);

    }

    printf("Average execution time resource lookup EP name: %d microseconds\n", total_time / runs);
}

void benchmark_resource_lookup_base_uri(void){

    memset(registered_endpoints_list, 0, sizeof(registered_endpoints_list));
    memset(deleted_registrations_list, 0, sizeof(deleted_registrations_list));
    memset(lookup_result, 0, sizeof(lookup_result));
    memset(relative_uris, 0, sizeof(relative_uris));
    memset(query_list, 0, sizeof(query_list));
    memset(location_str, 0, sizeof(location_str));
    head = NULL;
    number_registered_endpoints = 0;
    number_deleted_registrations = 0;

    random_init(0); // Initialize RIOT's random system
    generate_query_list(query_list, base_strings, base_count);

    generate_ipv6_list(IPv6_address_list);

 
    for (int i = 0; i < REGISTERED_ENDPOINTS_MAX_NUMBER; i++) {

        register_endpoint(IPv6_address_list[i], query_list[i], location_str, payload, &payload_len);

        memset(location_str, 0, sizeof(location_str));
    }


    uint8_t uri_query[CONFIG_NANOCOAP_URI_MAX] = " base=";

    strcat((char *)uri_query, IPv6_address_list[REGISTERED_ENDPOINTS_MAX_NUMBER - 1]);
    strcat((char *)uri_query, " ");


    int resource_number;

    total_time = 0;

    for (int i = 0; i < runs; i++) {
    
        start = ztimer_now(ZTIMER_USEC);

        resource_number = 0;
        memset(lookup_result, 0, sizeof(lookup_result));
        memset(relative_uris, 0, sizeof(relative_uris));


        if(strstr((char*)uri_query, "base") != NULL)
        {
            char base_value[BASE_URI_MAX_LEN];
            extract_value_from_query((char*)uri_query, base_value, "base=");
            find_endpoints_by_pattern(base_value, lookup_result, NULL, NULL, NULL, NULL,
                NULL, relative_uris, &resource_number);
            
        }

        end   = ztimer_now(ZTIMER_USEC);

        total_time += (end - start);

    }

    printf("Average execution time resource lookup base URI: %d microseconds\n", total_time / runs);
}

void benchmark_resource_lookup_et(void){

    memset(registered_endpoints_list, 0, sizeof(registered_endpoints_list));
    memset(deleted_registrations_list, 0, sizeof(deleted_registrations_list));
    memset(lookup_result, 0, sizeof(lookup_result));
    memset(relative_uris, 0, sizeof(relative_uris));
    memset(query_list, 0, sizeof(query_list));
    memset(location_str, 0, sizeof(location_str));
    head = NULL;
    number_registered_endpoints = 0;
    number_deleted_registrations = 0;

    random_init(0); // Initialize RIOT's random system
    generate_query_list(query_list, base_strings, base_count);

    generate_ipv6_list(IPv6_address_list);


    for (int i = 0; i < REGISTERED_ENDPOINTS_MAX_NUMBER; i++) {

        register_endpoint(IPv6_address_list[i], query_list[i], location_str, payload, &payload_len);

        memset(location_str, 0, sizeof(location_str));
    }


    uint8_t uri_query[CONFIG_NANOCOAP_URI_MAX] = " et=oic.r.glucose.medication ";

    int resource_number;

    total_time = 0;

    for (int i = 0; i < runs; i++) {
    
        start = ztimer_now(ZTIMER_USEC);

        resource_number = 0;
        memset(lookup_result, 0, sizeof(lookup_result));
        memset(relative_uris, 0, sizeof(relative_uris));


        if(strstr((char*)uri_query, "et") != NULL)
        {
            char endpoint_type[ENDPOINT_TYPE_MAX_LEN] = { 0 };
            extract_value_from_query((char*)uri_query, endpoint_type, "et=");
            find_endpoints_by_pattern(endpoint_type, lookup_result, NULL, NULL, NULL, NULL,
                NULL, relative_uris, &resource_number);

        }

        end   = ztimer_now(ZTIMER_USEC);

        total_time += (end - start);

    }

    printf("Average execution time resource lookup EP type: %d microseconds\n", total_time / runs);
}

void benchmark_resource_lookup_default(void){

    memset(registered_endpoints_list, 0, sizeof(registered_endpoints_list));
    memset(deleted_registrations_list, 0, sizeof(deleted_registrations_list));
    memset(lookup_result, 0, sizeof(lookup_result));
    memset(relative_uris, 0, sizeof(relative_uris));
    memset(query_list, 0, sizeof(query_list));
    memset(location_str, 0, sizeof(location_str));
    head = NULL;
    number_registered_endpoints = 0;
    number_deleted_registrations = 0;

    random_init(0); // Initialize RIOT's random system
    generate_query_list(query_list, base_strings, base_count);

    generate_ipv6_list(IPv6_address_list);


    for (int i = 0; i < REGISTERED_ENDPOINTS_MAX_NUMBER; i++) {

        register_endpoint(IPv6_address_list[i], query_list[i], location_str, payload, &payload_len);

        memset(location_str, 0, sizeof(location_str));
    }


    uint8_t uri_query[] = "";

    int resource_number;

    total_time = 0;

    for (int i = 0; i < runs; i++) {
    
        start = ztimer_now(ZTIMER_USEC);

        resource_number = 0;
        memset(lookup_result, 0, sizeof(lookup_result));
        memset(relative_uris, 0, sizeof(relative_uris));


        build_whole_result_string(uri_query, lookup_result, NULL, NULL, NULL, NULL,
                                    NULL, relative_uris, &resource_number);

        end   = ztimer_now(ZTIMER_USEC);

        total_time += (end - start);

    }

    printf("Average execution time no filter: %d microseconds\n", total_time / runs);
}

void benchmark_ep_lookup_href(void){

    memset(registered_endpoints_list, 0, sizeof(registered_endpoints_list));
    memset(deleted_registrations_list, 0, sizeof(deleted_registrations_list));
    memset(lookup_result, 0, sizeof(lookup_result));
    memset(relative_uris, 0, sizeof(relative_uris));
    memset(query_list, 0, sizeof(query_list));
    memset(location_str, 0, sizeof(location_str));
    head = NULL;
    number_registered_endpoints = 0;
    number_deleted_registrations = 0;

    random_init(0); // Initialize RIOT's random system
    generate_query_list(query_list, base_strings, base_count);

    generate_ipv6_list(IPv6_address_list);


    for (int i = 0; i < REGISTERED_ENDPOINTS_MAX_NUMBER; i++) {

        register_endpoint(IPv6_address_list[i], query_list[i], location_str, payload, &payload_len);

        memset(location_str, 0, sizeof(location_str));
    }


    uint8_t uri_query[] = " href=/reg/6 ";

    static char lookup_result[LOOKUP_RESULT_STR_MAX_LEN];

    char* first_bracket = "<";
    char* second_href_bracket = ">;";
    char* ep_key = "ep=\"";
    char* base = "\";base=\"";
    char* rt = "\";rt=\"";
    
    total_time = 0;

    for (int i = 0; i < runs; i++) {
    
        start = ztimer_now(ZTIMER_USEC);

        memset(lookup_result, 0, sizeof(lookup_result));


        if (strstr((char*)uri_query, "href") != NULL)
        {
            char path[LOCATION_STR_MAX_LEN];
            extract_value_from_query((char*)uri_query, path, "href=");
            int location_nr = extract_number_from_location(path);

            if (location_nr <= number_registered_endpoints && location_nr > 0)
            {
                node_ptr = &registered_endpoints_list[location_nr - 1].node_management;
                endpoint_ptr = container_of(node_ptr, Endpoint, node_management);
                build_result_string(lookup_result, first_bracket, second_href_bracket, ep_key, base, rt, endpoint_ptr, endpoint_ptr->et);
            }

        }

        end   = ztimer_now(ZTIMER_USEC);

        total_time += (end - start);

    }

    printf("Average execution time EP lookup href: %d microseconds\n", total_time / runs);
}

void benchmark_ep_lookup_ep_name(void){

    memset(registered_endpoints_list, 0, sizeof(registered_endpoints_list));
    memset(deleted_registrations_list, 0, sizeof(deleted_registrations_list));
    memset(lookup_result, 0, sizeof(lookup_result));
    memset(relative_uris, 0, sizeof(relative_uris));
    memset(query_list, 0, sizeof(query_list));
    memset(location_str, 0, sizeof(location_str));
    head = NULL;
    number_registered_endpoints = 0;
    number_deleted_registrations = 0;

    random_init(0); // Initialize RIOT's random system
    generate_query_list(query_list, base_strings, base_count);

    generate_ipv6_list(IPv6_address_list);


    for (int i = 0; i < REGISTERED_ENDPOINTS_MAX_NUMBER; i++) {

        register_endpoint(IPv6_address_list[i], query_list[i], location_str, payload, &payload_len);

        memset(location_str, 0, sizeof(location_str));
    }


    node_ptr = &registered_endpoints_list[REGISTERED_ENDPOINTS_MAX_NUMBER - 1].node_management;
    endpoint_ptr = container_of(node_ptr, Endpoint, node_management);

    uint8_t uri_query[CONFIG_NANOCOAP_URI_MAX] = " ep=";
    strcat((char *)uri_query, endpoint_ptr->name);
    strcat((char *)uri_query, " ");


    char* first_bracket = "<";
    char* second_href_bracket = ">;";
    char* ep_key = "ep=\"";
    char* base = "\";base=\"";
    char* rt = "\";rt=\"";
    
    total_time = 0;

    for (int i = 0; i < runs; i++) {
    
        start = ztimer_now(ZTIMER_USEC);

        memset(lookup_result, 0, sizeof(lookup_result));

        if(strstr((char*)uri_query, "ep") != NULL)
        {
            char ep_name[ENDPOINT_NAME_MAX_LEN];
            extract_value_from_query((char*)uri_query, ep_name, "ep=");
            Endpoint *ep = find_endpoint_by_pattern(ep_name);
            build_result_string(lookup_result, first_bracket, second_href_bracket, ep_key, base, rt, ep, ep->et);
        }

        end   = ztimer_now(ZTIMER_USEC);

        total_time += (end - start);

    }

    printf("Average execution time EP lookup name: %d microseconds\n", total_time / runs);
}

void benchmark_ep_lookup_base_uri(void){

    memset(registered_endpoints_list, 0, sizeof(registered_endpoints_list));
    memset(deleted_registrations_list, 0, sizeof(deleted_registrations_list));
    memset(lookup_result, 0, sizeof(lookup_result));
    memset(relative_uris, 0, sizeof(relative_uris));
    memset(query_list, 0, sizeof(query_list));
    memset(location_str, 0, sizeof(location_str));
    head = NULL;
    number_registered_endpoints = 0;
    number_deleted_registrations = 0;

    random_init(0); // Initialize RIOT's random system
    generate_query_list(query_list, base_strings, base_count);

    generate_ipv6_list(IPv6_address_list);


    for (int i = 0; i < REGISTERED_ENDPOINTS_MAX_NUMBER; i++) {

        register_endpoint(IPv6_address_list[i], query_list[i], location_str, payload, &payload_len);

        memset(location_str, 0, sizeof(location_str));
    }


    uint8_t uri_query[CONFIG_NANOCOAP_URI_MAX] = " base= ";

    strcat((char *)uri_query, IPv6_address_list[REGISTERED_ENDPOINTS_MAX_NUMBER - 1]);
    strcat((char *)uri_query, " ");


    char* first_bracket = "<";
    char* second_href_bracket = ">;";
    char* ep_key = "ep=\"";
    char* base = "\";base=\"";
    char* rt = "\";rt=\"";
    
    total_time = 0;

    for (int i = 0; i < runs; i++) {
    
        start = ztimer_now(ZTIMER_USEC);

        memset(lookup_result, 0, sizeof(lookup_result));


        if(strstr((char*)uri_query, "base") != NULL)
        {
            char base_value[BASE_URI_MAX_LEN];
            extract_value_from_query((char*)uri_query, base_value, "base=");
            find_endpoints_by_pattern(base_value, lookup_result, first_bracket, second_href_bracket, ep_key, base,
                                        rt, NULL, NULL);

        }

        end   = ztimer_now(ZTIMER_USEC);

        total_time += (end - start);

    }

    printf("Average execution time: %d microseconds\n", total_time / runs);
}

void benchmark_ep_lookup_et(void){

    memset(registered_endpoints_list, 0, sizeof(registered_endpoints_list));
    memset(deleted_registrations_list, 0, sizeof(deleted_registrations_list));
    memset(lookup_result, 0, sizeof(lookup_result));
    memset(relative_uris, 0, sizeof(relative_uris));
    memset(query_list, 0, sizeof(query_list));
    memset(location_str, 0, sizeof(location_str));
    head = NULL;
    number_registered_endpoints = 0;
    number_deleted_registrations = 0;

    random_init(0); // Initialize RIOT's random system
    generate_query_list(query_list, base_strings, base_count);

    generate_ipv6_list(IPv6_address_list);


    for (int i = 0; i < REGISTERED_ENDPOINTS_MAX_NUMBER; i++) {

        register_endpoint(IPv6_address_list[i], query_list[i], location_str, payload, &payload_len);

        memset(location_str, 0, sizeof(location_str));
    }


    uint8_t uri_query[] = " et=oic.r.glucose.medication ";


    char* first_bracket = "<";
    char* second_href_bracket = ">;";
    char* ep_key = "ep=\"";
    char* base = "\";base=\"";
    char* rt = "\";rt=\"";

    total_time = 0;

    for (int i = 0; i < runs; i++) {
    
        start = ztimer_now(ZTIMER_USEC);

        memset(lookup_result, 0, sizeof(lookup_result));


        if(strstr((char*)uri_query, "et") != NULL)
        {
            char endpoint_type[ENDPOINT_TYPE_MAX_LEN] = { 0 };
            extract_value_from_query((char*)uri_query, endpoint_type, "et=");
            find_endpoints_by_pattern(endpoint_type, lookup_result, first_bracket, second_href_bracket, ep_key, base,
                                        rt, NULL, NULL);
   
    
        }

        end   = ztimer_now(ZTIMER_USEC);

        total_time += (end - start);

    }

    printf("Average execution time EP lookup type: %d microseconds\n", total_time / runs);
}

void benchmark_ep_lookup_default(void){

    memset(registered_endpoints_list, 0, sizeof(registered_endpoints_list));
    memset(deleted_registrations_list, 0, sizeof(deleted_registrations_list));
    memset(lookup_result, 0, sizeof(lookup_result));
    memset(relative_uris, 0, sizeof(relative_uris));
    memset(query_list, 0, sizeof(query_list));
    memset(location_str, 0, sizeof(location_str));
    head = NULL;
    number_registered_endpoints = 0;
    number_deleted_registrations = 0;

    random_init(0); // Initialize RIOT's random system
    generate_query_list(query_list, base_strings, base_count);

    generate_ipv6_list(IPv6_address_list);


    for (int i = 0; i < REGISTERED_ENDPOINTS_MAX_NUMBER; i++) {

        register_endpoint(IPv6_address_list[i], query_list[i], location_str, payload, &payload_len);

        memset(location_str, 0, sizeof(location_str));
    }


    uint8_t uri_query[] = "";


    char* first_bracket = "<";
    char* second_href_bracket = ">;";
    char* ep_key = "ep=\"";
    char* base = "\";base=\"";
    char* rt = "\";rt=\"";

    total_time = 0;

    for (int i = 0; i < runs; i++) {
    
        start = ztimer_now(ZTIMER_USEC);

        memset(lookup_result, 0, sizeof(lookup_result));

        build_whole_result_string(uri_query, lookup_result, first_bracket, second_href_bracket, ep_key, base,
                                    rt, NULL, NULL);

        end   = ztimer_now(ZTIMER_USEC);

        total_time += (end - start);

    }

    printf("Average execution time lookup default case: %d microseconds\n", total_time / runs);
}

int main(void) {

    ztimer_sleep(ZTIMER_SEC, 1);

    benchmark_sequential_registrations();

    benchmark_idempotent_last_registration_existing();

    benchmark_registration_deleted_middle_elements();

    benchmark_sequential_epsim_registrations();

    benchmark_idempotent_last_epsim_registration_existing();

    benchmark_epsim_registration_deleted_middle_elements();

    benchmark_update_last_element();

    benchmark_delete_last_element();

    benchmark_resource_lookup_href();

    benchmark_resource_lookup_ep_name();

    benchmark_resource_lookup_base_uri();

    benchmark_resource_lookup_et();

    benchmark_resource_lookup_default();

    benchmark_ep_lookup_href();

    benchmark_ep_lookup_ep_name();

    benchmark_ep_lookup_base_uri();

    benchmark_ep_lookup_et();

    benchmark_ep_lookup_default();

    return 0;
}