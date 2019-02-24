#include "process.h"
#include "sockets.h"
#include "smtp.h"

#include <signal.h>
#include <sys/select.h>
#include <mqueue.h>

// process initialize
struct process_t* init_process(int* fd, pid_t log_pid) 
{
    struct process_t* proc = (struct process_t*) malloc(sizeof *proc);
    proc->pid = getpid();
    proc->lgpid = log_pid;
    proc->worked = true;
    proc->fd.listen = (fd == NULL) ? -1 : *fd;
    proc->fd.max = (fd == NULL) ? 0 : *fd;
    // lists of sockets
    proc->ws_list = NULL;
    proc->rs_list = NULL;
    char lgname[20];
    sprintf(lgname, "/process%d", log_pid);
    // logger message queue init
    if (getpid() == log_pid) {   // logger
        struct mq_attr attr;
        attr.mq_flags = attr.mq_curmsgs = 0;
        attr.mq_maxmsg = 10;
        attr.mq_msgsize = BUFFER_SIZE;
        proc->fd.logger = mq_open(lgname, O_CREAT | O_RDONLY | O_NONBLOCK, 0644, &attr);    // 0644: write, read, read   
    } else {    // workers
        //sleep(1);
        proc->fd.logger = mq_open(lgname, O_RDWR);  // for receive command and send msg to logger
    }
    if (proc->fd.logger == -1) {
        perror("mq_open() failed");
        free(proc);
        proc = NULL;
    } else if (proc->fd.logger > proc->fd.max) { // check fd message queue
        proc->fd.max = proc->fd.logger;
        (proc->pid == log_pid) ? printf("Logger(%d): queue created\n", log_pid) : printf("Worker(%d): linked log queue\n", getpid());
    }
    return proc;
}

bool is_exit(int fd, fd_set* readfds) 
{
    bool result = false;
    if (FD_ISSET(fd, readfds)) {
        char msg[BUFFER_SIZE];
        memset(msg, 0x00, sizeof(msg)); // clear buffer
        if (mq_receive(fd, msg, BUFFER_SIZE, NULL) >= 0) {
            if (strcmp(msg, "#") == 0) {
                printf("Worker(%d): received command <%s>\n", getpid(), msg);
                mq_log(fd, "close worker");
                result = true;
            } else 
                printf("Worker(%d): received message <%s>\n", getpid(), msg);
        }
    }
    return result;
}

bool check_listen_socket(int fd, struct cs_node_t** head, fd_set* readfds, int* max_fd)
{
    bool result = false;
    if (FD_ISSET(fd, readfds)) {
        struct cs_data_t* data = accept_client_socket(fd);
        if (data) {
            *max_fd = data->fd > *max_fd ? data->fd : *max_fd;   // check max fd
            struct cs_node_t* node = malloc(sizeof(*node)); // insert at head ws_list
            node->cs = data;
            node->next = *head;
            *head = node;
            result = true;
        } else {
            char msg[BUFFER_SIZE];
            sprintf(msg, "Worker(%d): socket data not binded\n", getpid());
        }
    }
    return result;
}

// parser select()
void parse_select(struct process_t* proc)
{
	struct timeval tv;	// timeout for select
	tv.tv_sec = SELECT_TIMEOUT;
	tv.tv_usec = 0;
	// call select: can change timeout
	int ndesc = select(proc->fd.max + 1, &(proc->readfds), &(proc->writefds), NULL, &tv);
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
            if (!(proc->worked = !is_exit(proc->fd.logger, &(proc->readfds))))
                return; // exit of worker
            check_listen_socket(proc->fd.listen, &proc->ws_list, &proc->readfds, &proc->fd.max);
            // handle sockets from ws_list
            for (struct cs_node_t* tmp = proc->ws_list; tmp != NULL; tmp = tmp->next) {
                if (FD_ISSET(tmp->cs->fd, &proc->readfds)) {
                    tmp->cs->flw = false;
                    main_handle(tmp->cs, proc->fd.logger);
                }
                if (FD_ISSET(tmp->cs->fd, &proc->writefds)) {
                    tmp->cs->flw = true;
                    main_handle(tmp->cs, proc->fd.logger);
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
        FD_SET(proc->fd.listen, &proc->readfds); // set listen socket
        FD_SET(proc->fd.logger, &proc->readfds); // set logger socket for exit mode 

        // delete all closed sockets
        close_client_sockets_by_state(&(proc->ws_list), SOCKET_STATE_CLOSED);

        // log
        /*char bf[BUFFER_SIZE];
        memset(bf, 0x00, strlen(bf));
        sprintf(bf, "writters = %p is_null = %d", proc->ws_list, (proc->ws_list == NULL));
        mq_log(proc->fd.logger, bf);
        memset(bf, 0x00, strlen(bf));
        sprintf(bf, "readers = %p is_null = %d", proc->rs_list, (proc->rs_list == NULL));
        mq_log(proc->fd.logger, bf);*/

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
    // free sockets
    close_client_sockets_by_state(&proc->rs_list, SOCKET_NOSTATE);
    close_client_sockets_by_state(&proc->ws_list, SOCKET_NOSTATE);
    
    // free message queue
    if (proc->fd.logger != -1)
        mq_close(proc->fd.logger);

    if (proc->lgpid == proc->pid) {
        char* lgname = malloc(sizeof(*lgname) * 20);
        sprintf(lgname, "/process%d", proc->lgpid);
        mq_unlink(lgname);
    }

    free(proc);
}

void body_process(int* fd_socket, pid_t* log_pid)
{
    struct process_t* proc = init_process(fd_socket, *log_pid);
    // success init
    if (proc != NULL) {
        // write in log
        char bf[BUFFER_SIZE];
        sprintf(bf, "child process forked with pid(%d), ppid(%d)", getpid(), getppid());
        mq_log(proc->fd.logger, bf);
        // run
        printf("Worker(%d): Started work\n", getpid());
        run_process(proc);
        free_process(proc);
    } else 
        printf("Worker(%d): failed init\n", getpid());

    printf("Worker(%d): process killed\n", getpid());
    kill(getpid(), SIGTERM);
}

// create and run process
// returns pid
pid_t create_process(int* listen_fd, pid_t* log_pid, void (*body)(int*, pid_t*))
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
int create_processes(int fd, pid_t* procs, int nproc, pid_t log_pid)
{
    int num = 0;
    for (int i = 0; i < nproc; i++) {
        pid_t tmp = create_process(&fd, &log_pid, body_process);
        // save in array
        if (tmp != -1) {
            num++;
            procs[i] = tmp;
        }
    }
    return num;
}
