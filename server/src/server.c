#include "sockets.h"
#include "process.h"

#include "common.h"

#include <errno.h>

#define SERVER_FAILED -1
#define SERVER_FINISH 0

int init_server(void)
{
    struct ss_node_t* fds = init_serv_sockets();
    if (fds == NULL)
        return SERVER_FAILED;

    pid_t pid = create_process(fds);
    if (pid == -1)
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