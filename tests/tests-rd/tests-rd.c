/*
 * Copyright (C) <year> <author>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/* clib includes */

#include "embUnit.h"

#include "rd.h"

#include "tests-rd.h"


bool check_strings_equality(char* str_1, char* str_2){

    if (strcmp(str_1, str_2) == 0){

        return true;

    }else{

        return false;
    }
    
    return false;

}

static void test_register_endpoint_three_nodes(void) {

    memset(registered_endpoints_list, 0, sizeof(registered_endpoints_list));
    memset(deleted_registrations_list, 0, sizeof(deleted_registrations_list));
    memset(lookup_result_list, 0, sizeof(lookup_result_list));
    number_registered_endpoints = INITIAL_NUMBER_REGISTERED_ENDPOINTS;
    number_deleted_registrations = INITIAL_NUMBER_DELETED_ENDPOINTS;
    head = NULL;

    /* Initial data structures for the first endpoint */
    char location_str_first_endpoint[LOCATION_STR_MAX_LEN] = { 0 };

    char addr_str_first_endpoint[IPV6_ADDR_MAX_STR_LEN] = "fe80::cafe:cafe:cafe:5";

    unsigned char query_buffer_first_endpoint[QUERY_BUFFER_MAX_LEN] = " lt=570 ep=RIOT-1024239EKAJD98 ";

    uint8_t *payload_first_endpoint = (uint8_t *)"<resource-link-1>,<resource-link-2>";

    uint16_t payload_first_len = strlen((char *)payload_first_endpoint);

    /* Initial data structures for the second endpoint */
    char location_str_second_endpoint[LOCATION_STR_MAX_LEN] = { 0 };

    char addr_str_second_endpoint[IPV6_ADDR_MAX_STR_LEN] = "fe80::cafe:cafe:cafe:6";

    unsigned char query_buffer_second_endpoint[QUERY_BUFFER_MAX_LEN] = " lt=800 ep=RIOT-098503495KJHK et=oic.r.glucose.medication ";

    uint8_t *payload_second_endpoint = (uint8_t *)"<resource-link-3>,<resource-link-4>";

    uint16_t payload_second_len = strlen((char *)payload_second_endpoint);

    /* Initial data structures for the third endpoint */
    char location_str_third_endpoint[LOCATION_STR_MAX_LEN] = { 0 };

    char addr_str_third_endpoint[IPV6_ADDR_MAX_STR_LEN] = "fe80::cafe:cafe:cafe:7";

    unsigned char query_buffer_third_endpoint[QUERY_BUFFER_MAX_LEN] = " lt=1200 ep=RIOT-89234738234238 d=special ";

    uint8_t *payload_third_endpoint = (uint8_t *)"<resource-link-5>,<resource-link-6>";

    uint16_t payload_third_len = strlen((char *)payload_third_endpoint);

    

    /* Register the first endpoint */
    int location_first_endpoint = register_endpoint(addr_str_first_endpoint, query_buffer_first_endpoint, location_str_first_endpoint, 
                                                    payload_first_endpoint, &payload_first_len);

    /* Register the second endpoint */
    int location_second_endpoint = register_endpoint(addr_str_second_endpoint, query_buffer_second_endpoint, location_str_second_endpoint,
                                                     payload_second_endpoint, &payload_second_len);

    /* Register the third endpoint */
    int location_third_endpoint = register_endpoint(addr_str_third_endpoint, query_buffer_third_endpoint, location_str_third_endpoint,
                                                    payload_third_endpoint, &payload_third_len);

    i_list_node node_1, node_2, node_3 = { 0 };

    /* The first expected registered endpoint */
    node_1.location_nr = 1;
    node_1.previous = NULL;
    node_1.next = &registered_endpoints_list[1].node_management;

    Endpoint ep_1 = { .base = "coap://[fe80::cafe:cafe:cafe:5]", .lt = 570, .et = "core.rd-ep", .name = "RIOT-1024239EKAJD98", .sector = "default",
                     .resources = "<resource-link-1>,<resource-link-2>", .node_management = node_1};

    /* The second expected registered endpoint */
    node_2.location_nr = 2;
    node_2.previous = &registered_endpoints_list[0].node_management;
    node_2.next = &registered_endpoints_list[2].node_management;

    Endpoint ep_2 = { .base = "coap://[fe80::cafe:cafe:cafe:6]", .lt = 800, .et = "oic.r.glucose.medication", .name = "RIOT-098503495KJHK", .sector = "default",
                     .resources = "<resource-link-3>,<resource-link-4>", .node_management = node_2};

    /* The third expected registered endpoint */
    node_3.location_nr = 3;
    node_3.previous = &registered_endpoints_list[1].node_management;
    node_3.next = NULL;

    Endpoint ep_3 = { .base = "coap://[fe80::cafe:cafe:cafe:7]", .lt = 1200, .et = "core.rd-ep", .name = "RIOT-89234738234238", .sector = "special",
                     .resources = "<resource-link-5>,<resource-link-6>", .node_management = node_3};

    /* Pointers to the registered endpoints */
    i_list_node *node_ptr_1 = &registered_endpoints_list[location_first_endpoint - 1].node_management;
    Endpoint *endpoint_ptr_1 = container_of(node_ptr_1, Endpoint, node_management);

    i_list_node *node_ptr_2 = &registered_endpoints_list[location_second_endpoint - 1].node_management;
    Endpoint *endpoint_ptr_2 = container_of(node_ptr_2, Endpoint, node_management);

    i_list_node *node_ptr_3 = &registered_endpoints_list[location_third_endpoint - 1].node_management;
    Endpoint *endpoint_ptr_3 = container_of(node_ptr_3, Endpoint, node_management);

    /* Equality conditions between the registered endpoints and the expected ones */
    bool condition_ep_1 = check_strings_equality(endpoint_ptr_1->base, ep_1.base) && check_strings_equality(endpoint_ptr_1->et, ep_1.et) && (endpoint_ptr_1->lt == ep_1.lt) &&
                        check_strings_equality(endpoint_ptr_1->name, ep_1.name) && check_strings_equality(endpoint_ptr_1->resources, ep_1.resources) && 
                        check_strings_equality(endpoint_ptr_1->sector, ep_1.sector) && (node_ptr_1 ->location_nr == node_1.location_nr)
                         && (node_ptr_1->previous == node_1.previous) && (node_ptr_1->next == node_1.next);

    bool condition_ep_2 = check_strings_equality(endpoint_ptr_2->base, ep_2.base) && check_strings_equality(endpoint_ptr_2->et, ep_2.et) && (endpoint_ptr_2->lt == ep_2.lt) &&
                            check_strings_equality(endpoint_ptr_2->name, ep_2.name) && check_strings_equality(endpoint_ptr_2->resources, ep_2.resources) 
                            && check_strings_equality(endpoint_ptr_2->sector, ep_2.sector) && (node_ptr_2 ->location_nr == node_2.location_nr)
                             && (node_ptr_2->previous == node_2.previous) && (node_ptr_2->next == node_2.next);

    bool condition_ep_3 = check_strings_equality(endpoint_ptr_3->base, ep_3.base) && check_strings_equality(endpoint_ptr_3->et, ep_3.et) && (endpoint_ptr_3->lt == ep_3.lt) &&
                            check_strings_equality(endpoint_ptr_3->name, ep_3.name) && check_strings_equality(endpoint_ptr_3->resources, ep_3.resources) 
                            && check_strings_equality(endpoint_ptr_3->sector, ep_3.sector) && (node_ptr_3 ->location_nr == node_3.location_nr)
                             && (node_ptr_3->previous == node_3.previous) && (node_ptr_3->next == node_3.next);
                        
    TEST_ASSERT(condition_ep_1 && condition_ep_2 && condition_ep_3);
}

