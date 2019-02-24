#include "common.h"

#include <sys/stat.h>
#include <sys/time.h>
#include <mqueue.h>
#include <time.h>

bool save_to_file(char* fname, char* txt, bool info)
{
    FILE* lf = fopen(fname, "a");
    char msg[BUFFER_SIZE];
    if (!lf) {
        sprintf(msg, "error opening log file(%s)", fname);
        perror(msg);
        return false;
    }
    // variable add '\n' or not
    (txt[strlen(txt) - 1] == '\n') ? sprintf(msg, "%s", txt) : sprintf(msg, "%s\n", txt); 
    // write with timetag
    if (info) {
        time_t ct = time(NULL);
        char* t = ctime(&ct);
        t[strlen(t) - 1] = '\0';
        fprintf(lf, "[%s]: %s", t, msg);
    } else  // without timetag
        fprintf(lf, "%s", msg);
    fflush(lf);
    fclose(lf);
    return true;
}

int mq_log(int lg, char* msg)
{
    char bf[BUFFER_SIZE];
    sprintf(bf, "%d <%s>", getpid(), msg);
    int res = mq_send(lg, bf, strlen(bf), 0);
    if (res == -1)  perror(msg);
    return res;
}

char* select_from_message(char* message, char* buffer, char* start, char* end) {
    if ( start && end ) {
        start++;
        int length = end - start;
        buffer = (char*) malloc(length + 1);
        memcpy(buffer, start, length + 1);
        buffer[length] = '\0';
    }
    return buffer;
}

void get_address(struct sockaddr_in* addr, socklen_t* addrlen)
{
    memset(addr, 0, sizeof(*addr));
    addr->sin_family = AF_UNSPEC;
    addr->sin_addr.s_addr = INADDR_ANY;
    addr->sin_port = htons(atoi(SERVER_PORT));
    *addrlen = sizeof(addr);
}

char* parse_mail(char* message)
{
    char *result = NULL;
	char *start = strchr(message, MAILSTART);
	if (!start)
		return NULL;
	char *end = strchr(start + 1, MAILEND);
	if (!end)
		return NULL;
	return select_from_message(message, result, start, end);
}

void generate_filename(char *seq) {
	struct timeval tv;
    struct timezone tz;
    
    gettimeofday(&tv,&tz);
    srand(tv.tv_usec);
    sprintf (seq,"%lx.%lx.%x",tv.tv_sec, tv.tv_usec, rand());
}

char* concat_strings(char *s1, char *s2) {
    int len1 = strlen(s1);
    int len2 = strlen(s2);
    char *result = malloc(len1 + len2 + 1);
    memcpy(result, s1, len1);
    memcpy(result+len1, s2, len2);
    result[len1+len2] = '\0';
    return result;
}

int make_dir(char* dir_path) {
    mkdir(dir_path, 0700);
    char* new_dir = concat_strings(dir_path,"/new");
    mkdir(new_dir, 0700);
    free(new_dir);
    new_dir = concat_strings(dir_path,"/cur");
    mkdir(new_dir, 0700);
    free(new_dir);
    new_dir = concat_strings(dir_path,"/tmp");
    mkdir(new_dir, 0700);
    free(new_dir);
    return 1;
}

char* make_dir_path(char *path, char *address_to) {
	char *tmp_maildir = concat_strings(path, address_to);
	struct stat file_stat;
	if (stat(tmp_maildir, &file_stat) == -1)
        mkdir(tmp_maildir, 0700);
    char* dir_path = concat_strings(tmp_maildir,"/maildir");
    if (stat(dir_path, &file_stat) == -1)
        make_dir(dir_path);
    free(tmp_maildir);
    return dir_path;
}

char* make_tmp_path(char* dir_path, char* name) {
    char* tmp_dir = concat_strings(dir_path,"/tmp/");
    char* tmp_file = concat_strings(tmp_dir, name);
    free(tmp_dir);
    return tmp_file;
}

char* make_new_path(char* dir_path, char* name) {
    char* new_dir = concat_strings(dir_path,"/new/");
    char* new_file = concat_strings(new_dir, name);
    free(new_dir);
    return new_file;
}   

int save_message(struct msg_t *message) {
	if ((strcmp(message->from, "") == 0) || (strcmp(message->to[0], "") == 0) || (strcmp(message->body, "") == 0))
		return PARSE_FAILED;
	int i;
	for (i = 0; i < message->rnum; i++) {
		char *dir_path = make_dir_path(MAILDIR, message->to[i]);
		char filename[20];
		generate_filename(filename);
		char *tmp_path = make_tmp_path(dir_path, filename);
		char *new_path = make_new_path(dir_path, filename);
		FILE *fp = fopen(tmp_path,"a");
        fprintf(fp,"From: %s\r\n",message->from);
        fprintf(fp,"To: %s\r\n",message->to[i]);
        fprintf(fp,"%s\n",message->body);
        fclose(fp);
        rename(tmp_path,new_path);
        free(tmp_path);
        free(new_path);
        free(dir_path);
	}
	return 0;
}