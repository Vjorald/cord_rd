#include <stdio.h>
#include "net/gcoap.h"
#include "net/cord/lc.h"
#include "net/sock/udp.h"
#include "net/sock/util.h"
#include "net/ipv6.h"
#include "board.h"

void resource_lookup(cord_lc_rd_t *rd, char *lc_buffer){

    clif_t link;
    link.target = "led";
    link.target_len = strlen(link.target);
     
    clif_attr_t attrs[4];

    clif_attr_t attr0 = { 0 };
    attr0.key = "ct";
    attr0.key_len = strlen(attr0.key);
    attr0.value = "40";
    attr0.value_len = strlen(attr0.value);

    clif_attr_t attr1 = { 0 };
    attr1.key = "rt";
    attr1.key_len = strlen(attr1.key);
    attr1.value = "light";
    attr1.value_len = strlen(attr1.value);

    attrs[0] = attr0;
    attrs[1] = attr1;

    link.attrs = attrs;
    link.attrs_len = sizeof(link.attrs);

    cord_lc_res_t result;
    result.link = link;
    result.attrs = attrs;
    result.max_attrs = sizeof(link.attrs);

    cord_lc_res(rd, &result, NULL, lc_buffer, sizeof(lc_buffer) - 1);
}


void lookup_result(cord_lc_rd_t *rd, char *lc_buffer){

    clif_t link;
    link.target = "led";
    link.target_len = strlen(link.target);
     
    clif_attr_t attrs[4];

    clif_attr_t attr0 = { 0 };
    attr0.key = "ct";
    attr0.key_len = strlen(attr0.key);
    attr0.value = "40";
    attr0.value_len = strlen(attr0.value);

    clif_attr_t attr1 = { 0 };
    attr1.key = "rt";
    attr1.key_len = strlen(attr1.key);
    attr1.value = "light";
    attr1.value_len = strlen(attr1.value);

    attrs[0] = attr0;
    attrs[1] = attr1;

    link.attrs = attrs;
    link.attrs_len = sizeof(link.attrs);

    cord_lc_res_t result;
    result.link = link;
    result.attrs = attrs;
    result.max_attrs = sizeof(link.attrs);

    _lookup_result(rd, &result, NULL, lc_buffer, sizeof(lc_buffer) - 1, CORD_LC_RES);

}


void endpoint_lookup(cord_lc_rd_t *rd, char *lc_buffer){

    clif_t link;
    link.target = "led";
    link.target_len = strlen(link.target);
     
    clif_attr_t attrs[4];

    clif_attr_t attr0 = { 0 };
    attr0.key = "ct";
    attr0.key_len = strlen(attr0.key);
    attr0.value = "40";
    attr0.value_len = strlen(attr0.value);

    clif_attr_t attr1 = { 0 };
    attr1.key = "rt";
    attr1.key_len = strlen(attr1.key);
    attr1.value = "light";
    attr1.value_len = strlen(attr1.value);

    attrs[0] = attr0;
    attrs[1] = attr1;

    link.attrs = attrs;
    link.attrs_len = sizeof(link.attrs);

    cord_lc_ep_t  endpoint;
    endpoint.link = link;
    endpoint.attrs = attrs;
    endpoint.max_attrs = sizeof(link.attrs);

    cord_lc_ep(rd, &endpoint, NULL, lc_buffer, sizeof(lc_buffer) - 1);
}


void raw_lookup(cord_lc_rd_t *rd, char *lc_buffer){

    clif_attr_t filter = { 0 };

    filter.value = "reg/1/";
    filter.value_len = strlen(filter.value);
    filter.key = "Location";
    filter.key_len = strlen(filter.key);

    clif_attr_t array_of_filters[4];

    array_of_filters[0] = filter;


    clif_attr_t *ptr_to_array_of_filters = array_of_filters;

    cord_lc_filter_t filters;
    filters.array = ptr_to_array_of_filters;
    filters.len = 1;
    filters.next = NULL;

    cord_lc_raw(rd, COAP_FORMAT_LINK, CORD_LC_EP, &filters, lc_buffer, sizeof(lc_buffer) - 1);
}


int main(void) {

 
    sock_udp_ep_t remote = { .family = AF_INET6 , .port = 5683 };


    ipv6_addr_set_all_nodes_multicast((ipv6_addr_t *)&remote.addr.ipv6,
                                    IPV6_ADDR_MCAST_SCP_LINK_LOCAL);

    cord_lc_rd_t rd = { &remote, "/resource-lookup/", "/endpoint-lookup/", 0, 0};


    static char lc_buffer[100] = { 0 };

    memset(lc_buffer, 0, sizeof(lc_buffer));

    cord_lc_raw(&rd, COAP_FORMAT_LINK, CORD_LC_EP, NULL, lc_buffer, sizeof(lc_buffer) - 1);

    printf("Endpoints: %s\n", lc_buffer);

    memset(lc_buffer, 0, sizeof(lc_buffer));

    cord_lc_raw(&rd, COAP_FORMAT_LINK, CORD_LC_RES, NULL, lc_buffer, sizeof(lc_buffer) - 1);

    printf("Resources: %s\n", lc_buffer);

    memset(lc_buffer, 0, sizeof(lc_buffer));


    return 0;
}