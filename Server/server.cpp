#include "server.h"

using namespace std;

chatroom::Server::Server(const int port, const std::string &ip)
{
    listeningFD = socket(AF_INET, SOCK_STREAM, 0); // man 2 socket.
    // cout << listeningfd << endl;
    // Checking if the socket failed.
    if (listeningFD == INVALID_FD)
    {
        perror("Socket");
        exit(EXIT_FAILURE);
    }

    int option = 1;
    setsockopt(listeningFD, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    // 2. Binding the address to the port:
    sockaddr_in address{}; // This creates and zero-initialize an IPv4-address structure (struct found in netinet/in.h).

    address.sin_family = AF_INET; // This tells the kernel that this address belongs to the IPv4 family.

    inet_pton(AF_INET, ip.c_str(), &address.sin_addr); // This turns human-readable IP string into the binary form that the kernel needs (found in arpa/inet.h).

    address.sin_port = htons(port); // This converts the host-order port number to big-endian netrowrk byte order. This is so that the kernel uses the correct TCP port on the wire.

    // This tries to attach the socket to the specified IP-and-port. Checking if the bind failed.
    if (::bind(listeningFD, reinterpret_cast<sockaddr *>(&address), sizeof(address)) == INVALID_FD)
    // reinterpret_cast<sockaddr*>(&addr) lets the compiler treat the address of your IPv4 struct as a pointer of the generic type that bind() expects.
    {
        perror("Bind");
        exit(EXIT_FAILURE);
    }

    // 3. Starting the listening:
    // This tells the kernel to start accepting incoming connections. Checks if fails.
    if (listen(listeningFD, 10) == INVALID_FD)
    {
        perror("Listen");
        exit(EXIT_FAILURE);
    }

    FD_ZERO(&list);             // This clears the list so it starts empty.
    FD_SET(listeningFD, &list); // This adds the socket to the list that select() should watch for incoming data.

    maxFD = listeningFD; // Initializing it with that value gaurantees the very first call to select() gets the right range.

    cout << "Server is listening on " << ip << ":" << port << endl;
}

chatroom::Server::~Server()
{
    if (listeningFD != -1)
    {
        close(listeningFD);
    }
}

void chatroom::Server::eventLoop()
{
    while (true)
    {
        readableset = list; // This take a fresh copy of every FD we care about, sinze select() erases every bit that isn't ready.

        // This checks if select() failed.
        if (select(maxFD + 1, &readableset, nullptr, nullptr, nullptr) == INVALID_FD)
        // The + 1 is added since select() only scans descriptor through nfds - 1.
        {
            perror("Select");
            break;
        }
        // cout << "end" << endl;

        // This loops through every possible FD value.
        for (int FD = 0; FD <= maxFD; FD++)
        {
            if (!FD_ISSET(FD, &readableset))
            {
                continue;
            }
            if (FD == listeningFD)
            {
                acceptClient();
            }
            else
            {
                handleClient(FD);
            }
        }
    }
}

void chatroom::Server::acceptClient()
{
    // This is whatever free FD was avaiable for the cleint connection.
    int clientFD = accept(listeningFD, nullptr, nullptr);
    // This checks if accept() fails.
    if (clientFD == INVALID_FD)
    {
        perror("Accept");
        return;
    }

    // Basically adds the newly accepted client socket to the list that select() should monitor for incoming messages.
    FD_SET(clientFD, &list);

    maxFD = max(maxFD, clientFD);
}

void chatroom::Server::handleClient(int FD)
{
    char buffer[BUFFERSIZE];
    // Signed so it can have INVALID_FD. This waits for the incoming data on FD, and puts it in the buffer. It returns how many bytes it recieved.
    ssize_t readBytes = recv(FD, buffer, BUFFERSIZE, 0);

    // This deals with if the client closed / disconnected (0) or there is an error (INVALID_FD).
    if (readBytes <= 0)
    {
        if (names.count(FD))
        {
            broadcast("**** " + names[FD] + " left ****\n", FD);
        }
        close(FD);         // This closes the client's socket.
        FD_CLR(FD, &list); // This removes that socket's but form the list, so select() stops watching it.
        names.erase(FD);   // This deletes the map entry that held that client's username.
        return;
    }

    // The string that holds the raw chat message (start and end of the memory range).
    string payload(buffer, buffer + readBytes);

    // Basically, if this socket's FD isn't already in the map, the username hasn't been stored yet, so we treat the incoming text as the client's username.
    if (!names.count(FD))
    {
        // Striping the trailing newline and stores the recieved username in the map, keyed by the client's socket FD.
        payload.erase(payload.find('\n'));
        names[FD] = payload;
        broadcast("**** " + payload + " joined ****\n", FD);
    }
    else
    {
        broadcast(names[FD] + ": " + payload, FD);
    }
}

void chatroom::Server::broadcast(const string &payload, int otherFD)
{
    cout << payload;

    for (const auto& p : names)
    {
        int FDTwo = p.first;
        if (FDTwo != otherFD) {
            ::send(FDTwo, payload.c_str(), payload.size(), 0);
        }
    }
}
