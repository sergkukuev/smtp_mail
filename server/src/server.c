#include "sockets.h"
#include "process.h"
#include "logger.h"

#include "common.h"
#include <sys/wait.h>
#include <signal.h>
#include <mqueue.h>

// failed initialization
#define NO_FAILED 0
#define FAILED_EXIT -1
#define FAILED_LSOCKET -2
#define FAILED_LOGGER -3
#define FAILED_PROCESSES -4

// server struct 
struct server_t {
    pid_t* workers;
    int nworkers;

    struct servfd_t {
        int listener;
        int logger;
        int exit;
    } fd;

    pid_t logger;
};

static struct server_t server;

bool init_exit_queue()
{
    char* name = malloc(sizeof(*name) * 20);
    sprintf(name, "/exit%d", getpid());
    struct mq_attr attr;
    attr.mq_flags = attr.mq_curmsgs = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = BUFFER_SIZE;
    server.fd.exit = mq_open(name, O_CREAT | O_RDWR | O_NONBLOCK, 0666, &attr);
    free(name);
    return server.fd.exit == -1 ? false : true;
}

void free_exit_queue()
{
    if (server.fd.exit != -1) {
        char* name = malloc(sizeof(*name) * 20);
        sprintf(name, "/exit%d", server.fd.logger);
        mq_close(server.fd.exit);
        mq_unlink(name);
        free(name);
    }
}

void gracefull_exit(int sig)
{
    signal(sig, SIG_IGN);
    printf("\nServer(%d): start gracefull close\n", getpid());
    mq_log(server.fd.logger, "#"); // send all for gracefull exit 
    for (int i = 0; i < server.nworkers; i++) {
        wait(&server.workers[i]);
    }
    wait(&server.logger);
}

int init_server()
{
    if (!init_exit_queue())
        return FAILED_EXIT;

    server.fd.listener = init_listen_socket();
    if (server.fd.listener < 0)
        return FAILED_LSOCKET;

    server.fd.logger = create_logger();
    if (server.fd.logger == -1)
        return FAILED_LOGGER;

    server.workers = create_processes(server.fd.listener, &server.nworkers, server.fd.logger);
    if (server.workers == NULL)
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
        case NO_FAILED: {
            signal(SIGINT, gracefull_exit);
            while(true) pause();
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
        case FAILED_EXIT:
            perror("init_server() failed");
            exit(EXIT_FAILURE);
            break;
        // no actions
        default:
            break;
    }
    return EXIT_SUCCESS;
}