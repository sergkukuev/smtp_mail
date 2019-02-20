#include "sockets.h"

#include <stdlib.h>
#include <stdio.h>

#include <errno.h>

#define SERVER_FAILED -1
#define SERVER_FINISH 0

int init_server(void)
{
    struct ss_node_t* head_fd = init_serv_sockets();
    if (head_fd == NULL)
        return SERVER_FAILED;

    /*  TODO: 
        
        init_logger()
        init_processes()
    */
   return 0;
}

int run_server()
{
    int state = 1;
    while(state == 1) { state = 0; }
    return state;
}

// parse command line
void parse_cmd(int argc, char** argv)
{
    // TODO: port, address, num-processes, etc.
}

/////////////////////////////////////////////////////////////////////
// MAIN FUNCTION
int main(int argc, char** argv)
{
    parse_cmd(argc, argv);
    if (init_server() == SERVER_FAILED) {
        perror("init_server() failed");
        exit(SERVER_FAILED);
    }

    return run_server();
}