static void test_lifetime_expiration(void) {

    memset(registered_endpoints_list, 0, sizeof(registered_endpoints_list));
    memset(deleted_registrations_list, 0, sizeof(deleted_registrations_list));
    memset(lookup_result_list, 0, sizeof(lookup_result_list));
    number_registered_endpoints = INITIAL_NUMBER_REGISTERED_ENDPOINTS;
    number_deleted_registrations = INITIAL_NUMBER_DELETED_ENDPOINTS;
    head = NULL;

    /* Initial data structures for the first endpoint */
    char location_str_first_endpoint[LOCATION_STR_MAX_LEN] = { 0 };

    char addr_str_first_endpoint[IPV6_ADDR_MAX_STR_LEN] = "fe80::cafe:cafe:cafe:5";

    unsigned char query_buffer_first_endpoint[QUERY_BUFFER_MAX_LEN] = " lt=11 ep=RIOT-1024239EKAJD98 ";

    uint8_t *payload_first_endpoint = (uint8_t *)"<resource-link-1>,<resource-link-2>";

    uint16_t payload_first_len = strlen((char *)payload_first_endpoint);

    /* Initial data structures for the second endpoint */
    char location_str_second_endpoint[LOCATION_STR_MAX_LEN] = { 0 };

    char addr_str_second_endpoint[IPV6_ADDR_MAX_STR_LEN] = "fe80::cafe:cafe:cafe:6";

    unsigned char query_buffer_second_endpoint[QUERY_BUFFER_MAX_LEN] = " lt=5 ep=RIOT-098503495KJHK et=oic.r.glucose.medication ";

    uint8_t *payload_second_endpoint = (uint8_t *)"<resource-link-3>,<resource-link-4>";

    uint16_t payload_second_len = strlen((char *)payload_second_endpoint);

    /* Initial data structures for the third endpoint */
    char location_str_third_endpoint[LOCATION_STR_MAX_LEN] = { 0 };

    char addr_str_third_endpoint[IPV6_ADDR_MAX_STR_LEN] = "fe80::cafe:cafe:cafe:7";

    unsigned char query_buffer_third_endpoint[QUERY_BUFFER_MAX_LEN] = " lt=8 ep=RIOT-89234738234238 d=special ";

    uint8_t *payload_third_endpoint = (uint8_t *)"<resource-link-5>,<resource-link-6>";

    uint16_t payload_third_len = strlen((char *)payload_third_endpoint);

    

    /* Register the first endpoint */
    int location_first_endpoint = register_endpoint(addr_str_first_endpoint, query_buffer_first_endpoint, location_str_first_endpoint, 
                                                    payload_first_endpoint, &payload_first_len);

    /* Register the second endpoint */
    int location_second_endpoint = register_endpoint(addr_str_second_endpoint, query_buffer_second_endpoint, location_str_second_endpoint,
                                                     payload_second_endpoint, &payload_second_len);

    /* Register the third endpoint */
    int location_third_endpoint = register_endpoint(addr_str_third_endpoint, query_buffer_third_endpoint, location_str_third_endpoint,
                                                    payload_third_endpoint, &payload_third_len);

    /* Wait till the lifetime of the second endpoint expires */
    ztimer_sleep(ZTIMER_SEC, 5); 

    bool deleted_second_endpoint = (strlen((char *)registered_endpoints_list[location_second_endpoint - 1].name) == 0) && 
                                    (registered_endpoints_list[location_second_endpoint - 1].node_management.location_nr == 0) &&
                                    (registered_endpoints_list[location_second_endpoint - 1].node_management.next == NULL) &&
                                    (registered_endpoints_list[location_second_endpoint - 1].node_management.previous == NULL);

    /* Wait till the lifetime of the third endpoint  expires */
    ztimer_sleep(ZTIMER_SEC, 8);

    bool deleted_third_endpoint = (strlen((char *)registered_endpoints_list[location_third_endpoint - 1].name) == 0) && 
                                    (registered_endpoints_list[location_third_endpoint - 1].node_management.location_nr == 0) &&
                                    (registered_endpoints_list[location_third_endpoint - 1].node_management.next == NULL) &&
                                    (registered_endpoints_list[location_third_endpoint - 1].node_management.previous == NULL);

     /* Wait till the lifetime of the third endpoint expires */
    ztimer_sleep(ZTIMER_SEC, 11);

    bool deleted_first_endpoint = (strlen((char *)registered_endpoints_list[location_first_endpoint - 1].name) == 0) && 
                                    (registered_endpoints_list[location_first_endpoint - 1].node_management.location_nr == 0) &&
                                    (registered_endpoints_list[location_first_endpoint - 1].node_management.next == NULL) &&
                                    (registered_endpoints_list[location_first_endpoint - 1].node_management.previous == NULL);

    TEST_ASSERT(deleted_second_endpoint && deleted_third_endpoint && deleted_first_endpoint);
}

