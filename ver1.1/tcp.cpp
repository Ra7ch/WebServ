#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

const int MAX_CLIENTS = 5;
const int BUFFER_SIZE = 1024;

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket == -1) {
        perror("Error creating socket");
        return EXIT_FAILURE;
    }

    sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(8080);

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        perror("Error binding socket");
        close(serverSocket);
        return EXIT_FAILURE;
    }

    if (listen(serverSocket, MAX_CLIENTS) == -1) {
        perror("Error listening on socket");
        close(serverSocket);
        return EXIT_FAILURE;
    }

    std::cout << "Server is listening on port 8080...\n";

    fd_set masterSet;
    FD_ZERO(&masterSet);
    FD_SET(serverSocket, &masterSet);

    int maxDescriptor = serverSocket;

    while (true) {
        fd_set readSet = masterSet;
        int activity = select(maxDescriptor + 1, &readSet, nullptr, nullptr, nullptr);

        if (activity == -1) {
            perror("Error in select");
            break;
        }

        if (FD_ISSET(serverSocket, &readSet)) {
            // New connection
            int clientSocket = accept(serverSocket, nullptr, nullptr);

            if (clientSocket == -1) {
                perror("Error accepting connection");
            } else {
                std::cout << "New connection accepted. Client socket: " << clientSocket << std::endl;
                FD_SET(clientSocket, &masterSet);
                maxDescriptor = std::max(maxDescriptor, clientSocket);
            }
        }

        for (int i = serverSocket + 1; i <= maxDescriptor; ++i) {
            if (FD_ISSET(i, &readSet)) {
                // Data received on a client socket
                char buffer[BUFFER_SIZE];
                ssize_t bytesRead = recv(i, buffer, sizeof(buffer), 0);

                if (bytesRead <= 0) {
                    // Connection closed or error
                    if (bytesRead == 0) {
                        std::cout << "Client " << i << " disconnected.\n";
                    } else {
                        perror("Error receiving data");
                    }
                    close(i);
                    FD_CLR(i, &masterSet);
                } else {
                    // Process received data
                    buffer[bytesRead] = '\0';
                    std::cout << "Received data from client " << i << ": " << buffer << std::endl;

                    // send back to the client
                    //const char http[10000] = "HTTP/1.1 200 OK\nContent-Type: text/html\nConnection: close\n\n<!DOCTYPE html>\n<html>\n<head>\n<title>Simple Page</title>\n</head>\n<body>\n<h1>Hello, World!</h1>\n<p>This is a simple HTML page.</p>\n</body>\n</html>\n\0";
                    for (int j = 1; j <= maxDescriptor; j++) {
                        if (FD_ISSET(j, &masterSet)) {
                            if (j != i && j != serverSocket)
                                send(j, buffer, bytesRead, 0);
                        }
                    }
                   // send(i, http, strlen(http), 0);
                }
            }
        }
    }

    // Close all client sockets
    for (int i = serverSocket + 1; i <= maxDescriptor; ++i) {
        if (FD_ISSET(i, &masterSet)) {
            close(i);
        }
    }

    // Close the server socket
    close(serverSocket);

    return 0;
}
