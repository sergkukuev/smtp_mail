#include "common.h"


void get_address(struct sockaddr_in* addr, socklen_t* addrlen)
{
    memset(addr, 0, sizeof(*addr));
    addr->sin_family = AF_UNSPEC;
    addr->sin_addr.s_addr = INADDR_ANY;
    addr->sin_port = htons(atoi(SERVER_PORT));
    *addrlen = sizeof(addr);
}

char* parse_mail(char* buf)
{
    return buf;
}
			