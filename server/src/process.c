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
    // list and sets of sockets
    proc->s_list = NULL;
    proc->readfds = malloc(sizeof(*proc->readfds));
    proc->writefds = malloc(sizeof(*proc->writefds));
    char lgname[20];
    sprintf(lgname, "/process%d", log_pid);
    // logger message queue init
    if (getpid() == log_pid) {   // logger
        struct mq_attr attr;
        attr.mq_flags = attr.mq_curmsgs = 0;
        attr.mq_maxmsg = 10;
        attr.mq_msgsize = BUFFER_SIZE;
        proc->fd.logger = mq_open(lgname, O_CREAT | O_RDONLY | O_NONBLOCK, 0644, &attr);    // 0644: write, read, read  
        memset(lgname, 0x0, sizeof(lgname));
        sprintf(lgname, "/exit%d", log_pid);
        proc->fd.cmd = mq_open(lgname, O_CREAT | O_RDONLY | O_NONBLOCK, 0644, &attr);
    } else {    // workers
        sleep(1);
        proc->fd.logger = mq_open(lgname, O_WRONLY);  // for send message to logger
        memset(lgname, 0x0, sizeof(lgname));
        sprintf(lgname, "/exit%d", log_pid);
        proc->fd.cmd = mq_open(lgname, O_RDONLY);   // for receive command
    }
    if (proc->fd.logger == -1 || proc->fd.cmd == -1) {
        perror("mq_open() failed");
        free(proc);
        proc = NULL;
    } else if (proc->fd.logger > proc->fd.max || proc->fd.cmd > proc->fd.max) { // check fd message queue
        proc->fd.max = (proc->fd.logger > proc->fd.cmd) ? proc->fd.logger : proc->fd.cmd;
        (proc->pid == log_pid) ? printf("Logger(%d): queue created\n", log_pid) : printf("Worker(%d): linked log queue\n", getpid());
    }
    return proc;
}

// cleanup
void free_process(struct process_t* proc)
{
    // free sockets
    close_client_sockets_by_state(&proc->s_list, SOCKET_NOSTATE);
    free(proc->readfds);
    free(proc->writefds);
    
    // free message queue
    if (proc->fd.logger != -1)
        mq_close(proc->fd.logger);
    if (proc->fd.cmd != -1)
        mq_close(proc->fd.cmd);

    if (proc->lgpid == proc->pid) {
        char* lgname = malloc(sizeof(*lgname) * 20);
        sprintf(lgname, "/process%d", proc->lgpid);
        mq_unlink(lgname);
        memset(lgname, 0x0, strlen(lgname));
        sprintf(lgname, "/exit%d", proc->lgpid);
        mq_unlink(lgname);
    }
    free(proc);
}

bool is_exit(int fd, fd_set* readfds) 
{
    bool result = false;
    if (FD_ISSET(fd, readfds)) {
        char msg[BUFFER_SIZE];
        memset(msg, 0x00, sizeof(msg)); // clear buffer
        if (mq_receive(fd, msg, BUFFER_SIZE, NULL) >= 0) {
            if (strcmp(msg, "#") == 0) {
                printf("Worker(%d): accept command on close\n", getpid());
                mq_log(fd, "close worker");
                result = true;
            }
            // else 
            // printf("Worker(%d): received message <%s>\n", getpid(), msg);
        }
    }
    return result;
}

bool is_came_on_lsocket(int fd, int* max_fd, fd_set* readfds, struct cs_node_t** head)
{
    bool result = false;
    if (FD_ISSET(fd, readfds)) {
        struct cs_data_t* data = accept_client_socket(fd);
        if (data) {
            *max_fd = data->fd > *max_fd ? data->fd : *max_fd;   // check max fd            
            // add socket at head of list
            struct cs_node_t* node = malloc(sizeof(*node)); // insert at head ws_list
            node->cs = data;
            node->next = *head;
            *head = node;
            result = true;
        } 
        // else 
        // sprintf(msg, "socket data not binded");
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
	int ndesc = select(proc->fd.max + 1, proc->readfds, proc->writefds, NULL, &tv);
	switch(ndesc) {
		// error
		case -1:
			perror("select() failed");
			break;
		// no events - close not set sockets
		case 0:
			printf("Worker(%d): select timeout\n", getpid());
			for (struct cs_node_t* i = proc->s_list; i != NULL; i = i->next)
				if (!FD_ISSET(i->cs->fd, proc->readfds))
					i->cs->state = SOCKET_STATE_CLOSED;
			break;
		// sockets ready - need checks
		default: {
            printf("Worker(%d): start parse select\n", getpid());
            if (!(proc->worked = !is_exit(proc->fd.cmd, proc->readfds)))
                return; // exit of worker
            // handle listen socket
            if (is_came_on_lsocket(proc->fd.listen, &proc->fd.max, proc->readfds, &proc->s_list)) {
                char msg[BUFFER_SIZE];
                sprintf(msg, "socket data binded(fd = %d)", proc->s_list->cs->fd);
                printf("Worker(%d): %s\n", getpid(), msg);
                mq_log(proc->fd.logger, msg);
            }
            // handle other sockets
            for (struct cs_node_t* tmp = proc->s_list; tmp != NULL; tmp = tmp->next)
                if (FD_ISSET(tmp->cs->fd, proc->readfds) || FD_ISSET(tmp->cs->fd, proc->writefds))
                    main_handle(tmp->cs, proc->fd.logger);
            break;
        }
	}
}

// runner
void run_process(struct process_t* proc)
{
    while(proc->worked) {
        // clear fd
        FD_ZERO(proc->readfds);
        FD_ZERO(proc->writefds);
        // set listener socket and logger 
        FD_SET(proc->fd.listen, proc->readfds);
        FD_SET(proc->fd.cmd, proc->readfds);

        // delete all closed sockets
        close_client_sockets_by_state(&(proc->s_list), SOCKET_STATE_CLOSED);

        // log
        char bf[BUFFER_SIZE];
        memset(bf, 0x00, strlen(bf));
        sprintf(bf, "socket list pointer = %p is_null = %d", proc->s_list, (proc->s_list == NULL));
        mq_log(proc->fd.logger, bf);

        // sets all sockets in array
        for (struct cs_node_t* i = proc->s_list; i != NULL; i = i->next)
            i->cs->flw ? FD_SET(i->cs->fd, proc->writefds) : FD_SET(i->cs->fd, proc->readfds);
        // call select() and proc->worked change inside
        parse_select(proc);
    }
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
        printf("Worker(%d): started work\n", getpid());
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
