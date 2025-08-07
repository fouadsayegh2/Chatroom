#include "server.h"

int main()
{
    chatroom::Server chat(54000, "127.0.0.1");
    chat.eventLoop();
    return 0;
}