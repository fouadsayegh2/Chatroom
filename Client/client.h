#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string>

using namespace std;

#define INVALID_FD -1

namespace chatroom
{
    class Client
    {
    public:
        Client(const int port, const string &ip);
        ~Client();

        void eventLoop();

    private:
        void handleKeyboard(); // This handles new connections.
        void handleSocket();

        int sock{-1};

        fd_set list;        // This contains every socket the server cares about.
        fd_set readableset; // This is a copy the code passes to select(). After it returns, it shows which sockets in the list are ready for reading.

        int maxFD{-1};

        size_t BUFFERSIZE{4096};
    };
}