static void test_registration_idempotent(void) {

    memset(registered_endpoints_list, 0, sizeof(registered_endpoints_list));
    memset(deleted_registrations_list, 0, sizeof(deleted_registrations_list));
    memset(lookup_result_list, 0, sizeof(lookup_result_list));
    number_registered_endpoints = INITIAL_NUMBER_REGISTERED_ENDPOINTS;
    number_deleted_registrations = INITIAL_NUMBER_DELETED_ENDPOINTS;
    head = NULL;

    /* Initial data structures for the first endpoint */
    char location_str_first_endpoint[LOCATION_STR_MAX_LEN] = { 0 };

    char addr_str_first_endpoint[IPV6_ADDR_MAX_STR_LEN] = "fe80::cafe:cafe:cafe:5";

    unsigned char query_buffer_first_endpoint[QUERY_BUFFER_MAX_LEN] = " lt=570 ep=RIOT-1024239EKAJD98 ";

    uint8_t *payload_first_endpoint = (uint8_t *)"<resource-link-1>,<resource-link-2>";

    uint16_t payload_first_len = strlen((char *)payload_first_endpoint);

    /* Initial data structures for the second endpoint */
    char location_str_second_endpoint[LOCATION_STR_MAX_LEN] = { 0 };

    char addr_str_second_endpoint[IPV6_ADDR_MAX_STR_LEN] = "fe80::cafe:cafe:cafe:6";

    unsigned char query_buffer_second_endpoint[QUERY_BUFFER_MAX_LEN] = " lt=800 ep=RIOT-098503495KJHK et=oic.r.glucose.medication ";

    uint8_t *payload_second_endpoint = (uint8_t *)"<resource-link-3>,<resource-link-4>";

    uint16_t payload_second_len = strlen((char *)payload_second_endpoint);

    /* Initial data structures for the third endpoint */
    char location_str_third_endpoint[LOCATION_STR_MAX_LEN] = { 0 };

    char addr_str_third_endpoint[IPV6_ADDR_MAX_STR_LEN] = "fe80::cafe:cafe:cafe:7";

    unsigned char query_buffer_third_endpoint[QUERY_BUFFER_MAX_LEN] = " lt=1200 ep=RIOT-89234738234238 d=special ";

    uint8_t *payload_third_endpoint = (uint8_t *)"<resource-link-5>,<resource-link-6>";

    uint16_t payload_third_len = strlen((char *)payload_third_endpoint);


    /* Register the first endpoint */
                                    register_endpoint(addr_str_first_endpoint, query_buffer_first_endpoint, location_str_first_endpoint, 
                                                    payload_first_endpoint, &payload_first_len);

    /* Register the second endpoint */
    int location_second_endpoint = register_endpoint(addr_str_second_endpoint, query_buffer_second_endpoint, location_str_second_endpoint,
                                                        payload_second_endpoint, &payload_second_len);

    /* Register the third endpoint */
                                    register_endpoint(addr_str_third_endpoint, query_buffer_third_endpoint, location_str_third_endpoint,
                                                    payload_third_endpoint, &payload_third_len);


    /* Register the second endpoint again */
    int location_second_again = register_endpoint(addr_str_second_endpoint, query_buffer_second_endpoint, location_str_second_endpoint,
                                                    payload_second_endpoint, &payload_second_len);

                        
    TEST_ASSERT(location_second_endpoint == location_second_again); //The registration updates/overrides the existing registration in the same location.
}

