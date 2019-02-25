#!/bin/bash
((
echo open 127.0.0.1 2525
sleep 2
echo "EHLO my-test.ru"
sleep 2
echo "VRFY my-test.ru"
sleep 2
echo "MAIL <a@yandex.ru>"
sleep 2
echo "RCPT <b@mail.ru>"
sleep 2
echo "DATA"
sleep 2
echo "Hello, cruel world!"
sleep 2
echo "Im sleep.."
sleep 2
echo "."
sleep 2
echo "QUIT"
sleep 2
) | telnet)