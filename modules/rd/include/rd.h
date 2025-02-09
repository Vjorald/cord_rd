#include "i_list.h"
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
#define ENDPOINT_NAME_MAX_LEN 63
#define RESOURCES_MAX_LEN 100
#define SECTOR_NAME_MAX_LEN 63
#define MAX_PAGE_DIGIT_NUMBER 3
#define MAX_COUNT_DIGIT_NUMBER 3
#define REGISTERED_ENDPOINTS_MAX_NUMBER 100
#define DELETED_ENDPOINTS_MAX_NUMBER 100
#define LOOKUP_RESULTS_MAX_LEN 100
#define ENDPOINT_TYPE_MAX_LEN 50
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

typedef struct rd{
    char base[BASE_URI_MAX_LEN];
    char name[ENDPOINT_NAME_MAX_LEN];
    int lt;
    char et[ENDPOINT_TYPE_MAX_LEN];
    char sector[SECTOR_NAME_MAX_LEN];
    char resources[RESOURCES_MAX_LEN];
    bool epsim;
    nanocoap_cache_entry_t *cache;
    sock_udp_ep_t *remote;
    coap_pkt_t epsim_pkt;
    sock_udp_t epsim_sock;
    i_list_node node_management;
} Endpoint;


extern i_list_node *head;

extern Endpoint list[REGISTERED_ENDPOINTS_MAX_NUMBER];

extern Endpoint deleted_registrations_list[DELETED_ENDPOINTS_MAX_NUMBER];

extern Endpoint lookup_result_list[LOOKUP_RESULTS_MAX_LEN];

extern ztimer_t lifetime_expiry;

extern ztimer_t epsim_get_request;

extern int number_registered_endpoints;

extern int number_deleted_registrations;


void resource_directory_init(void);

void parse_query_buffer(unsigned char *query_buffer, char *ep, char *lt, char *et, char *sector, char *base);

void build_location_string(int location_nr, char* location_str);

void build_base_uri_string(char* addr_str, char* base_uri);

void find_endpoints_by_pattern(char* pattern);

Endpoint *find_endpoint_by_pattern(char* pattern);

int check_existing_endpoint(char *ep_name, char* sector);

void update_endpoint(char *payload, int *payload_len, unsigned char *query_buffer, Endpoint *endpoint_ptr, char *addr_str);

int register_endpoint(char *addr_str, unsigned char *query_buffer, char *location_str, char *payload, int *payload_len);

void initialize_endpoint(char *lifetime, char *endpoint_name, Endpoint *endpoint_ptr, i_list_node *node_ptr, char *base_uri, char *payload, int *payload_len, char* location_str, int location_nr, char *et, char *sector);

int get_next_empty_location(Endpoint* deleted_list);

int get_previous_endpoint_location(int actual_location);

int extract_number_from_location(char *path);

void extract_value_from_query(const char *input, char *href_value, char* pref);

void build_result_string(char* lookup_result, char* first_bracket, char* second_href_bracket, char* ep_key, char* base, char* rt, Endpoint* endpoint, char *et);

void build_resource_string(int number_sensors, char extracted_sensor_uris[RESOURCE_URI_MAX_NUMBER][RESOURCE_URI_MAX_LEN], char* lookup_result, Endpoint* endpoint);

void build_whole_result_string(uint8_t *uri_query, char *lookup_result, char *first_bracket, char *second_href_bracket, char *ep_key,
                                char *base, char *rt, char relative_uris[RESOURCE_URI_MAX_NUMBER][RESOURCE_URI_MAX_LEN], int *resource_number);

int extract_resource_uris(const char *input, char uris[RESOURCE_URI_MAX_NUMBER][RESOURCE_URI_MAX_LEN]);

void send_get_request(Endpoint *endpoint_ptr);

int printList(Endpoint* endpoint);

void get_all_registered_endpoints(void);

void append_endpoint(i_list_node *new_node);

void connect_endpoint_with_the_rest(i_list_node *node_ptr, int location_nr);

void disconnect_endpoint_from_the_rest(int location_nr, i_list_node *node_ptr);

void delete_endpoint(int location_nr);

void update_registration_lifetimes(int expired_lifetime);

i_list_node* find_next_expiring_endpoint(void);


