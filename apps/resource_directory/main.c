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
/*
void message_callback(void *argument)
{
    char *message = (char *)argument;
    puts(message);
}
*/

int main(void)
{ 
    /*
    uint32_t expiration_time = ztimer_now(ZTIMER_SEC) + 5;

    printf("Expiration time: %d\n", expiration_time);

    ztimer_sleep(ZTIMER_SEC, 2);

    uint32_t time_left = expiration_time - ztimer_now(ZTIMER_SEC);

    printf("%d seconds left\n", time_left);
    */
/*
    ztimer_now_t first = ztimer_now(ZTIMER_SEC);

    ztimer_sleep(ZTIMER_SEC, 2);

    ztimer_now_t second = ztimer_now(ZTIMER_SEC);

    ztimer_sleep(ZTIMER_SEC, 2);

    ztimer_now_t third = ztimer_now(ZTIMER_SEC);

    ztimer_t timeout;                    
    timeout.callback = message_callback; 
    timeout.arg = "Timeout!";             
    ztimer_set(ZTIMER_SEC, &timeout, 2);


    char str[20] = { 0 };   // Buffer to hold the resulting string

    sprintf(str, "%d", second + first); // Convert integer to string

    ztimer_t timeout_1;                    
    timeout_1.callback = message_callback; 
    timeout_1.arg = str;             
    ztimer_set(ZTIMER_SEC, &timeout_1, 4);

    char str1[20] = { 0 }; 

    sprintf(str1, "%d", third + first);

    ztimer_t timeout_2;                    
    timeout_2.callback = message_callback; 
    timeout_2.arg = str1;             
    ztimer_set(ZTIMER_SEC, &timeout_2, 6);
*/
    resource_directory_init();
    
    return 0;
}