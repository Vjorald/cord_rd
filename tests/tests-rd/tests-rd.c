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

/* your macros */

/* your global variables */

uint8_t _epsim_req_buf[CONFIG_GCOAP_PDU_BUF_SIZE] = { 0 };

coap_pkt_t epsim_pkt = { 0 };

sock_udp_t epsim_sock = { 0 };

sock_udp_ep_t epsim_remote = { 0 };

int location_epsim_endpoint = -1;

intrusive_list_node *head;

Endpoint list[REGISTERED_ENDPOINTS_MAX_NUMBER];

Endpoint deleted_registrations_list[DELETED_ENDPOINTS_MAX_NUMBER];

Endpoint lookup_result_list[LOOKUP_RESULTS_MAX_LEN];

ztimer_t lifetime_expiries[REGISTERED_ENDPOINTS_MAX_NUMBER];

ztimer_t epsim_request_before_lifetime_expiry[REGISTERED_ENDPOINTS_MAX_NUMBER];

ztimer_t epsim_request_cache_expiry[REGISTERED_ENDPOINTS_MAX_NUMBER];

epsim_callback_data callback_data_list[REGISTERED_ENDPOINTS_MAX_NUMBER];

int number_registered_endpoints = INITIAL_NUMBER_REGISTERED_ENDPOINTS;

int number_deleted_registrations = INITIAL_NUMBER_DELETED_ENDPOINTS;

static void set_up(void)
{
    /* omit if not needed */
}

static void tear_down(void)
{
    /* omit if not needed */
}

bool check_strings_equality(char* str_1, char* str_2){

    if (strcmp(str_1, str_2) == 0){

        return true;

    }else{

        return false;
    }
    
    return false;

}

