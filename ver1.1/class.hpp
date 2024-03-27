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

class Request {
    int client;
    char *method; // 4
    char *path; // 1020
    char *version; 
    char *host;
    char *connection;
    char *cache_control;
    char *user_agent;
    char *accept;
    char *accept_encoding;
    char *accept_language;
    char *body;
    string type;
    char *FullRequest;
    map<string, string> headers;
    ofstream RequestFile;
    RequestState state = RequestState::HEADERS;
    int level = 0;
    char *key;
    char *value;
    char *filename;
    // /n/r
    public:
        Request(char *FullRequest, int client): client(client) {
            string file  = ".HttpRequest";
            filename = ft_strjoin(const_cast<char *>(file.c_str()), const_cast<char *>(std::to_string(client).c_str()));
            this->FullRequest = FullRequest;
            RequestFile.open(filename, std::ios::out | std::ios::trunc);
            if (!RequestFile.is_open()) {
                std::cerr << "Error opening file for appending Request1\n";
                exit (1);
            }
            state = RequestState::HEADERS;
        }

        void readFullRequest(char *buffer) {
            if (state == RequestState::HEADERS) {
                char* b = buffer;
                RequestFile << b;
                // if (chmod(".HttpBodyRequest", S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH | S_IWOTH | S_IWGRP) != 0) {
                //     std::cerr << "Error changing file permissions." << std::endl;
                //     exit (1);
                // }
                const char *endHeader1 = strstr(buffer, "\r\n\r\n");
                if (endHeader1 != NULL) {
                    parseHeadersRequest();
                    state = RequestState::BODY;
                }
            }
            else if (state == RequestState::BODY) {
                std::istringstream ss(buffer);
                string body;
                string line;
                auto transferEncodingIterator = headers.find("Transfer-Encoding");
                if (transferEncodingIterator != headers.end() && transferEncodingIterator->second == "chunked") {
                    // Read the chunked body
                    while (std::getline(ss, line)) {
                        if (line == "\r") {
                            // skip the size of the chunk
                            std::getline(ss, line);
                            continue;
                        }
                        if (line.empty()) {
                            // Empty line indicates end of chunked body
                            break;
                        }
                        body += line;
                    }
                } else {
                    body += line;
                }
                RequestFile << body;
                cout << "body1: " << body << endl;
                const char *endbody = strstr(buffer, "\r\n\r\n");
                if (endbody != NULL) {
                    state = RequestState::HEADERS;
                }
                // create a file for the body and handle it
            }
        }

        // fill the map with the headers and handle the chunked body if there is some here
        void parseHeadersRequest() {
            // Read and store the request type
            std::ifstream RequestFile2(filename);
            RequestFile2.open(filename);
            if (!RequestFile2.is_open()) {
                std::cerr << "Error opening file for reading Request\n";
                exit (1);
            }
            std::getline(RequestFile2, type);

            std::string line;
            while (std::getline(RequestFile2, line)) {
                if (line.empty() || line == "\r") {
                    // Empty line indicates end of headers
                    break;
                }
                size_t pos = line.find(": ");
                if (pos != std::string::npos) {
                    std::string key = line.substr(0, pos);
                    std::string value = line.substr(pos + 2); // Skip ": "
                    headers[key] = value; // Store the header in the map
                }
            }
            string body;
            auto transferEncodingIterator = headers.find("Transfer-Encoding");
            if (transferEncodingIterator != headers.end() && transferEncodingIterator->second == "chunked") {
                // Read the chunked body
                while (std::getline(RequestFile2, line)) {
                    if (line == "\r") {
                        // skip the size of the chunk
                        std::getline(RequestFile2, line);
                        continue;
                    }
                    if (line.empty()) {
                        // Empty line indicates end of chunked body
                        break;
                    }
                    body += line;
                }
            } else {
                body += line;
            }
            // Clear the content of the hidden file for storing the body
            RequestFile.close();
            RequestFile.open(filename, std::ios::out | std::ios::trunc);
            if (!RequestFile.is_open()) {
                std::cerr << "Error opening file for appending Request2\n";
                exit (1);
            }
            cout << "body2: " << body << endl;
            RequestFile << body;
            RequestFile2.close();
        }
        
        void parseRequest() {
            // Read and store the request type
            std::ifstream RequestFile2(filename);
            RequestFile2.open(filename);
            if (!RequestFile2.is_open()) {
                std::cerr << "Error opening file for reading Request\n";
                exit (1);
            }
            std::getline(RequestFile2, type);

            std::string line;
            while (std::getline(RequestFile2, line)) {
                if (line.empty() || line == "\r") {
                    // Empty line indicates end of headers
                    break;
                }
                size_t pos = line.find(": ");
                if (pos != std::string::npos) {
                    std::string key = line.substr(0, pos);
                    std::string value = line.substr(pos + 2); // Skip ": "
                    headers[key] = value; // Store the header in the map
                }
            }

            // Clear the content of the hidden file for storing the body
            std::ofstream bodyFile(".HttpBodyRequest", std::ios::trunc);
            if (!bodyFile.is_open()) {
                std::cerr << "Error opening file for appending Request3\n";
                exit (1);
            }
            while (std::getline(RequestFile2, line)) {
                bodyFile << line;   // Store the body in the file
            }

            bodyFile.close();
            RequestFile2.close();
        }

        ~Request() {
            RequestFile.close();
        }
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
    Request *request;
    Request2 *request2;
    int ServerSocket;
  //  Responce *responce;
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    int port;
    std::string ip;
    string host;
    bool responceReady = false;

    public:
        Client(int& clientSocket, sockaddr_in& clientAddress, socklen_t& clientAddressLength, int ServerSocket) {
            this->clientSocket = clientSocket;
            // Request *request = new Request(FullRequest, clientSocket);
            this->clientAddress = clientAddress;
            this->clientAddressLength = clientAddressLength;
            this->ServerSocket = ServerSocket;
            //this->next = nullptr;
            //this->request = request;
            //int clientSocket2 = 0;
            //clientSocket2 = clientSocket;
            //request2 = new Request2(clientSocket2);
            //Request2 request2(clientSocket);
            this->request2 = new Request2(clientSocket);
            this->request2->buffer = buffer;
            if (getsockname(clientSocket, (struct sockaddr*)&clientAddr, &addrLen) == -1) {
                perror("Error getting client address");
                return;
            }
            port = ntohs(clientAddr.sin_port);
            std::cout << "Client connected from port: " << port << std::endl;
        }
            int getSocketDescriptor() const {
            return clientSocket;
        }

        Request* setRequest() {
            return request;
        }

        Request2* setRequest2() {
            return request2;
        }

        Request2* getRequest2() const {
            return request2;
        }

        Request* getRequest() const {
            return request;
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