static void test_delete_endpoint(void) {

    memset(registered_endpoints_list, 0, sizeof(registered_endpoints_list));
    memset(deleted_registrations_list, 0, sizeof(deleted_registrations_list));
    memset(lookup_result_list, 0, sizeof(lookup_result_list));
    number_registered_endpoints = INITIAL_NUMBER_REGISTERED_ENDPOINTS;
    number_deleted_registrations = INITIAL_NUMBER_DELETED_ENDPOINTS;
    head = NULL;

    /* Initial data structures for the first endpoint */
    char location_str_first_endpoint[LOCATION_STR_MAX_LEN] = { 0 };

    char addr_str_first_endpoint[IPV6_ADDR_MAX_STR_LEN] = "fe80::cafe:cafe:cafe:5";

    unsigned char query_buffer_first_endpoint[QUERY_BUFFER_MAX_LEN] = " lt=570 ep=RIOT-1024239EKAJD98 ";

    uint8_t *payload_first_endpoint = (uint8_t *)"<resource-link-1>,<resource-link-2>";

    uint16_t payload_first_len = strlen((char *)payload_first_endpoint);

    /* Initial data structures for the second endpoint */
    char location_str_second_endpoint[LOCATION_STR_MAX_LEN] = { 0 };

    char addr_str_second_endpoint[IPV6_ADDR_MAX_STR_LEN] = "fe80::cafe:cafe:cafe:6";

    unsigned char query_buffer_second_endpoint[QUERY_BUFFER_MAX_LEN] = " lt=800 ep=RIOT-098503495KJHK et=oic.r.glucose.medication ";

    uint8_t *payload_second_endpoint = (uint8_t *)"<resource-link-3>,<resource-link-4>";

    uint16_t payload_second_len = strlen((char *)payload_second_endpoint);

    /* Initial data structures for the third endpoint */
    char location_str_third_endpoint[LOCATION_STR_MAX_LEN] = { 0 };

    char addr_str_third_endpoint[IPV6_ADDR_MAX_STR_LEN] = "fe80::cafe:cafe:cafe:7";

    unsigned char query_buffer_third_endpoint[QUERY_BUFFER_MAX_LEN] = " lt=1200 ep=RIOT-89234738234238 d=special ";

    uint8_t *payload_third_endpoint = (uint8_t *)"<resource-link-5>,<resource-link-6>";

    uint16_t payload_third_len = strlen((char *)payload_third_endpoint);

    

    /* Register the first endpoint */
    int location_first_endpoint = register_endpoint(addr_str_first_endpoint, query_buffer_first_endpoint, location_str_first_endpoint, 
                                                    payload_first_endpoint, &payload_first_len);

    /* Register the second endpoint */
    int location_second_endpoint = register_endpoint(addr_str_second_endpoint, query_buffer_second_endpoint, location_str_second_endpoint,
                                                     payload_second_endpoint, &payload_second_len);

    /* Register the third endpoint */
    int location_third_endpoint = register_endpoint(addr_str_third_endpoint, query_buffer_third_endpoint, location_str_third_endpoint,
                                                    payload_third_endpoint, &payload_third_len);

    i_list_node node_1, node_2, node_3 = { 0 };

    /* The first expected registered endpoint */
    node_1.location_nr = 1;
    node_1.previous = NULL;
    node_1.next = &registered_endpoints_list[2].node_management; //Since the second node is deleted

    Endpoint ep_1 = { .base = "coap://[fe80::cafe:cafe:cafe:5]", .lt = 570, .et = "core.rd-ep", .name = "RIOT-1024239EKAJD98", .sector = "default",
                     .resources = "<resource-link-1>,<resource-link-2>", .node_management = node_1};

    /* The second expected deleted node */
    node_2.location_nr = 0;
    node_2.previous = NULL;
    node_2.next = NULL;

    /* The third expected registered endpoint */
    node_3.location_nr = 3;
    node_3.previous = &registered_endpoints_list[0].node_management; //Since the second node is deleted
    node_3.next = NULL;

    Endpoint ep_3 = { .base = "coap://[fe80::cafe:cafe:cafe:7]", .lt = 1200, .et = "core.rd-ep", .name = "RIOT-89234738234238", .sector = "special",
                     .resources = "<resource-link-5>,<resource-link-6>", .node_management = node_3};
    

    /* Pointers to the registered endpoints */
    i_list_node *node_ptr_1 = &registered_endpoints_list[location_first_endpoint - 1].node_management;
    Endpoint *endpoint_ptr_1 = container_of(node_ptr_1, Endpoint, node_management);

    i_list_node *node_ptr_2 = &registered_endpoints_list[location_second_endpoint - 1].node_management;

    i_list_node *node_ptr_3 = &registered_endpoints_list[location_third_endpoint - 1].node_management;
    Endpoint *endpoint_ptr_3 = container_of(node_ptr_3, Endpoint, node_management);


    disconnect_endpoint_from_the_rest(node_ptr_2->location_nr, node_ptr_2);
    delete_endpoint(node_ptr_2->location_nr);


    /* Equality conditions between the registered endpoints and the expected ones */
    bool condition_ep_1 = check_strings_equality(endpoint_ptr_1->base, ep_1.base) && check_strings_equality(endpoint_ptr_1->et, ep_1.et) && (endpoint_ptr_1->lt == ep_1.lt) &&
                        check_strings_equality(endpoint_ptr_1->name, ep_1.name) && check_strings_equality(endpoint_ptr_1->resources, ep_1.resources) && 
                        check_strings_equality(endpoint_ptr_1->sector, ep_1.sector) && (node_ptr_1 ->location_nr == node_1.location_nr)
                         && (node_ptr_1->previous == node_1.previous) && (node_ptr_1->next == node_1.next);

    bool condition_ep_2 = (node_ptr_2 ->location_nr == node_2.location_nr) && (node_ptr_2->previous == node_2.previous) && 
                            (node_ptr_2->next == node_2.next);

    bool condition_ep_3 = check_strings_equality(endpoint_ptr_3->base, ep_3.base) && check_strings_equality(endpoint_ptr_3->et, ep_3.et) && (endpoint_ptr_3->lt == ep_3.lt) &&
                            check_strings_equality(endpoint_ptr_3->name, ep_3.name) && check_strings_equality(endpoint_ptr_3->resources, ep_3.resources) 
                            && check_strings_equality(endpoint_ptr_3->sector, ep_3.sector) && (node_ptr_3 ->location_nr == node_3.location_nr)
                             && (node_ptr_3->previous == node_3.previous) && (node_ptr_3->next == node_3.next);

    TEST_ASSERT(condition_ep_1 && condition_ep_2 && condition_ep_3);
}

