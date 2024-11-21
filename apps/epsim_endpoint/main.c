#include <stdio.h>

#include "net/cord/epsim.h"
#include <net/gnrc/netif.h>
#include "net/gcoap.h"

#include "net/sock/udp.h"
#include "net/sock/util.h"
#include "net/ipv6.h"
//#include <netdev_tap.h>

#include "shell.h"
#include "msg.h"
#include "board.h"
#include "periph/gpio.h"
#include "net/cord/common.h"
#include "net/cord/config.h"
#include "ztimer.h"


static ssize_t _riot_board_handler(coap_pkt_t* pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx);


static ssize_t _led_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx);


static const gpio_t leds[] = {
    GPIO_PIN(1, 9),
    GPIO_PIN(1, 10),
};


static const coap_resource_t _resources[] = {
   
    { "/riot/boards", COAP_GET, _riot_board_handler, NULL },
    { "/led/", COAP_GET | COAP_PUT | COAP_MATCH_SUBTREE, _led_handler, NULL },
};

static gcoap_listener_t _listener = {
    _resources,
    ARRAY_SIZE(_resources),
    GCOAP_SOCKET_TYPE_UDP,
    NULL,
    NULL,
    NULL
};

char * configure_network(sock_udp_ep_t * remote, char *addr, char *port) {
    
     char *full_addr = (char*)malloc(50 * sizeof(char));
     strcat(full_addr, "[");
     strcat(full_addr, addr);
     strcat(full_addr, "]:");
     strcat(full_addr, port);

     //printf("%s", full_addr);

     if (sock_udp_str2ep(remote, full_addr) < 0) {
        puts("Unable to parse destination address");
        return "Unable to parse destination address";
    }
    return full_addr;
}

char * send_message(sock_udp_ep_t * remote, char *message, int *res, char *full_addr){

    if((*res = sock_udp_send(NULL, message, strlen(message), remote)) < 0) {
        puts("could not send");
        return "could not send";
    }
    else {
        printf("Success: send %u byte to %s\n", (unsigned) *res, full_addr);
        return "Success!";
    }
}

static ssize_t _led_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx)
{
    (void) ctx; 

    
    char uri[CONFIG_NANOCOAP_URI_MAX] = { 0 };
    
    if (coap_get_uri_path(pdu, (uint8_t *)uri) <= 0) {
        
        return gcoap_response(pdu, buf, len, COAP_CODE_BAD_REQUEST);
    }

    
    char *led_str = uri + strlen("/led/");
    unsigned led_number = atoi(led_str);

    
    if (led_number >= ARRAY_SIZE(leds)) {
        return gcoap_response(pdu, buf, len, COAP_CODE_BAD_REQUEST);
    }

    ssize_t resp_len = 0;
    int led_status = 0;
    unsigned method = coap_method2flag(coap_get_code_detail(pdu));

    switch (method) {
    case COAP_PUT: 
        if (pdu->payload_len) {
            led_status = atoi((char *)pdu->payload);
        } else {
            return gcoap_response(pdu, buf, len, COAP_CODE_BAD_REQUEST);
        }

        if (led_status) {
            gpio_set(leds[led_number]);
        } else {
            gpio_clear(leds[led_number]);
        }
        return gcoap_response(pdu, buf, len, COAP_CODE_CHANGED);
    case COAP_GET: 
        gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);

       
        coap_opt_add_format(pdu, COAP_FORMAT_TEXT);

        
        resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);

       
        led_status = gpio_read(leds[led_number]);

        if (led_status) {
            pdu->payload[0] = '1';
        } else {
            pdu->payload[0] = '0';
        }
        resp_len++;
        return resp_len;
}

    return 0;
}

static ssize_t _riot_board_handler(coap_pkt_t *pdu, uint8_t *buf, size_t len, coap_request_ctx_t *ctx)
{
    (void)ctx;

    
    gcoap_resp_init(pdu, buf, len, COAP_CODE_CONTENT);

  
    coap_opt_add_format(pdu, COAP_FORMAT_TEXT);

    size_t resp_len = coap_opt_finish(pdu, COAP_OPT_FINISH_PAYLOAD);


    if (pdu->payload_len >= strlen("native")) {
        memcpy(pdu->payload, "native", strlen("native"));
        return resp_len + strlen("native");
    }
    else {

        puts("gcoap_cli: msg buffer too small");
        return gcoap_response(pdu, buf, len, COAP_CODE_INTERNAL_SERVER_ERROR);
    }
}

int main(void) {


    gcoap_register_listener(&_listener);

    sock_udp_t sock;

    sock_udp_ep_t endpoint = { 0 };
    

    if (sock_udp_str2ep(&endpoint, "[fe80::cafe:cafe:cafe:2]:5683") < 0) {
        puts("Unable to parse destination address");
        return 1;
    }

    if(sock_udp_create(&sock, &endpoint, NULL, 0) < 0) {
        printf("Sock creation unsuccessful!");
    }



    sock_udp_ep_t remote = { .family = AF_INET6 };
    remote.port = 5683;

    ipv6_addr_set_all_nodes_multicast((ipv6_addr_t *)&remote.addr.ipv6,
                                   IPV6_ADDR_MCAST_SCP_LINK_LOCAL);
    /*
    puts("All up, running the shell now");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);
    */


    static char endpoint_buffer[10000];
    //printf("Initiated, result: %d", res);
   // int res = cord_ep_discover_regif(&remote, endpoint_buffer, sizeof(endpoint_buffer) - 1);


    int res = cord_epsim_register(&remote);

    printf("%s, result: %d \n", endpoint_buffer, res);


    return 0;
}