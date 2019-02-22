#include "logger.h"
#include "process.h"

#include <sys/select.h>
#include <mqueue.h>
#include <signal.h>

#define LOG(msg) save_to_file("server_log", msg, true)

void run_logger(struct process_t* pr) 
{
	while (pr->worked) {
		struct timeval tv;
		tv.tv_sec = SELECT_TIMEOUT;
		tv.tv_usec = 0;

		FD_ZERO(&(pr->readfds));
		if (pr->lg != -1)   FD_SET(pr->lg, &(pr->readfds));
		switch(select(pr->max_fd + 1, &(pr->readfds), NULL, NULL, &tv)) {
            case 0:
                printf("Logger(%d): Timeout\n", getpid());
                break;
            default:
                if (pr->lg != -1 && FD_ISSET(pr->lg, &(pr->readfds))) {
                    char msg[BUFFER_SIZE];
                    memset(msg, 0x00, sizeof(msg));
                    if (mq_receive(pr->lg, msg, BUFFER_SIZE, NULL) >= 0) {
                        printf("Logger(%d): received message <%s>\n", getpid(), msg);
                        LOG(msg);
                    } else {
                        printf("Logger(%d): None\n", getpid());
                    }
                }
            break;
        }
    }
}

void body_logger(int* fd, int pid)
{
    char msg[BUFFER_SIZE];
    sprintf(msg, "new log session(%d)", getpid());
    LOG(msg);
    struct process_t* pr = init_process(pid, getpid(), fd);
    run_logger(pr);
    free_process(pr);
}

pid_t create_logger()
{
    return create_process(NULL, getpid(), body_logger);
}