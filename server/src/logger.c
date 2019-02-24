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
		FD_SET(pr->fd.logger, &(pr->readfds));
		switch(select(pr->fd.max + 1, &(pr->readfds), NULL, NULL, &tv)) {
            case 0:
                printf("Logger(%d): Timeout\n", getpid());
                break;
            default:
                if (FD_ISSET(pr->fd.logger, &(pr->readfds))) {
                    char msg[BUFFER_SIZE];
                    memset(msg, 0x00, sizeof(msg)); // clear buffer
                    if (mq_receive(pr->fd.logger, msg, BUFFER_SIZE, NULL) >= 0) {
                        if ((strcmp(msg, "#") != 0) && (strcmp(msg, "$") != 0)) {   // ignore command
                            printf("Logger(%d): received message from %s\n", getpid(), msg);
                            LOG(msg);
                        }
                        if (strcmp(msg, "$") == 0) {
                            printf("Logger(%d): accept command on close\n", getpid());
                            LOG("close logger");
                            pr->worked = false;
                        }
                    }
                    /* else {
                        printf("Logger(%d): None\n", getpid());
                    } */
                }
            break;
        }
    }
}

void body_logger(int* fd, pid_t* pid)
{
    *pid = getpid();
    struct process_t* pr = init_process(fd, *pid);
    if (pr != NULL) {
        char msg[BUFFER_SIZE];
        sprintf(msg, "new log session(%d)", *pid);
        LOG(msg);
        // run
        printf("Logger(%d): started work\n", *pid);
        run_logger(pr);
        free_process(pr);
    } else 
        printf("Logger(%d): failed init\n", *pid);

    printf("Logger(%d): process killed\n", *pid);
    kill(getpid(), SIGTERM);
}

pid_t create_logger()
{
    pid_t lg = -1;
    return create_process(NULL, &lg, body_logger);
}