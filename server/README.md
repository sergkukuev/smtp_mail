## Серверная часть SMTP-сервер ()

**NOTE:** Проверять работоспособность сервера с помощью telnet'a.

### Unit tests
>
>
>     CUnit - A unit testing framework for C - Version 2.1-2
>     http://cunit.sourceforge.net/
>
>
Suite: suite_test</br>
&nbsp;&nbsp;&nbsp;&nbsp;  Test: socket_listen_correct ...passed </br>
&nbsp;&nbsp;&nbsp;&nbsp;  Test: socket_bind_client_data_correct ...passed</br>
&nbsp;&nbsp;&nbsp;&nbsp;  Test: socket_accept_incorrect ...passed</br>
&nbsp;&nbsp;&nbsp;&nbsp;  Test: socket_clients_close_correct ...passed</br>
&nbsp;&nbsp;&nbsp;&nbsp;  Test: socket_send_data_incorrect ...passed</br>
&nbsp;&nbsp;&nbsp;&nbsp;  Test: socket_recv_data_incorrect ...passed</br>
&nbsp;&nbsp;&nbsp;&nbsp;  Test: full_correct_session ...passed</br>
&nbsp;&nbsp;&nbsp;&nbsp;  Test: handle_helo_correct ...passed</br>
&nbsp;&nbsp;&nbsp;&nbsp;  Test: handle_helo_incorrect ...passed</br>
&nbsp;&nbsp;&nbsp;&nbsp;  Test: handle_ehlo_correct ...passed</br>
&nbsp;&nbsp;&nbsp;&nbsp;  Test: handle_ehlo_incorrect ...passed</br>
&nbsp;&nbsp;&nbsp;&nbsp;  Test: handle_mail_correct ...passed</br>
&nbsp;&nbsp;&nbsp;&nbsp;  Test: handle_mail_incorrect_state ...passed</br>
&nbsp;&nbsp;&nbsp;&nbsp;  Test: handle_mail_incorrect ...passed</br>
&nbsp;&nbsp;&nbsp;&nbsp;  Test: handle_rcpt_correct ...passed</br>
 &nbsp;&nbsp;&nbsp;&nbsp; Test: handle_rcpt_incorrect_state ...passed</br>
&nbsp;&nbsp;&nbsp;&nbsp;  Test: handle_rcpt_incorrect ...passed</br>
&nbsp;&nbsp;&nbsp;&nbsp;  Test: handle_data_correct ...passed</br>
&nbsp;&nbsp;&nbsp;&nbsp;  Test: handle_data_incorrect_state ...passed</br>
&nbsp;&nbsp;&nbsp;&nbsp;  Test: handle_data_incorrect ...passed</br>
&nbsp;&nbsp;&nbsp;&nbsp;  Test: handle_rset_correct ...passed</br>
&nbsp;&nbsp;&nbsp;&nbsp;  Test: handle_rset_incorrect ...passed</br>
&nbsp;&nbsp;&nbsp;&nbsp;  Test: handle_vrfy_correct ...passed</br>
&nbsp;&nbsp;&nbsp;&nbsp;  Test: handle_vrfy_incorrect ...passed</br>
&nbsp;&nbsp;&nbsp;&nbsp;  Test: handle_quit_correct ...passed</br>
&nbsp;&nbsp;&nbsp;&nbsp;  Test: handle_quit_incorrect ...passed</br>
&nbsp;&nbsp;&nbsp;&nbsp;  Test: handle_noop_correct ...passed</br>
&nbsp;&nbsp;&nbsp;&nbsp;  Test: handle_noop_incorrect ...passed</br>
&nbsp;&nbsp;&nbsp;&nbsp;  Test: handle_undef_correct ...passed</br>
&nbsp;&nbsp;&nbsp;&nbsp;  Test: handle_undef_incorrect ...passed</br>

| Run Summary: |   Type | Total  |  Ran | Passed|   Failed |  Inactive |
|--------------|:------:|:------:|:----:|:-----:|:--------:|-----------|
|       | suites   |   1   |   1  |  n/a     | 0      |  0 |
|       | tests    |  30   |  30  |   30     | 0      |  0 |
|       |     asserts  |  108  |  108 |   108  |    0   |   n/a |

**Elapsed time** =    0.000 seconds

