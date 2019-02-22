#include "process.h"
#include "sockets.h"
#include "smtp.h"

#include <signal.h>
#include <sys/select.h>
#include <mqueue.h>

// process initialize
struct process_t* init_process(int pid, int log_pid, int* fd) 
{
    struct process_t* proc = (struct process_t*) malloc(sizeof *proc);
    proc->pid = pid;
    proc->worked = true;
    proc->l_fd = *fd;
    proc->max_fd = (fd == NULL) ? 0 : *fd;
    // lists of sockets
    proc->ws_list = NULL;
    proc->rs_list = NULL;
    // log init
    proc->lg = log_pid;
    // message queue for logger
    if (pid == log_pid) {
        char name[BUFFER_SIZE];
        sprintf(name, "/process%d", getpid());
        struct mq_attr attr;
        attr.mq_flags = attr.mq_curmsgs = 0;
        attr.mq_maxmsg = 10;
        attr.mq_msgsize = BUFFER_SIZE;
        proc->mq = malloc(sizeof *proc->mq);
        *(proc->mq) = mq_open(name, O_CREAT | O_RDONLY | O_NONBLOCK, 0644, &attr);    // 0644: write, read, read
        if (*(proc->mq) == -1) {
            perror("mq_open() failed");
            free(proc->mq);
        } else if (*(proc->mq) > proc->max_fd)  // check fd message queue
            proc->max_fd = *(proc->mq);
    }
    return proc;
}

bool check_listen_socket(int fd, struct cs_node_t** head, fd_set* readfds, int* max_fd)
{
    if (FD_ISSET(fd, readfds)) {
        struct cs_data_t* data = accept_client_socket(fd);
        if (data) {
            *max_fd = data->fd > *max_fd ? data->fd : *max_fd;   // check max fd
            struct cs_node_t* node = malloc(sizeof(*node)); // insert at head ws_list
            node->cs = data;
            node->next = *head;
            *head = node;
            return true;
        }
    }
    return false;
}

// parser select()
void parse_select(struct process_t* proc)
{
	struct timeval tv;	// timeout for select
	tv.tv_sec = SELECT_TIMEOUT;
	tv.tv_usec = 0;
	// call select: can change timeout
	int ndesc = select(proc->max_fd + 1, &(proc->readfds), &(proc->writefds), NULL, &tv);
	switch(ndesc) {
		// error
		case -1:
			perror("select() failed");
			break;
		// no events - close sockets
		case 0:
			printf("Server(%d): Timeout\n", getpid());
			for (struct cs_node_t* i = proc->ws_list; i != NULL; i = i->next)
				if (!FD_ISSET(i->cs->fd, &(proc->readfds)))
					i->cs->state = SOCKET_STATE_CLOSED;
			break;
		// sockets ready - need checks
		default:
            check_listen_socket(proc->l_fd, &proc->ws_list, &proc->readfds, &proc->max_fd);
            // handle sockets from ws_list
            for (struct cs_node_t* tmp = proc->ws_list; tmp != NULL; tmp = tmp->next) {
                if (FD_ISSET(tmp->cs->fd, &proc->readfds)) {
                    tmp->cs->flw = false;
                    main_handle(tmp->cs, proc->lg);
                }
                if (FD_ISSET(tmp->cs->fd, &proc->writefds)) {
                    tmp->cs->flw = true;
                    main_handle(tmp->cs, proc->lg);
                }
            }
            break;
	}
}

// runner
void run_process(struct process_t* proc)
{
    while(proc->worked) {
        // clear fd
        FD_ZERO(&(proc->readfds));
        FD_ZERO(&(proc->writefds));
        FD_SET(proc->l_fd, &proc->readfds);

        // delete all closed sockets
        close_client_sockets_by_state(&(proc->ws_list), SOCKET_STATE_CLOSED);

        // log
        char bf[BUFFER_SIZE];
        sprintf(bf, "writters = %p is_null = %d", proc->ws_list, (proc->ws_list == NULL));
        //mq_log(proc->lg, bf);
        sprintf(bf, "readers = %p is_null = %d", proc->rs_list, (proc->rs_list == NULL));
        //mq_log(proc->lg, bf);

        // sets all sockets in array
        for (struct cs_node_t* i = proc->rs_list; i != NULL; i = i->next)
            FD_SET(i->cs->fd, &(proc->readfds));
        for (struct cs_node_t* i = proc->ws_list; i != NULL; i = i->next) {
            FD_SET(i->cs->fd, &(proc->readfds));
            FD_SET(i->cs->fd, &(proc->writefds));
        }
        // call select() and proc->worked change inside
        parse_select(proc);
    }
}

// cleanup
void free_process(struct process_t* proc)
{
    close_client_sockets_by_state(&proc->rs_list, SOCKET_NOSTATE);
    close_client_sockets_by_state(&proc->ws_list, SOCKET_NOSTATE);
    
    if (proc->mq != NULL) {
        char name[BUFFER_SIZE];
        sprintf(name, "/process%d", *(proc->mq));
        mq_close(*(proc->mq));
        mq_unlink(name);
        free(proc->mq);
    }
    free(proc);
    printf("Server(%d): process killed\n", getpid());
    kill(getpid(), SIGTERM);
}

void body_process(int* fd_socket, int log_pid)
{
    struct process_t* proc = init_process(getpid(), log_pid, fd_socket);
    // open mq logger
    char name[BUFFER_SIZE];
    sprintf(name, "/process%d", proc->lg);
    mqd_t lg = mq_open(name, O_WRONLY); 
    if (lg > 0) proc->lg = lg;
    // write in log
    char bf[BUFFER_SIZE];
    sprintf(bf, "child process forked with pid(%d), ppid(%d)", getpid(), getppid());
    //mq_log(proc->lg, bf);
    
    run_process(proc);
    free_process(proc);
}

// create and run process
// returns pid
int create_process(int* listen_fd, int log_pid, void (*body)(int*, int))
{
    int pid = fork();
    switch (pid) {
        // process not created
        case -1: 
            printf("Server(%d): fork() failed\n", getpid()); 
            break;
        // process-child
        case 0: 
            (*body)(listen_fd, log_pid);
            break;
        // process-parent
        default:
            printf("Server(%d): create proccess(%d)\n", getpid(), pid);
            break;
    }
    return pid;
}

// create processes
// returns pid array
int* create_processes(int fd, int lg, int* nproc)
{
    int* pids = malloc(sizeof(*pids) * *nproc);
    for (int i = 0; i < *nproc; i++)
        *(pids + i) = create_process(&fd, lg, body_process);
    return pids;
}