static void test_register_endpoint_three_nodes(void) {
    /* Initial data structures for the first endpoint */
    char location_str_first_endpoint[LOCATION_STR_MAX_LEN] = { 0 };

    char addr_str_first_endpoint[IPV6_ADDR_MAX_STR_LEN] = "fe80::cafe:cafe:cafe:5";

    unsigned char query_buffer_first_endpoint[QUERY_BUFFER_MAX_LEN] = " lt=570 ep=RIOT-1024239EKAJD98 ";

    char *payload_first_endpoint = "<resource-link-1>,<resource-link-2>";

    /* Initial data structures for the second endpoint */
    char location_str_second_endpoint[LOCATION_STR_MAX_LEN] = { 0 };

    char addr_str_second_endpoint[IPV6_ADDR_MAX_STR_LEN] = "fe80::cafe:cafe:cafe:6";

    unsigned char query_buffer_second_endpoint[QUERY_BUFFER_MAX_LEN] = " lt=800 ep=RIOT-098503495KJHK et=oic.r.glucose.medication ";

    char *payload_second_endpoint = "<resource-link-3>,<resource-link-4>";

    /* Initial data structures for the third endpoint */
    char location_str_third_endpoint[LOCATION_STR_MAX_LEN] = { 0 };

    char addr_str_third_endpoint[IPV6_ADDR_MAX_STR_LEN] = "fe80::cafe:cafe:cafe:7";

    unsigned char query_buffer_third_endpoint[QUERY_BUFFER_MAX_LEN] = " lt=1200 ep=RIOT-89234738234238 d=special ";

    char *payload_third_endpoint = "<resource-link-5>,<resource-link-6>";

    /* Register the first endpoint */
    int location_first_endpoint = register_endpoint(addr_str_first_endpoint, query_buffer_first_endpoint, location_str_first_endpoint, 
                                                    payload_first_endpoint, strlen(payload_first_endpoint));

    /* Register the second endpoint */
    int location_second_endpoint = register_endpoint(addr_str_second_endpoint, query_buffer_second_endpoint, location_str_second_endpoint,
                                                     payload_second_endpoint, strlen(payload_second_endpoint));

    /* Register the third endpoint */
    int location_third_endpoint = register_endpoint(addr_str_third_endpoint, query_buffer_third_endpoint, location_str_third_endpoint,
                                                    payload_third_endpoint, strlen(payload_third_endpoint));

    intrusive_list_node node_1, node_2, node_3 = { 0 };

    /* The first expected registered endpoint */
    node_1.location_nr = 1;
    node_1.previous = NULL;
    node_1.next = &node_2;

    Endpoint ep_1 = { .base = "coap://[fe80::cafe:cafe:cafe:5]", .lt = 570, .et = "core.rd-ep", .name = "RIOT-1024239EKAJD98", .sector = "default",
                     .ressources = "<resource-link-1>,<resource-link-2>", .node_management = node_1};

    /* The second expected registered endpoint */
    node_2.location_nr = 2;
    node_2.previous = &node_1;
    node_2.next = &node_3;

    Endpoint ep_2 = { .base = "coap://[fe80::cafe:cafe:cafe:6]", .lt = 800, .et = "oic.r.glucose.medication", .name = "RIOT-098503495KJHK", .sector = "default",
                     .ressources = "<resource-link-3>,<resource-link-4>", .node_management = node_2};

    /* The third expected registered endpoint */
    node_3.location_nr = 3;
    node_3.previous = &node_2;
    node_3.next = NULL;

    Endpoint ep_3 = { .base = "coap://[fe80::cafe:cafe:cafe:7]", .lt = 1200, .et = "core.rd-ep", .name = "RIOT-89234738234238", .sector = "special",
                     .ressources = "<resource-link-5>,<resource-link-6>", .node_management = node_3};

    /* Pointers to the registered endpoints */
    intrusive_list_node *node_ptr_1 = &list[location_first_endpoint - 1].node_management;
    Endpoint *endpoint_ptr_1 = container_of(node_ptr_1, Endpoint, node_management);

    intrusive_list_node *node_ptr_2 = &list[location_second_endpoint - 1].node_management;
    Endpoint *endpoint_ptr_2 = container_of(node_ptr_2, Endpoint, node_management);

    intrusive_list_node *node_ptr_3 = &list[location_third_endpoint - 1].node_management;
    Endpoint *endpoint_ptr_3 = container_of(node_ptr_3, Endpoint, node_management);

    /* Equality conditions between the registered endpoints and the expected ones */
    bool condition_ep_1 = check_strings_equality(endpoint_ptr_1->base, ep_1.base) && check_strings_equality(endpoint_ptr_1->et, ep_1.et) && (endpoint_ptr_1->lt == ep_1.lt) &&
                        check_strings_equality(endpoint_ptr_1->name, ep_1.name) && check_strings_equality(endpoint_ptr_1->ressources, ep_1.ressources) && 
                        check_strings_equality(endpoint_ptr_1->sector, ep_1.sector) && (node_ptr_1 ->location_nr == node_1.location_nr)
                         && (node_ptr_1->previous == node_1.previous) && (node_ptr_1->next == node_1.next);

    bool condition_ep_2 = check_strings_equality(endpoint_ptr_2->base, ep_2.base) && check_strings_equality(endpoint_ptr_2->et, ep_2.et) && (endpoint_ptr_2->lt == ep_2.lt) &&
                            check_strings_equality(endpoint_ptr_2->name, ep_2.name) && check_strings_equality(endpoint_ptr_2->ressources, ep_2.ressources) 
                            && check_strings_equality(endpoint_ptr_2->sector, ep_2.sector) && (node_ptr_2 ->location_nr == node_2.location_nr)
                             && (node_ptr_2->previous == node_2.previous) && (node_ptr_2->next == node_2.next);

    bool condition_ep_3 = check_strings_equality(endpoint_ptr_3->base, ep_3.base) && check_strings_equality(endpoint_ptr_3->et, ep_3.et) && (endpoint_ptr_3->lt == ep_3.lt) &&
                            check_strings_equality(endpoint_ptr_3->name, ep_3.name) && check_strings_equality(endpoint_ptr_3->ressources, ep_3.ressources) 
                            && check_strings_equality(endpoint_ptr_3->sector, ep_3.sector) && (node_ptr_3 ->location_nr == node_3.location_nr)
                             && (node_ptr_3->previous == node_3.previous) && (node_ptr_3->next == node_3.next);
                        
    TEST_ASSERT(condition_ep_1 && condition_ep_2 && condition_ep_3);
}

static void test_<function1>_<what2>(void) {
    /* ... */

    TEST_ASSERT(/* ... */);
}

/* ... */

static void test_<function2>_<what1>(void) {
    /* ... */

    TEST_ASSERT(/* ... */);
}

static void test_<function2>_<what2>(void) {
    /* ... */

    TEST_ASSERT(/* ... */);
}

/* ... */

Test *tests_<module>_<header>_tests(void)
{
    EMB_UNIT_TESTFIXTURES(fixtures) {
        new_TestFixture(test_<function1>_<what1>),
        new_TestFixture(test_<function1>_<what2>),
        new_TestFixture(test_<function2>_<what1>),
        new_TestFixture(test_<function2>_<what2>),
        /* ... */
    };

    EMB_UNIT_TESTCALLER(<module>_<header>_tests, set_up, tear_down, fixtures);
    /* set up and tear down function can be NULL if omitted */

    return (Test *)&<module>_<header>_tests;
}