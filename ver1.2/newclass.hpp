#include "webServ.hpp"


class Client {
    int clientSocket;
    sockaddr_in clientAddress;
    socklen_t clientAddressLength;
    char buffer[BUFFER_SIZE];
    ssize_t bytesRead;
    char body[BUFFER_SIZE];
    char *FullRequest;
    Client* next;

    public:
        Client(int clientSocket, sockaddr_in clientAddress, socklen_t clientAddressLength) {
            this->clientSocket = clientSocket;
            this->clientAddress = clientAddress;
            this->clientAddressLength = clientAddressLength;
        }
            int getSocketDescriptor() const {
            return clientSocket;
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
};

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
    map<string, string> body;
    ofstream RequestFile;
    RequestState state = RequestState::HEADERS;
    int level = 0;
    char *key;
    char *value;
    // /n/r
    public:
        Request(char *FullRequest) {
            this->FullRequest = FullRequest;
            RequestFile.open(".HttpRequest");
            if (!RequestFile.is_open()) {
                std::cerr << "Error opening file for appending Request\n";
                exit (1);
            }
        }

        void readFullRequest(char *buffer) {
            RequestFile.open(".HttpRequest");
            if (!RequestFile.is_open()) {
                std::cerr << "Error opening file for appending Request\n";
                exit (1);
            }
            RequestFile << buffer;
            const char *endHeader1 = strstr(buffer, "\r\n\r\n");
            if (endHeader1 != NULL && state == RequestState::HEADERS) {
                parseRequest();
                state = RequestState::BODY;
            }
            if (state == RequestState::BODY) {
                RequestFile << buffer;
                // create a file for the body and handle it
            }
        }
        
        void parseRequest() {
            // Read and store the request type
            std::ifstream RequestFile2(".HttpRequest");
            RequestFile2.open(".HttpRequest");
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
            string body = nullptr;
            while (std::getline(RequestFile2, line)) {
                body += line;
            }

            // Clear the content of the hidden file for storing the body
            std::ofstream bodyFile(".HttpRequest", std::ios::trunc);
            if (!bodyFile.is_open()) {
                std::cerr << "Error opening file for appending Request\n";
                exit (1);
            }
            bodyFile << body;
            bodyFile.close();
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
        void printResponce() {
            std::cout << "Status: " << status << std::endl;
            std::cout << "Server: " << server << std::endl;
            std::cout << "Date: " << date << std::endl;
            std::cout << "Content-Type: " << content_type << std::endl;
            std::cout << "Content-Length: " << content_length << std::endl;
            std::cout << "Connection: " << connection << std::endl;
        }
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