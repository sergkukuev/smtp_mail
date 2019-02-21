#include "logger.h"
#include "process.h"

#include <sys/types.h>
#include <sys/select.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

int save_to_file(char* txt)
{
    FILE* lf = fopen("server_log", "a");
    if (!lf) {
        perror("error opening log file(server_log)");
        return -1;
    }
    time_t ct = time(NULL);
    char* t = ctime(&ct);
    t[strlen(t) - 1] = '\0';
    fprintf(lf, "[%s]: %s", t, txt);
    fflush(lf);
    fclose(lf);
    return 0;
}

void run_logger(struct process_t* pr) 
{
	while (pr->worked) {
		struct timeval tv;
		tv.tv_sec = 15;
		tv.tv_usec = 0;

		FD_ZERO(&(pr->readfds));
		if (pr->mq != NULL)
			FD_SET(*(pr->mq), &(pr->readfds));

		switch(select(pr->max_fd + 1, &(pr->readfds), NULL, NULL, &tv)) {
        case 0:
            printf("Logger(%d): Timeout\n", getpid());
            break;
        default:
            if (pr->mq != NULL && FD_ISSET(*(pr->mq), &(pr->readfds))) {
                char msg[BUFFER_SIZE];
                memset(msg, 0x00, sizeof(msg));
                if (mq_receive(*(pr->mq), msg, BUFFER_SIZE, NULL) >= 0) {
                    printf("Logger(%d): Received message <%s>\n", getpid(), msg);
                    save_to_file(msg);
                    if (strcmp(msg, "#") == 0) {
                        pr->worked = false;
                        continue;
                    }
                } else {
                    printf("Logger(%d): None\n", getpid());
                }
            }
            break;
        }
    }
}

pid_t create_logger() 
{
	pid_t pid = fork();
    switch (pid) {
    case -1:
        printf("Server(%d): fork() failed", getpid()); 
        break;
    // child
    case 0: {
        struct process_t* pr = create_process(NULL, getpid()); 
        run_logger(pr);
        kill(getpid(), SIGTERM);
        break;
    }
    // parent
    default:
        printf("Server(%d): create proccess(%d) for logger", getpid(), pid);
        break;
    }
    return pid;
}