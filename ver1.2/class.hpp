#ifndef CLASS_HPP
#define CLASS_HPP

#include "webServ.hpp"

char *ft_strjoin(char *s1, char *s2) {
    char *newStr = (char *)malloc(strlen(s1) + strlen(s2) + 1);
    strcpy(newStr, s1);
    strcat(newStr, s2);
    return newStr;
}

enum class RequestState {
    REQUEST_LINE,
    HEADERS,
    BODY
};

//////////////////////////////////////////////
///////////////////////////////////////
/////////////////////////////////////////
////////////////////////////////////////


//////////////////////////////////////////////
///////////////////////////////////////
/////////////////////////////////////////
////////////////////////////////////////
//////////////////////////////////////////////
///////////////////////////////////////

class Client {
    int clientSocket;
    sockaddr_in clientAddress;
    socklen_t clientAddressLength;
    char buffer[BUFFER_SIZE + 1];
    ssize_t bytesRead;
    char body[BUFFER_SIZE ];
    char *FullRequest;
    Client* next;
    Request2 *request2;
    int ServerSocket;
  //  Responce *responce;
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    int port;
    std::string ip;
    string host;
    bool responceReady = false;
    GlobalConfig *config;

    public:
        Client(int& clientSocket, sockaddr_in& clientAddress, socklen_t& clientAddressLength, int ServerSocket, GlobalConfig *config) {
            this->clientSocket = clientSocket;
            // Request *request = new Request(FullRequest, clientSocket);
            this->clientAddress = clientAddress;
            this->clientAddressLength = clientAddressLength;
            this->ServerSocket = ServerSocket;
            this->config = config;
            //this->next = nullptr;
            //this->request = request;
            //int clientSocket2 = 0;
            //clientSocket2 = clientSocket;
            //request2 = new Request2(clientSocket2);
            //Request2 request2(clientSocket);
            this->request2 = new Request2(clientSocket, config);
            this->request2->buffer = buffer;
            if (getsockname(clientSocket, (struct sockaddr*)&clientAddr, &addrLen) == -1) {
                perror("Error getting client address");
                return;
            }
            port = ntohs(clientAddr.sin_port);
            std::cout << "Client connected from port: " << port << std::endl;
        }
        
        const int getSocketDescriptor() const {
            return clientSocket;
        }

        Request2* setRequest2() {
            return request2;
        }

        Request2* getRequest2() const {
            return request2;
        }

        const sockaddr_in& getClientAddress() const {
            return clientAddress;
        }

        char* getBuffer() {
            return buffer;
        }

        ssize_t getBytesRead() const {
            return bytesRead;
        }

        void setBytesRead(ssize_t bytesRead) {
            this->bytesRead = bytesRead;
        }

        Client* getNext() const {
            return next;
        }

        void setNext(Client* next) {
            this->next = next;
        }

        void setHost(string host) {
            this->host = host;
        }
};

class Responce {
    char *status;
    char *server;
    char *date;
    char *content_type;
    char *content_length;
    char *connection;
    char *body;
    char *FullResponce;
    public:
        Responce(char *FullResponce) {
            this->FullResponce = FullResponce;
        }
        void parseResponce() {
            char *token = strtok(FullResponce, "\n");
            while (token != NULL) {
                if (strstr(token, "HTTP/1.1") != NULL) {
                    status = token;
                } else if (strstr(token, "Server") != NULL) {
                    server = token;
                } else if (strstr(token, "Date") != NULL) {
                    date = token;
                } else if (strstr(token, "Content-Type") != NULL) {
                    content_type = token;
                } else if (strstr(token, "Content-Length") != NULL) {
                    content_length = token;
                } else if (strstr(token, "Connection") != NULL) {
                    connection = token;
                }
                token = strtok(NULL, "\n");
            }
        }
        // void printResponce() {
        //     std::cout << "Status: " << status << std::endl;
        //     std::cout << "Server: " << server << std::endl;
        //     std::cout << "Date: " << date << std::endl;
        //     std::cout << "Content-Type: " << content_type << std::endl;
        //     std::cout << "Content-Length: " << content_length << std::endl;
        //     std::cout << "Connection: " << connection << std::endl;
        // }
};
//         void parseRequest() {
//             char *k = nullptr;
//             char *v = nullptr;
//             char *b = nullptr;
//             if (level == 0) {
//                 while (level == 0 && FullRequest != NULL) {
//                     k = strtok(FullRequest, ": ");
//                     v = strtok(NULL, "\r\n");
//                     b = strtok(NULL, "\r\n\r\n");
//                     key = ft_strjoin(key, k);
//                     if (v) {
//                         value = ft_strjoin(value, v);
//                         level ++;
//                         headers[key] = value;
//                     }
//                     if (b) {level = 2; RequestFile << b;}
//                 }
//             }
//             else if (level == 1) {
//                 while (level == 1 && FullRequest != NULL) {
//                     v = strtok(FullRequest, "\r\n");
//                     b = strtok(NULL, "\r\n\r\n");
//                     if (!b)
//                         k = strtok(NULL, ": ");
//                     value = ft_strjoin(value, v);
//                     if (k) {
//                         key = ft_strjoin(key, k);
//                         level = 0;
//                         headers[key] = value;
//                     }
//                     if (b) {level = 2; RequestFile << b;}
//                 }
//             }
//             else if (level == 2) {
//                 RequesFile << FullRequest;
//             }
//         }

//         void printRequest() {
//             std::cout << "Method: " << method << std::endl;
//             std::cout << "Host: " << host << std::endl;
//             std::cout << "Connection: " << connection << std::endl;
//             std::cout << "Cache-Control: " << cache_control << std::endl;
//             std::cout << "User-Agent: " << user_agent << std::endl;
//             std::cout << "Accept: " << accept << std::endl;
//             std::cout << "Accept-Encoding: " << accept_encoding << std::endl;
//             std::cout << "Accept-Language: " << accept_language << std::endl;
//             std::cout << "Version: " << version << std::endl;
//         }
// };


#endif