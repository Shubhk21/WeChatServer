# WeChatServer

This is server implementation for WeChat application.

1) Cross Platform (Windows and Unix)
2) Capable of handling thousands of clients at moment.
3) Kills the bottelneck of one socket for each client using IOCP(windows) and kqueue(Unix).
4) Handles authentication(login/register) and data(message) transfer using thread pooling.
5) Takes care of session management(No duplicate logins for same user).


# Tools and Dependencies  

1) Build - Cmake
2) Database - PostgreSQL (libpq)

