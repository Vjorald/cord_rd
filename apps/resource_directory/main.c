#include "rd.h"


//#define MAIN_QUEUE_SIZE (4)
//static msg_t _main_msg_queue[MAIN_QUEUE_SIZE];

int main(void)
{ 

    gnrc_netif_t *netif = gnrc_netif_iter(NULL);
    if (netif) {

        printf("Network interface initialized!\n");
    }
/*
    msg_init_queue(_main_msg_queue, MAIN_QUEUE_SIZE);

    puts("All up, running the shell now");
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(NULL, line_buf, SHELL_DEFAULT_BUFSIZE);
*/
    resource_directory_init();
    
    return 0;
}