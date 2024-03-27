#include "class.hpp"


int main(int argc, char **argv) {
    // Parsing config file
    if (argc != 2) {
        std::cout << "Check your input again..." << std::endl;
        return 1;
    }

    GlobalConfig config(argv[1]);
    vector < int > Ports = config.getPorts();
    vector <int> serverSockets;

    int j = 0;
    // int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    // if (serverSocket == -1) {
    //     perror("Error creating socket");
    //     return EXIT_FAILURE;
    // }


    // Listening on all ports
    // create a socket for each port and add it to the epool or a vector and then listen on all of them
    for (int i = 0; i < Ports.size(); i++) {
        int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == -1) {
            perror("Error creating socket");
            return EXIT_FAILURE;
        }
        serverSockets.push_back(serverSocket);
        struct sockaddr_in serverAddress;
        memset( & serverAddress, 0, sizeof(serverAddress));
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = INADDR_ANY;
        serverAddress.sin_port = htons(Ports[i]);
        cout << "1- " << Ports[i] << endl;

        if (bind(serverSocket, (struct sockaddr * ) & serverAddress, sizeof(serverAddress)) == -1) {
            perror("Error binding socket");
            close(serverSocket);
            return EXIT_FAILURE;
        }

        if (listen(serverSocket, MAX_CLIENTS) == -1) {
            perror("Error listening on socket");
            close(serverSocket);
            return EXIT_FAILURE;
        }

        std::cout << "Server is listening on port " << Ports[i] << "...\n";
    }

    // sockaddr_in serverAddress;
    // memset(&serverAddress, 0, sizeof(serverAddress));
    // serverAddress.sin_family = AF_INET;
    // serverAddress.sin_addr.s_addr = INADDR_ANY;
    // serverAddress.sin_port = htons(8080);

    // if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
    //     perror("Error binding socket");
    //     close(serverSocket);
    //     return EXIT_FAILURE;
    // }

    // if (listen(serverSocket, MAX_CLIENTS) == -1) {
    //     perror("Error listening on socket");
    //     close(serverSocket);
    //     return EXIT_FAILURE;
    // }

    //std::cout << "Server is listening on port 8080...\n";

    int epollFd = epoll_create1(0);
    if (epollFd == -1) {
        perror("Error creating epoll instance");
        for (int i = 0; i < serverSockets.size(); i++) {
            close(serverSockets[i]);
        }
        //close(serverSocket);
        return EXIT_FAILURE;
    }

    // struct epoll_event event;
    // event.events = EPOLLIN;
    // event.data.fd = serverSocket;
    // if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverSocket, &event) == -1) {
    //     perror("Error adding server socket to epoll instance");
    //     close(serverSocket);
    //     close(epollFd);
    //     return EXIT_FAILURE;
    // }

    // Create a vector to store epoll events for each server socket
    std::vector<struct epoll_event> serverEvents(serverSockets.size());

    // Add each server socket to the epoll instance
    for (int i = 0; i < serverSockets.size(); i++) {
        serverEvents[i].events = EPOLLIN;
        serverEvents[i].data.fd = serverSockets[i];
        if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverSockets[i], &serverEvents[i]) == -1) {
            perror("Error adding server socket to epoll instance");
            close(serverSockets[i]);
            close(epollFd);
            return EXIT_FAILURE;
        }
    }

    struct epoll_event events[((MAX_CLIENTS + 1) * serverSockets.size())]; // Events array for epoll_wait

    Client* head = nullptr; // Head of the linked list

    while (true) {
        int numEvents = epoll_wait(epollFd, events, ((MAX_CLIENTS + 1) * serverSockets.size()), -1);
        if (numEvents == -1) {
            perror("Error in epoll_wait");
            break;
        }

        for (int i = 0; i < numEvents; ++i) {
            int fd = events[i].data.fd;

            vector<int>::iterator it = find(serverSockets.begin(), serverSockets.end(), fd);

            if (it != serverSockets.end()) {
                // New connection
                sockaddr_in clientAddress;
                socklen_t clientAddressLen = sizeof(clientAddress);
                int clientSocket = accept(*it, (struct sockaddr*)&clientAddress, &clientAddressLen);
                if (clientSocket == -1) {
                    perror("Error accepting connection");
                } else {
                    std::cout << "New connection accepted. Client socket: " << clientSocket << std::endl;

                    // Create a new client object
                    //getpeername(clientSocket, (struct sockaddr*)&clientAddress, &clientAddressLen);
                    Client* newClient = new Client(clientSocket, clientAddress, clientAddressLen, *it);

                    // Add the new client to the linked list
                    newClient->setNext(head);
                    head = newClient;

                    // Add the new client socket to the epoll instance
                    struct epoll_event event;
                    event.events = EPOLLIN | EPOLLOUT;
                    event.data.fd = clientSocket;
                    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocket, &event) == -1) {
                        perror("Error adding client socket to epoll instance");
                        close(clientSocket);
                    }
                }
            } else {
                // Data received on a client socket
                Client* current = head;
                while (current != nullptr) {
                    if (current->getSocketDescriptor() == fd && (events[i].events & EPOLLIN)) {
                       // vector<char> buffer(BUFFER_SIZE);
                        ssize_t bytesRead = recv(fd, current->getBuffer(), BUFFER_SIZE, 0);
                        
                        if (bytesRead <= 0) {
                            // Connection closed or error
                            if (bytesRead == 0) {
                                std::cout << "Client " << fd << " disconnected.\n";
                            } else {
                                perror("Error receiving data");
                            }
                            close(fd);
                        } else {
                            // Process received data
                            current->getBuffer()[bytesRead] = '\0';
                            current->setBytesRead(bytesRead);
                            /////
                            current->getRequest2()->parse(bytesRead);
                            current->setHost(current->getRequest2()->getHost());
                            cout << "*******************************" << endl;
                            current->getRequest2()->printHeaders();

                            // Echo back to the client
                            //send(fd, current->getBuffer(), bytesRead, 0);
                        }
                        break;
                    }
                    current = current->getNext();
                }
            }
        }
    }

    // Close all client sockets and delete client objects
    Client* current = head;
    while (current != nullptr) {
        close(current->getSocketDescriptor());
        Client* next = current->getNext();
        delete current;
        current = next;
    }

    // Close the server socket
    // close(serverSocket);
    for (int i = 0; i < serverSockets.size(); i++) {
        close(serverSockets[i]);
    }
    close(epollFd);

    return 0;
}
