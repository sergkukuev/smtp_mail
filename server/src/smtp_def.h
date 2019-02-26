#ifndef __SMTP_DEF_H__
#define __SMTP_DEF_H__

// smtp keys code
#define KEY_FAILED -1
#define KEY_HELO 0
#define KEY_EHLO 1
#define KEY_MAIL 2
#define KEY_RCPT 3
#define KEY_DATA 4
#define KEY_NOOP 5
#define KEY_RSET 6
#define KEY_QUIT 7
#define KEY_VRFY 8

// smtp keys string
#define STR_HELO "HELO"
#define STR_EHLO "EHLO"
#define STR_MAIL "MAIL"
#define STR_RCPT "RCPT"
#define STR_DATA "DATA"
#define STR_NOOP "NOOP"
#define STR_RSET "RSET"
#define STR_QUIT "QUIT"
#define STR_VRFY "VRFY"

// replies smtp server
// 2xx
#define REPLY_HELLO "220 SMTP bmstu myserver.ru ready\r\n"
#define REPLY_EHLO "250-VRFY\r\n250-NOOP\r\n250-RSET\r\n"
#define REPLY_QUIT "221 close connection\r\n"
#define REPLY_OK "250 OK\r\n"
#define REPLY_ADMIN "252 Administrative prohibition\r\n"
// 3xx
#define REPLY_DATA "354 Enter message, ending with \".\" on a line by itself\r\n"
// 4xx
#define REPLY_TERMINATE "421 closing server\r\n"
#define REPLY_UN_MAIL "450 mailbox unavailable\r\n"
#define REPLY_MUCH_REC "451 too much recepients\r\n"
#define REPLY_MEM "452 memory is filled\r\n"
// 5xx
#define REPLY_UNREC "500 Unrecognized command\r\n"
#define REPLY_ARGS "501 invalid argument(s)\r\n"
#define REPLY_SEQ "503 Wrong command sequence\r\n"

#endif // !__SMTP_DEF_H__