static void test_update_node(void) {

    memset(registered_endpoints_list, 0, sizeof(registered_endpoints_list));
    memset(deleted_registrations_list, 0, sizeof(deleted_registrations_list));
    memset(lookup_result_list, 0, sizeof(lookup_result_list));
    number_registered_endpoints = INITIAL_NUMBER_REGISTERED_ENDPOINTS;
    number_deleted_registrations = INITIAL_NUMBER_DELETED_ENDPOINTS;
    head = NULL;

    /* Initial data structures for the first endpoint */
    char location_str_first_endpoint[LOCATION_STR_MAX_LEN] = { 0 };

    char addr_str_first_endpoint[IPV6_ADDR_MAX_STR_LEN] = "fe80::cafe:cafe:cafe:5";

    unsigned char query_buffer_first_endpoint[QUERY_BUFFER_MAX_LEN] = " lt=570 ep=RIOT-1024239EKAJD98 ";

    uint8_t *payload_first_endpoint = (uint8_t *)"<resource-link-1>,<resource-link-2>";

    uint16_t payload_first_len = strlen((char *)payload_first_endpoint);

    /* Initial data structures for the second endpoint */
    char location_str_second_endpoint[LOCATION_STR_MAX_LEN] = { 0 };

    char addr_str_second_endpoint[IPV6_ADDR_MAX_STR_LEN] = "fe80::cafe:cafe:cafe:6";

    unsigned char query_buffer_second_endpoint[QUERY_BUFFER_MAX_LEN] = " lt=800 ep=RIOT-098503495KJHK et=oic.r.glucose.medication ";

    uint8_t *payload_second_endpoint = (uint8_t *)"<resource-link-3>,<resource-link-4>";

    uint16_t payload_second_len = strlen((char *)payload_second_endpoint);

    /* Initial data structures for the third endpoint */
    char location_str_third_endpoint[LOCATION_STR_MAX_LEN] = { 0 };

    char addr_str_third_endpoint[IPV6_ADDR_MAX_STR_LEN] = "fe80::cafe:cafe:cafe:7";

    unsigned char query_buffer_third_endpoint[QUERY_BUFFER_MAX_LEN] = " lt=1200 ep=RIOT-89234738234238 d=special ";

    uint8_t *payload_third_endpoint = (uint8_t *)"<resource-link-5>,<resource-link-6>";

    uint16_t payload_third_len = strlen((char *)payload_third_endpoint);

    

    /* Register the first endpoint */
    int location_first_endpoint = register_endpoint(addr_str_first_endpoint, query_buffer_first_endpoint, location_str_first_endpoint, 
                                                    payload_first_endpoint, &payload_first_len);

    /* Register the second endpoint */
    int location_second_endpoint = register_endpoint(addr_str_second_endpoint, query_buffer_second_endpoint, location_str_second_endpoint,
                                                     payload_second_endpoint, &payload_second_len);

    /* Register the third endpoint */
    int location_third_endpoint = register_endpoint(addr_str_third_endpoint, query_buffer_third_endpoint, location_str_third_endpoint,
                                                    payload_third_endpoint, &payload_third_len);

    i_list_node node_1, node_2, node_3 = { 0 };

    /* The first expected registered endpoint */
    node_1.location_nr = 1;
    node_1.previous = NULL;
    node_1.next = &registered_endpoints_list[1].node_management;

    Endpoint ep_1 = { .base = "coap://[fe80::cafe:cafe:cafe:5]", .lt = 570, .et = "core.rd-ep", .name = "RIOT-1024239EKAJD98", .sector = "default",
                    .resources = "<resource-link-1>,<resource-link-2>", .node_management = node_1};

    /* The second expected updated endpoint */
    node_2.location_nr = 2;
    node_2.previous = &registered_endpoints_list[0].node_management;
    node_2.next = &registered_endpoints_list[2].node_management;

    Endpoint ep_2 = { .base = "coap://[fe80::cafe:cafe:cafe:6]", .lt = 90000, .et = "oic.r.glucose.medication", .name = "RIOT-CNOAOACSI7867", .sector = "updated",
                    .resources = "<resource-link-9>,<resource-link-10>", .node_management = node_2};

    /* The third expected registered endpoint */
    node_3.location_nr = 3;
    node_3.previous = &registered_endpoints_list[1].node_management;
    node_3.next = NULL;

    Endpoint ep_3 = { .base = "coap://[fe80::cafe:cafe:cafe:7]", .lt = 1200, .et = "core.rd-ep", .name = "RIOT-89234738234238", .sector = "special",
                    .resources = "<resource-link-5>,<resource-link-6>", .node_management = node_3};

    /* Pointers to the registered endpoints */
    i_list_node *node_ptr_1 = &registered_endpoints_list[location_first_endpoint - 1].node_management;
    Endpoint *endpoint_ptr_1 = container_of(node_ptr_1, Endpoint, node_management);

    i_list_node *node_ptr_2 = &registered_endpoints_list[location_second_endpoint - 1].node_management;
    Endpoint *endpoint_ptr_2 = container_of(node_ptr_2, Endpoint, node_management);

    i_list_node *node_ptr_3 = &registered_endpoints_list[location_third_endpoint - 1].node_management;
    Endpoint *endpoint_ptr_3 = container_of(node_ptr_3, Endpoint, node_management);


    char addr_str[IPV6_ADDR_MAX_STR_LEN] = "fe80::cafe:cafe:cafe:6";

    unsigned char query_buffer[QUERY_BUFFER_MAX_LEN] = " lt=500 ep=RIOT-CNOAOACSI7867 d=updated ";

    char payload[RESOURCES_MAX_LEN] = "<resource-link-9>,<resource-link-10>";

    int payload_len = strlen((char *)payload);

    /* Update the second endpoint */
    update_endpoint(payload, &payload_len, query_buffer, endpoint_ptr_2, addr_str);

    /* Equality conditions between the registered endpoints and the expected ones */
    bool condition_ep_1 = check_strings_equality(endpoint_ptr_1->base, ep_1.base) && check_strings_equality(endpoint_ptr_1->et, ep_1.et) && (endpoint_ptr_1->lt == ep_1.lt) &&
                        check_strings_equality(endpoint_ptr_1->name, ep_1.name) && check_strings_equality(endpoint_ptr_1->resources, ep_1.resources) && 
                        check_strings_equality(endpoint_ptr_1->sector, ep_1.sector) && (node_ptr_1 ->location_nr == node_1.location_nr)
                         && (node_ptr_1->previous == node_1.previous) && (node_ptr_1->next == node_1.next);

    bool condition_ep_2 = check_strings_equality(endpoint_ptr_2->base, ep_2.base) && check_strings_equality(endpoint_ptr_2->et, ep_2.et) && (endpoint_ptr_2->lt == ep_2.lt) &&
                            check_strings_equality(endpoint_ptr_2->name, ep_2.name) && check_strings_equality(endpoint_ptr_2->resources, ep_2.resources) 
                            && check_strings_equality(endpoint_ptr_2->sector, ep_2.sector) && (node_ptr_2 ->location_nr == node_2.location_nr)
                             && (node_ptr_2->previous == node_2.previous) && (node_ptr_2->next == node_2.next);

    bool condition_ep_3 = check_strings_equality(endpoint_ptr_3->base, ep_3.base) && check_strings_equality(endpoint_ptr_3->et, ep_3.et) && (endpoint_ptr_3->lt == ep_3.lt) &&
                            check_strings_equality(endpoint_ptr_3->name, ep_3.name) && check_strings_equality(endpoint_ptr_3->resources, ep_3.resources) 
                            && check_strings_equality(endpoint_ptr_3->sector, ep_3.sector) && (node_ptr_3 ->location_nr == node_3.location_nr)
                             && (node_ptr_3->previous == node_3.previous) && (node_ptr_3->next == node_3.next);

    TEST_ASSERT(condition_ep_1 && condition_ep_2 && condition_ep_3);
}

