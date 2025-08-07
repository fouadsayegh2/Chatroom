#include "client.h"

int main()
{
    chatroom::Client chat(54000, "127.0.0.1");
    chat.eventLoop();
    return 0;
}