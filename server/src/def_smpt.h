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
#define RSMTP_220 "220 Служба готова к работе. \r\n"
#define RSMTP_221 "221 Служба закрывает канал передачи данных. \r\n"
#define RSMTP_250 "250 Выполнение почтовой команды успешно окончено. \r\n"
#define RSMTP_252 "252 Невозможно проверить наличие почтового ящика для пользователя, но сообщение принято, и сервер попытается его доставить. \r\n"
#define RSMTP_354 "354 Начало приема сообщения. Сообщение должно заканчиваться точкой на новой строке и новой строкой. \r\n"
#define RSMTP_450 "450 Запрошенная команда не принята – недоступен почтовый ящик (почтовый ящик временно занят). \r\n"
#define RSMTP_500 "500 Синтаксическая ошибка, команда не распознана. \r\n"

#endif // !__DEF_SMPT_H__