static void test_resource_lookup(void) {

    memset(registered_endpoints_list, 0, sizeof(registered_endpoints_list));
    memset(deleted_registrations_list, 0, sizeof(deleted_registrations_list));
    memset(lookup_result_list, 0, sizeof(lookup_result_list));
    number_registered_endpoints = INITIAL_NUMBER_REGISTERED_ENDPOINTS;
    number_deleted_registrations = INITIAL_NUMBER_DELETED_ENDPOINTS;
    head = NULL;

   /* Initial data structures for the first endpoint */
   char location_str_first_endpoint[LOCATION_STR_MAX_LEN] = { 0 };

   char addr_str_first_endpoint[IPV6_ADDR_MAX_STR_LEN] = "fe80::cafe:cafe:cafe:5";

   unsigned char query_buffer_first_endpoint[QUERY_BUFFER_MAX_LEN] = " lt=570 ep=RIOT-1024239EKAJD98 ";

   uint8_t *payload_first_endpoint = (uint8_t *)"<resource-link-1>,<resource-link-2>";

   uint16_t payload_first_len = strlen((char *)payload_first_endpoint);

   /* Initial data structures for the second endpoint */
   char location_str_second_endpoint[LOCATION_STR_MAX_LEN] = { 0 };

   char addr_str_second_endpoint[IPV6_ADDR_MAX_STR_LEN] = "fe80::cafe:cafe:cafe:6";

   unsigned char query_buffer_second_endpoint[QUERY_BUFFER_MAX_LEN] = " lt=800 ep=RIOT-098503495KJHK et=oic.r.glucose.medication ";

   uint8_t *payload_second_endpoint = (uint8_t *)"<resource-link-3>,<resource-link-4>";

   uint16_t payload_second_len = strlen((char *)payload_second_endpoint);

   /* Initial data structures for the third endpoint */
   char location_str_third_endpoint[LOCATION_STR_MAX_LEN] = { 0 };

   char addr_str_third_endpoint[IPV6_ADDR_MAX_STR_LEN] = "fe80::cafe:cafe:cafe:7";

   unsigned char query_buffer_third_endpoint[QUERY_BUFFER_MAX_LEN] = " lt=1200 ep=RIOT-89234738234238 d=special ";

   uint8_t *payload_third_endpoint = (uint8_t *)"<resource-link-5>,<resource-link-6>";

   uint16_t payload_third_len = strlen((char *)payload_third_endpoint);

   

   /* Register the first endpoint */
   register_endpoint(addr_str_first_endpoint, query_buffer_first_endpoint, location_str_first_endpoint, 
                                                   payload_first_endpoint, &payload_first_len);

   /* Register the second endpoint */
   register_endpoint(addr_str_second_endpoint, query_buffer_second_endpoint, location_str_second_endpoint,
                                                    payload_second_endpoint, &payload_second_len);

   /* Register the third endpoint */
    register_endpoint(addr_str_third_endpoint, query_buffer_third_endpoint, location_str_third_endpoint,
                                                   payload_third_endpoint, &payload_third_len);

    /* Data structures for the lookup */
    char uri_query[CONFIG_NANOCOAP_URI_MAX] = { 0 };
    static char lookup_result[LOOKUP_RESULT_STR_MAX_LEN];
    char relative_uris[RESOURCE_URI_MAX_NUMBER][RESOURCE_URI_MAX_LEN];
    int resource_number = 0;
    memset(lookup_result, 0, sizeof(lookup_result));
    memset(relative_uris, 0, sizeof(relative_uris));

    /* Lookup using the location path as filter*/
    memcpy(uri_query, "href=/reg/2/", strlen((char *)"href=/reg/2/"));

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

    bool condition_1, condition_2, condition_3, condition_4, condition_5;

    if (strstr(lookup_result, "<resource-link-3>,<resource-link-4>") != NULL){

        condition_1 = true;
    }

    /* Lookup using the endpoint name as filter*/
    memset(lookup_result, 0, sizeof(lookup_result));
    memset(relative_uris, 0, sizeof(relative_uris));
    memset(uri_query, 0, sizeof(uri_query));

    memcpy(uri_query, "ep=RIOT-098503495KJHK", strlen((char *)"ep=RIOT-098503495KJHK"));

    char ep_name[ENDPOINT_NAME_MAX_LEN];
    extract_value_from_query((char*)uri_query, ep_name, "ep=");
    Endpoint *ep = find_endpoint_by_pattern(ep_name);
    resource_number = extract_resource_uris(ep->resources, relative_uris);
    build_resource_string(resource_number, relative_uris, lookup_result, ep);

    if (strstr(lookup_result, "<resource-link-3>,<resource-link-4>") != NULL){

        condition_2 = true;
    }

    /* Lookup using the base uri as filter*/
    memset(lookup_result, 0, sizeof(lookup_result));
    memset(relative_uris, 0, sizeof(relative_uris));
    memset(uri_query, 0, sizeof(uri_query));

    memcpy(uri_query, "base=fe80::cafe:cafe:cafe:6", strlen((char *)"base=fe80::cafe:cafe:cafe:6"));

    char base_value[BASE_URI_MAX_LEN];
    extract_value_from_query((char*)uri_query, base_value, "base=");
    find_endpoints_by_pattern(base_value);
    
    build_whole_result_string((uint8_t*)uri_query, lookup_result, NULL, NULL, NULL, NULL,
                                NULL, relative_uris, &resource_number);

    if (strstr(lookup_result, "<resource-link-3>,<resource-link-4>") != NULL){

        condition_3 = true;
    }

    /*Lookup using the endpoint type as filter*/
    memset(lookup_result, 0, sizeof(lookup_result));
    memset(relative_uris, 0, sizeof(relative_uris));
    memset(uri_query, 0, sizeof(uri_query));

    memcpy(uri_query, "et=oic.r.glucose.medication", strlen((char *)"et=oic.r.glucose.medication"));

    char endpoint_type[ENDPOINT_TYPE_MAX_LEN] = { 0 };
    extract_value_from_query((char*)uri_query, endpoint_type, "et=");
    find_endpoints_by_pattern(endpoint_type);

    build_whole_result_string((uint8_t*)uri_query, lookup_result, NULL, NULL, NULL, NULL,
                                NULL, relative_uris, &resource_number);

    if (strstr(lookup_result, "<resource-link-3>,<resource-link-4>") != NULL){

        condition_4 = true;
    }

    /*Lookup using no filtering*/
    memset(lookup_result, 0, sizeof(lookup_result));
    memset(relative_uris, 0, sizeof(relative_uris));
    memset(uri_query, 0, sizeof(uri_query));
    
    get_all_registered_endpoints();

    build_whole_result_string((uint8_t*)uri_query, lookup_result, NULL, NULL, NULL, NULL,
                                NULL, relative_uris, &resource_number);

    if (strstr(lookup_result, "<resource-link-1>,<resource-link-2>") != NULL && 
        strstr(lookup_result, "<resource-link-3>,<resource-link-4>") != NULL && 
        strstr(lookup_result, "<resource-link-5>,<resource-link-6>") != NULL){

        condition_5 = true;
    }

    TEST_ASSERT(condition_1 && condition_2 && condition_3 && condition_4 && condition_5);
}

