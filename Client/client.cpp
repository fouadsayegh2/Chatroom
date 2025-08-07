#include "client.h"

using namespace std;

chatroom::Client::Client(const int port, const std::string &ip)
{
    // 1. Creating the socket:
    sock = socket(AF_INET, SOCK_STREAM, 0); // man 2 socket.
    // Checking if the socket failed.
    if (sock == INVALID_FD)
    {
        perror("Socket");
        exit(EXIT_FAILURE);
    }

    // 2. Building the server address:
    sockaddr_in server{};

    server.sin_family = AF_INET;

    inet_pton(AF_INET, ip.c_str(), &server.sin_addr);

    server.sin_port = htons(port);

    // 3. Connection:
    // Calls the kernel to estabalish a TCP connection from the local socket to the server.
    if (connect(sock, reinterpret_cast<sockaddr *>(&server), sizeof(server)) == INVALID_FD)
    {
        perror("Connect");
        exit(EXIT_FAILURE);
    }

    // 4. Sending the username:
    string name;
    cout << "Please enter your chat name: ";
    getline(cin, name);
    name += '\n';
    send(sock, name.c_str(), name.size(), 0);

    FD_ZERO(&list);
    FD_SET(STDIN_FILENO, &list); // This watches the keyboard (0).
    FD_SET(sock, &list);         // This watches the TCP socket for incoming messages.

    maxFD = sock;
}

chatroom::Client::~Client()
{
    if (sock != -1)
    {
        close(sock);
    }
}

void chatroom::Client::eventLoop()
{
    while (true)
    {
        readableset = list;

        if (select(maxFD + 1, &readableset, nullptr, nullptr, nullptr) == INVALID_FD)
        {
            perror("Select");
            break;
        }

        // Checking if the keyboard input is ready.
        if (FD_ISSET(STDIN_FILENO, &readableset))
        {
            handleKeyboard();
        }

        // Checking if the socket has data or close.
        if (FD_ISSET(sock, &readableset))
        {
            handleSocket();
        }
    }
}

// Sends to the server.
void chatroom::Client::handleKeyboard()
{
    string line;
    if (!getline(cin, line))
    {
        cout << "Goodbye" << endl;
        exit(EXIT_SUCCESS); // Graceful exit.
    }
    line += '\n';
    send(sock, line.c_str(), line.size(), 0);
}

// Recieves from the server.
void chatroom::Client::handleSocket()
{
    char buffer[BUFFERSIZE];

    ssize_t readBytes = recv(sock, buffer, BUFFERSIZE, 0);
    if (readBytes <= 0)
    {
        if (readBytes == 0)
        {
            cerr << "The server has closed it's connection." << endl;
        }
        else
        {
            perror("Recieve");
        }
        exit(EXIT_FAILURE); // Error exit.
    }

    cout.write(buffer, readBytes); // This sends the bytes into the buffer straight to the terminal.
    cout.flush();                  // This forces any text still buffered to appear immediatly.
}
