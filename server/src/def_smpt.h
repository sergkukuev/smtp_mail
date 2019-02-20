#ifndef __DEF_SMTP_H__
#define __DEF_SMTP_H__

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

// smtp keys string
#define STR_HELO "HELO"
#define STR_EHLO "EHLO"
#define STR_MAIL "MAIL"
#define STR_RCPT "RCPT"
#define STR_DATA "DATA"
#define STR_NOOP "NOOP"
#define STR_RSET "RSET"
#define STR_QUIT "QUIT"

// answers
#define SMTP_220 "220 \r\n"
#define SMTP_221 "221 Ok\r\n"
#define SMTP_250 "250 Ok\r\n"
#define SMTP_252 "252 Ok\r\n"
#define SMTP_354 "354 Ok\r\n"
#define SMTP_450 "450 Error\r\n"
#define ANS_500 "500 Error\r\n"

#endif // !__DEF_SMPT_H__