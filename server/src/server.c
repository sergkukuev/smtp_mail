#include "sockets.h"
#include "process.h"
#include "logger.h"

#include "common.h"
#include <sys/wait.h>
#include <signal.h>
#include <mqueue.h>

// failed initialization
#define NO_FAILED 0
#define FAILED_LSOCKET -2
#define FAILED_LOGGER -3
#define FAILED_PROCESSES -4

// server struct 
struct server_t {
    pid_t* workers;
    pid_t logger;
    int nworkers;
    int lstfd;  // listener fd
};

static struct server_t server;

void graceful_exit(int sig)
{
    signal(sig, SIG_IGN);
    printf("\nServer(%d): start gracefull close\n", getpid());
    // connect to logger message queue
    char name[BUFFER_SIZE];
    sprintf(name, "/process%d", server.logger);
    int lg = mq_open(name, O_WRONLY);
    if (lg == -1) {
        perror("graceful_exit() failed");
        exit(EXIT_FAILURE);
    }
    // spam exit for workers
    for (int i = 0; i < server.nworkers; i++) {
        if (mq_send(lg, "#", sizeof(char), 0) == -1) {
            perror("graceful_exit() failed");
            exit(EXIT_FAILURE);
        }
    }
    // special command for logger
    if (mq_send(lg, "$", sizeof(char), 0) == -1) {
        perror("graceful_exit() failed");
        exit(EXIT_FAILURE);
    }
    // waiting success exit all child processes
    int status = 0;
    while (wait(&status) > 0) sleep(1);
    char* lgname = malloc(sizeof(*lgname) * 20);
    sprintf(lgname, "/process%d", lg);
    mq_close(lg);
    mq_unlink(lgname);
    free(server.workers);
    exit(EXIT_SUCCESS);
}

int init_server()
{
    server.lstfd = init_listen_socket();
    if (server.lstfd < 0)
        return FAILED_LSOCKET;

    server.logger = create_logger();
    if (server.logger == -1)
        return FAILED_LOGGER;

    server.workers = malloc(sizeof(*(server.workers)) * server.nworkers);
    int ncreated = create_processes(server.lstfd, server.workers, server.nworkers, server.logger);
    if (server.nworkers != ncreated)
        return FAILED_PROCESSES;

    return NO_FAILED;
}

// parse command line
void parse_cmd(int argc, char** argv)
{
    // TODO: port, address, num-processes, etc.
    server.nworkers = 2;
}

/////////////////////////////////////////////////////////////////////
// MAIN FUNCTION
int main(int argc, char** argv)
{
    parse_cmd(argc, argv);
    switch(init_server()) {
        // main loop
        case NO_FAILED: {
            signal(SIGINT, graceful_exit);
            while(1);
            break;
        }
        case FAILED_PROCESSES: {
            break;
        }
        case FAILED_LOGGER: {
            break;
        }
        case FAILED_LSOCKET: {
            break;
        }
        default:
            perror("init_server() failed");
            exit(EXIT_FAILURE);
            break;
    }
    return EXIT_SUCCESS;
}