#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "random.h"
#include "rd.h"
#include "rd_utilities.h"

#define EP_NAME_LEN 15 
#define QUERY_LIST_SIZE 1000
#define IPV6_LIST_SIZE 1000
#define NUMBER_CYCLES_BENCHMARK 1000

void generate_random_ep_name(char *buffer, size_t length) {
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    strcpy(buffer, "RIOT-"); 
    
    for (size_t i = 5; i < length - 1; i++) { 
        buffer[i] = charset[random_uint32() % (sizeof(charset) - 1)];
    }
    buffer[length - 1] = '\0'; 
}

void generate_query_list(unsigned char list[QUERY_LIST_SIZE][QUERY_BUFFER_MAX_LEN], const char *base_strings[], size_t base_count) {
    for (size_t i = 0; i < QUERY_LIST_SIZE; i++) {
        char random_str[EP_NAME_LEN + 6];
        generate_random_ep_name(random_str, sizeof(random_str));
        strcpy((char *)list[i], base_strings[i % base_count]);
        strcat((char *)list[i], "ep=");
        strcat((char *)list[i], random_str);
        strcat((char *)list[i], " ");
    }
}

void generate_random_ipv6(char *buffer) {

    buffer[0] = '\0'; 
    for (size_t i = 0; i < 8; i++) {
        if (i > 0) {
            strcat(buffer, ":");
        }
        char segment[5];
        unsigned int value = random_uint32() & 0xFFFF;
        for (int j = 3; j >= 0; j--) {
            segment[j] = "0123456789abcdef"[value % 16];
            value /= 16;
        }
        segment[4] = '\0';
        strcat(buffer, segment);
    }
}

void generate_ipv6_list(char list[IPV6_LIST_SIZE][IPV6_ADDR_MAX_STR_LEN]) {
    for (size_t i = 0; i < IPV6_LIST_SIZE; i++) {
        generate_random_ipv6(list[i]);
    }
}

void benchmark_sequential_registrations(void);

void benchmark_idempotent_last_registration_existing(void);

void benchmark_registration_deleted_middle_elements(void);

void benchmark_sequential_epsim_registrations(void);

void benchmark_idempotent_last_epsim_registration_existing(void);

void benchmark_epsim_registration_deleted_middle_elements(void);

void benchmark_update_last_element(void);

void benchmark_delete_last_element(void);

void benchmark_resource_lookup_href(void);

void benchmark_resource_lookup_ep_name(void);

void benchmark_resource_lookup_base_uri(void);

void benchmark_resource_lookup_et(void);

void benchmark_resource_lookup_default(void);

void benchmark_ep_lookup_href(void);

void benchmark_ep_lookup_ep_name(void);

void benchmark_ep_lookup_base_uri(void);

void benchmark_ep_lookup_et(void);

void benchmark_ep_lookup_default(void);