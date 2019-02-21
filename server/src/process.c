#include "process.h"
#include "sockets.h"

#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

// process initialize
struct process_t* init_process(pid_t pid, struct ss_node_t* ss) 
{
    struct process_t* proc = (struct process_t*) malloc(sizeof *proc);
    proc->pid = pid;
    proc->worked = true;
    proc->max_fd = -1;

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
    } else if (*(proc->mq) > proc->max_fd)  // check fd message queue
        proc->max_fd = *(proc->mq);

    // lists of sockets
    proc->ss_list = NULL;
    proc->ls_list = init_client_sockets(ss, &proc->max_fd);

    return proc;
}

// runner
void run_process(struct process_t* proc)
{
    while(proc->worked) {
        // clear fd
        FD_ZERO(&(proc->readfds));
        FD_ZERO(&(proc->writefds));

        // set all sockets to ss_list
        for (struct cs_node_t* i = proc->ls_list; i != NULL; i = i->next)
            FD_SET(i->cs.fd, &(proc->readfds));
        // set exists client socket
        for (struct cs_node_t* i = proc->ss_list; i != NULL; i = i->next) {
            FD_SET(i->cs.fd, &(proc->readfds));
            FD_SET(i->cs.fd, &(proc->writefds));
        }
        // set message queue
        if (proc->mq != NULL)
            FD_SET(*(proc->mq), &(proc->readfds));
        // call select() and proc->worked change inside
        parse_select(proc);
    }
}

// cleanup
void free_process(struct process_t* proc)
{
    // TODO: free sockets
    mq_close(*(proc->mq));
    mq_unlink(proc->mq_name);

    free(proc);
    kill(getpid(), SIGTERM);
}

void body_process(struct ss_node_t* fd_sock)
{
    struct process_t* proc = init_process(getpid(), fd_sock);
    run_process(proc);
    free_process(proc);
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
        case 0:
            body_process(fd_socket);
            break;
        // process-parent
        default:
            break;
    }
    return pid;
}