static void test_endpoint_lookup(void) {

    memset(registered_endpoints_list, 0, sizeof(registered_endpoints_list));
    memset(deleted_registrations_list, 0, sizeof(deleted_registrations_list));
    memset(lookup_result_list, 0, sizeof(lookup_result_list));
    number_registered_endpoints = INITIAL_NUMBER_REGISTERED_ENDPOINTS;
    number_deleted_registrations = INITIAL_NUMBER_DELETED_ENDPOINTS;
    head = NULL;

    /* Initial data structures for the first endpoint */
    char location_str_first_endpoint[LOCATION_STR_MAX_LEN] = { 0 };

    char addr_str_first_endpoint[IPV6_ADDR_MAX_STR_LEN] = "fe80::cafe:cafe:cafe:5";

    unsigned char query_buffer_first_endpoint[QUERY_BUFFER_MAX_LEN] = " lt=570 ep=RIOT-1024239EKAJD98 ";

    uint8_t *payload_first_endpoint = (uint8_t *)"<resource-link-1>,<resource-link-2>";

    uint16_t payload_first_len = strlen((char *)payload_first_endpoint);

    /* Initial data structures for the second endpoint */
    char location_str_second_endpoint[LOCATION_STR_MAX_LEN] = { 0 };

    char addr_str_second_endpoint[IPV6_ADDR_MAX_STR_LEN] = "fe80::cafe:cafe:cafe:6";

    unsigned char query_buffer_second_endpoint[QUERY_BUFFER_MAX_LEN] = " lt=800 ep=RIOT-098503495KJHK et=oic.r.glucose.medication ";

    uint8_t *payload_second_endpoint = (uint8_t *)"<resource-link-3>,<resource-link-4>";

    uint16_t payload_second_len = strlen((char *)payload_second_endpoint);

    /* Initial data structures for the third endpoint */
    char location_str_third_endpoint[LOCATION_STR_MAX_LEN] = { 0 };

    char addr_str_third_endpoint[IPV6_ADDR_MAX_STR_LEN] = "fe80::cafe:cafe:cafe:7";

    unsigned char query_buffer_third_endpoint[QUERY_BUFFER_MAX_LEN] = " lt=1200 ep=RIOT-89234738234238 d=special ";

    uint8_t *payload_third_endpoint = (uint8_t *)"<resource-link-5>,<resource-link-6>";

    uint16_t payload_third_len = strlen((char *)payload_third_endpoint);

    

    /* Register the first endpoint */
    register_endpoint(addr_str_first_endpoint, query_buffer_first_endpoint, location_str_first_endpoint, 
                                                    payload_first_endpoint, &payload_first_len);

    /* Register the second endpoint */
    register_endpoint(addr_str_second_endpoint, query_buffer_second_endpoint, location_str_second_endpoint,
                                                     payload_second_endpoint, &payload_second_len);

    /* Register the third endpoint */
    register_endpoint(addr_str_third_endpoint, query_buffer_third_endpoint, location_str_third_endpoint,
                                                    payload_third_endpoint, &payload_third_len);


    /* Data structures for the lookup */
    char uri_query[CONFIG_NANOCOAP_URI_MAX] = { 0 };
    static char lookup_result[LOOKUP_RESULT_STR_MAX_LEN];
    
    memset(lookup_result, 0, sizeof(lookup_result));

    char* first_bracket = "<";
    char* second_href_bracket = ">;";
    char* ep_key = "ep=\"";
    char* base = "\";base=\"";
    char* rt = "\";rt=\"";

    bool condition_1, condition_2, condition_3, condition_4, condition_5;

    /* Lookup using the location path as filter*/
    memcpy(uri_query, "href=/reg/2/", strlen((char *)"href=/reg/2/"));

    char path[LOCATION_STR_MAX_LEN];
    extract_value_from_query((char*)uri_query, path, "href=");
    int location_nr = extract_number_from_location(path);

    if (location_nr <= number_registered_endpoints && location_nr > 0)
    {
        i_list_node *node_ptr = &registered_endpoints_list[location_nr - 1].node_management;
        Endpoint *endpoint_ptr = container_of(node_ptr, Endpoint, node_management);
        build_result_string(lookup_result, first_bracket, second_href_bracket, ep_key, base, rt, endpoint_ptr, endpoint_ptr->et);
    }

    if (strstr(lookup_result, "ep=RIOT-098503495KJHK") != NULL){

        condition_1 = true;
    }

    /* Lookup using the endpoint name as filter*/
    memset(lookup_result, 0, sizeof(lookup_result));
    memset(uri_query, 0, sizeof(uri_query));

    memcpy(uri_query, "ep=RIOT-098503495KJHK", strlen((char *)"ep=RIOT-098503495KJHK"));

    char ep_name[ENDPOINT_NAME_MAX_LEN];
    extract_value_from_query((char*)uri_query, ep_name, "ep=");
    Endpoint *ep = find_endpoint_by_pattern(ep_name);
    build_result_string(lookup_result, first_bracket, second_href_bracket, ep_key, base, rt, ep, ep->et);

    if (strstr(lookup_result, "ep=RIOT-098503495KJHK") != NULL){

        condition_2 = true;
    }

    /* Lookup using the base uri as filter*/
    memset(lookup_result, 0, sizeof(lookup_result));
    memset(uri_query, 0, sizeof(uri_query));

    memcpy(uri_query, "base=fe80::cafe:cafe:cafe:6", strlen((char *)"base=fe80::cafe:cafe:cafe:6"));

    char base_value[BASE_URI_MAX_LEN];
    extract_value_from_query((char*)uri_query, base_value, "base=");
    find_endpoints_by_pattern(base_value);

    build_whole_result_string((uint8_t*)uri_query, lookup_result, first_bracket, second_href_bracket, ep_key, base,
                                rt, NULL, NULL);

    if (strstr(lookup_result, "ep=RIOT-098503495KJHK") != NULL){

        condition_3 = true;
    }

    /*Lookup using the endpoint type as filter*/
    memset(lookup_result, 0, sizeof(lookup_result));
    memset(uri_query, 0, sizeof(uri_query));

    memcpy(uri_query, "et=oic.r.glucose.medication", strlen((char *)"et=oic.r.glucose.medication"));

    char endpoint_type[ENDPOINT_TYPE_MAX_LEN] = { 0 };
    extract_value_from_query((char*)uri_query, endpoint_type, "et=");
    find_endpoints_by_pattern(endpoint_type);

    build_whole_result_string((uint8_t*)uri_query, lookup_result, first_bracket, second_href_bracket, ep_key, base,
                                rt, NULL, NULL);

    if (strstr(lookup_result, "ep=RIOT-098503495KJHK") != NULL){

        condition_4 = true;
    }

    /*Lookup using no filtering*/
    memset(lookup_result, 0, sizeof(lookup_result));
    memset(uri_query, 0, sizeof(uri_query));
    
    get_all_registered_endpoints();

    build_whole_result_string((uint8_t*)uri_query, lookup_result, first_bracket, second_href_bracket, ep_key, base,
                                rt, NULL, NULL);

    if (strstr(lookup_result, "ep=RIOT-1024239EKAJD98") != NULL && 
        strstr(lookup_result, "ep=RIOT-098503495KJHK") != NULL && 
        strstr(lookup_result, "ep=RIOT-89234738234238") != NULL){

        condition_5 = true;
    }

    TEST_ASSERT(condition_1 && condition_2 && condition_3 && condition_4 && condition_5);
}


Test *tests_rd_tests(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_register_endpoint_three_nodes),
        new_TestFixture(test_registration_idempotent),
        new_TestFixture(test_lifetime_expiration),
        new_TestFixture(test_delete_endpoint),
        new_TestFixture(test_update_node),
        new_TestFixture(test_resource_lookup),
        new_TestFixture(test_endpoint_lookup),
    };

    EMB_UNIT_TESTCALLER(rd_tests, NULL, NULL, fixtures);

    return (Test *)&rd_tests;
}

int main(void) {
    TestRunner_start();
    TestRunner_runTest(tests_rd_tests());
    TestRunner_end();
    return 0;
}