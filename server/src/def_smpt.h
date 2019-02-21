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

// replies smtp
#define RSMTP_HELLO "220 SMTP CCSMTP bmstu myserver.ru \r\n"
#define RSMTP_221 "221 closing connection \r\n"
#define RSMTP_250 "250 OK \r\n"
#define RSMTP_250_TEXT "250- "
#define RSMTP_250_RESET "250 Reset OK \r\n"
#define RSMTP_250_RCPT "250 Accepted \r\n"
#define RSMTP_354 "354 Enter message, ending with \".\" on a line by itself \r\n"
#define RSMTP_450 "450 mailbox unavailable \r\n"
#define RSMTP_451 "451 too much recepients \r\n"
#define RSMTP_500 "500 Unrecognized command \r\n"
#define RSMTP_500_FILLED "500 filled \r\n"
#define RSMTP_501 "501 invalid argument(s) \r\n"
#define RSMTP_503 "503 Wrong command sequence \r\n"

#endif // !__DEF_SMPT_H__