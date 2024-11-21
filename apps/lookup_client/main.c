#include <stdio.h>

#include "net/cord/lc.h"
#include "net/gcoap.h"
//#include "phydat.h"
//#include "board.h"
//#include "net/gnrc.h"
//#include "net/ipv6/addr.h"
//#include "net/gnrc/netif.h"

#include "net/sock/udp.h"
#include "net/sock/util.h"
#include "net/ipv6.h"
#include <netdev_tap.h>

#include "shell.h"
#include "msg.h"
#include "board.h"



int main(void) {


 
    sock_udp_ep_t remote = { .family = AF_INET6 };

    remote.port = 5683;

    ipv6_addr_set_all_nodes_multicast((ipv6_addr_t *)&remote.addr.ipv6,
                                    IPV6_ADDR_MCAST_SCP_LINK_LOCAL);

    cord_lc_rd_t rd = { &remote, "/resource-lookup/", "/endpoint-lookup/", 0, 0};

    //cord_lc_res_t result;

    //cord_lc_filter_t filters;

    

    //int res = _lookup_result(&rd, &result, NULL, lc_buffer, sizeof(lc_buffer) - 1, CORD_LC_RES); 	

    static char lc_buffer[10000];
    /*
    cord_lc_ep_t  endpoint;

    int res = cord_lc_ep(&rd, &endpoint, NULL, lc_buffer, sizeof(lc_buffer) - 1);

    printf("result: %d, %s \n", res, lc_buffer);	
    */
   
    //int res = cord_lc_raw(&rd, COAP_FORMAT_LINK, CORD_LC_EP, NULL, lc_buffer, sizeof(lc_buffer) - 1); 	
    //printf("result: %d, %s \n", res, lc_buffer); 
   

   //int res = cord_lc_rd_init(&rd, lc_buffer, sizeof(lc_buffer) -1, &remote); 	


   //printf("result: %d, %s \n", res, lc_buffer);
    cord_lc_res_t resource;
   
    int res = cord_lc_res(&rd, &resource, NULL, lc_buffer, sizeof(lc_buffer) -1); 	
    
    printf("result: %d, %s \n", res, lc_buffer);

    return 0;
}