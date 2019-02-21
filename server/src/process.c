#include "process.h"
#include "sockets.h"

#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

// process initialize
struct process_t* init_process(pid_t pid, pid_t log_pid, struct ss_node_t* ss) 
{
    struct process_t* proc = (struct process_t*) malloc(sizeof *proc);
    proc->pid = pid;
    proc->worked = true;
    proc->max_fd = -1;
    char tmp[50];

    // log init
    sprintf(tmp, "/process%d", log_pid);
    proc->lg = log_pid;
    proc->lg_name = tmp;

    // mqueue initialize
    sprintf(tmp, "/process%d", getpid());
    proc->mq_name = tmp;

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

        // delete all closed sockets
        proc->ss_list = delete_sockets_by_state(proc->ss_list, SOCKET_STATE_CLOSED);

        // log
        char bf[BUFFER_SIZE];
        sprintf(bf, "ls_list = %p is_null = %d", proc->ls_list, (proc->ls_list == NULL));
        mq_log(proc->lg, bf);
        sprintf(bf, "ss_list = %p is_null = %d", proc->ss_list, (proc->ss_list == NULL));
        mq_log(proc->lg, bf);

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
    delete_client_sockets(&proc->ls_list);
    delete_client_sockets(&proc->ss_list);
    mq_close(*(proc->mq));
    mq_unlink(proc->mq_name);
    free(proc->mq);

    free(proc);
    printf("Server(%d): process killed\n", getpid());
    kill(getpid(), SIGTERM);
}

// process creating
// return process id or -1
pid_t create_process(struct ss_node_t* fd_socket, pid_t log_pid)
{
    pid_t pid = fork();
    switch (pid) {
        // process not created
        case -1: 
            printf("Server(%d): fork() failed\n", getpid()); 
            break;
        // process-child
        case 0: {
            struct process_t* proc = init_process(getpid(), log_pid, fd_socket);
            // open mq logger
            mqd_t lg = mq_open(proc->lg_name, O_WRONLY); 
            if (lg > 0)
                proc->lg = lg;
            // write in log
            char bf[BUFFER_SIZE];
            sprintf(bf, "child process forked with pid(%d)", getpid());
            mq_log(proc->lg, bf);
            sprintf(bf, "his parent pid(%d)", getppid());
            mq_log(proc->lg, bf);
            
            run_process(proc);
            free_process(proc);
            break;
        }
        // process-parent
        default:
            printf("Server(%d): create proccess(%d)\n", getpid(), pid);
            break;
    }
    return pid;
}