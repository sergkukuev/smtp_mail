#include "process.h"
#include "sockets.h"

#include <sys/types.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

// process initialize
struct process_t* init_process(pid_t pid, struct ss_node_t* ss) 
{
    struct process_t* proc = (struct process_t*) malloc(sizeof *proc);
    proc->pid = pid;
    // lists of sockets
    proc->ss_list = ss;
    proc->cs_list = init_client_sockets(ss);

    // mqueue initialize
    char qname[50];
    sprintf(qname, "/process%d", getpid());
    proc->mq_name = qname;

    struct mq_attr attr;
    attr.mq_flags = attr.mq_curmsgs = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = BUFFER_SIZE;

    proc->mq = malloc(sizeof *proc->mq);
    *(proc->mq) = mq_open(proc->mq_name, O_CREAT | O_RDONLY | O_NONBLOCK, 0644, &attr);    // 0644: write, read, read
    if (*(proc->mq) == -1) {
        perror("mq_open() failed");
        free(proc->mq);
    }

    return proc;
}

// cleanup
void free_process(struct process_t* proc)
{
    // TODO: free sockets
    mq_close(*(proc->mq));
    mq_unlink(proc->mq_name);

    free(proc);
}

// process creating
// return process id or -1
pid_t create_process(struct ss_node_t* fd_socket)
{
    pid_t pid = fork();
    switch (pid) {
        // process not created
        case -1: 
            perror("fork() failed");
            break;
        // process-child
        case 0: {
            struct process_t* proc = init_process(getpid(), fd_socket);
            // run_process(proc);
            free_process(proc);
        }
        // process-parent
        default:
            break;
    }
    return pid;
}