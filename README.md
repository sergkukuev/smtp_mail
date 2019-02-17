# Почтовый сервер (SMTP)
Реализация почтового сервера (SMTP-клиент и SMTP-сервер) для курса НИРС "Проектирование вычислительных сетей" 2 курса магистратуры МГТУ им. Баумана.

## Стек технологий
- яп: Си стандарта С99;
- сборка: GNU make.

## Поставленные задачи
- Сервер должен подерживать команды HELO и EHLO, MAIL, RCPT, DATA, RSET, QUIT, VERIFY протокола SMTP;
- Серверу следует реализовать только указанные команды. VERIFY должен при этом выдавать всегда ошибку;
- Отправитель должен поддерживать набор команд, достаточный для отправки почты как минимум одной крупной почтовой публичной службе с веб-интерфейсом;
 - В состав сервера и клиента должен входить Makefile, имеющий цели для следующих задач: сборки системы (default), выполнения тестирования (проверка утечек памяти, стиля, модульное тестирование, тестирование протокола) (test_units, test_memory, test_system, test_style, tests), создание pdf с РПЗ (report);
- Для разбора параметров командной строки рекомендуется (но не обязательно) использовать autoopts. Файл конфигурации программы не обязателен, но возможен (используйте libconfig). Минимальный список поддерживаемых параметров командной строки / файла конфигурации — ниже.
  - Для программы получения почты: порт, корневой каталог для почты (например, /var/mail, или /home/student/test_mail), корневой каталог для очередей сообщений, пользователь и группа для понижения привелегий, имя файла журнала (лог), сеть (сети), для который разрешен релей почты. сетевой адрес привязки, максимальное число рабочих потоков / процессов (см. задание).
  - Для программы передачи почты: корневой каталог для очередей сообщений, имя файла журнала (лог), общее время на попытки отправить письмо, минимальное время между попытками повтора, максимальное число рабочих потоков / процессов (см. задание).
- Система должна реализовать журналирование. Допустимо выводить сообщения в stdout, ошибки --- в stderr. Журналирование следует делать как указанно в варианте, для связи (если она требуется) нужно использовать Posix MQ или SysV MQ (последнее добавлено по просьбе маководов, где как минимум год назад не было Posix MQ);
Из-за проблемы well-known ports программа должна реализовать понижение привилегий (если указаны соответствующие параметры запуска).
- В исходных текстах должна быть предусмотрена обработка всех возможных ошибок и корректное освобождение занятых ресурсов в случае ошибок;
- Программа должна работать с IPv4 и IPv6;
- Программа должна отслеживать таймауты для разрыва соединения с клиентом;
- В силу непереносимости вызова sendfile() в этом сезоне его использовать запрещено;
- Процессы создаются только через fork(). Никакого clone(), он непереносим (а его разрешение дало странный результат в прошлом сезоне);
- Увеличение размера буфера через realloc() / strndup() вообще плохо, а с малым шагом — особенно;
- При системном тестировании из скрипта нужно повторять connect() до успеха, а не спать.

## Реализация
- Конкретный формат очереди(ей) сообщений, куда инфомрация записывается SMTP-сервером и откуда считывается SMTP-клиентом должен совпадать;
- Создание сервера, обеспечивающего локальную доставку и добавление удаленной доставки: 
  - Используется вызов select и рабочие процессы;
  - Журналирование в отдельном процессе;
  - Проверять обратную зону днс.
- Создание клиента, обеспечивающего удаленную доставку и поддерживающего очереди сообщений:
  - Обработка нескольких исходящих соеденений в одном потоке выполнения;
  - На один удаленный MX надо создавать не более одного сокета;
  - Используется вызов pselect и рабочие потоки;
  - Журналирование в отдельном процессе;
  - Пытаться отправлять все сообщения для одного MX за одну сессию.
