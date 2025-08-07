#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <map>

using namespace std;

#define INVALID_FD -1

namespace chatroom
{
    class Server
    {
    public:
        Server(const int port, const string &ip);
        ~Server();

        void eventLoop();

    private:
        void acceptClient(); // This handles new connections.
        void handleClient(int FD);
        void broadcast(const string &payload, int otherFD);

        int listeningFD{-1};

        fd_set list;        // This contains every socket the server cares about.
        fd_set readableset; // This is a copy the code passes to select(). After it returns, it shows which sockets in the list are ready for reading.

        int maxFD{-1};

        size_t BUFFERSIZE{4096};

        map<int, string> names; // This holds the client's socket FD and username.
    };
}