#include "sockets.h"
#include "process.h"
#include "logger.h"

#include "common.h"
#include <mqueue.h>

#define SERVER_FAILED -1
#define SERVER_SUCCESS 0

// pid_t processes and logger
int init_server(pid_t* pr, pid_t* lg)
{
    int sfd = init_listen_socket();
    if (sfd < 0)
        return SERVER_FAILED;

    *lg = create_logger();
    if (*lg == -1)
        return SERVER_FAILED;

    int n = 2;
    pr = create_processes(sfd, &n, *lg);
    if (*pr == -1)
        return SERVER_FAILED;
    return SERVER_SUCCESS;
}

int run_server(pid_t pr, pid_t lg)
{
    // period send info we are alive 
    char bf[100];
    sprintf(bf, "/process%d", lg);
    mqd_t mq = mq_open(bf, O_WRONLY); // bf - log queue
    while(1) {
        sprintf(bf, "%s", "server is alive");
        mq_log(mq, bf);
        sleep(200);
    }
    return SERVER_SUCCESS;
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
    pid_t pr, lg;
    if (init_server(&pr, &lg) == SERVER_FAILED) {
        perror("init_server() failed");
        exit(SERVER_FAILED);
    }

    return run_server(pr, lg);
}