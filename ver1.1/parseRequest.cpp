#include "webServ.hpp"

class Request2 {
    enum State {
        REQUEST_LINE,
        HEADERS,
        BODY
    };
    public:
        int client;
        std::string method;
        std::string path;
        std::string version;
        std::map<std::string, std::string> headers;
        std::string body;
        State state = REQUEST_LINE;
        bool lineComplete = true;
        bool headersComplete = false;
        std::string header;
        string file = ".data" + client;
        bool bodyComplete = true;
        bool chunked = false;
        int content_length = INT_MAX;


        Request2(int client) : client(client) {
            ofstream bodyFile(file, ios::out | ios::trunc);
            bodyFile.close();

        }
    
        void parse(const std::string& request) {
            std::istringstream requestStream(request);
            std::string line;
            if (state == REQUEST_LINE) {
                std::getline(requestStream, line);
                std::istringstream lineStream(line);
                lineStream >> method >> path >> version;
                state = HEADERS;
            }
            while (std::getline(requestStream, line) && state == HEADERS) {
                if (line == "\r") {
                    headersComplete = true;
                    state = BODY;
                    break;
                }
                if (line.back() == '\r') {
                    if (lineComplete) {
                        std::string::size_type pos = line.find(':');
                        header = line.substr(0, pos);
                        headers[header] = line.substr(pos + 2);
                    }
                    else {
                        std::string::size_type pos = line.find('\r');
                        headers[header] += line.substr(0, pos);
                    }
                    lineComplete = true;
                }
                else {
                    if (lineComplete) {
                        std::string::size_type pos = line.find(':');
                        header = line.substr(0, pos);
                        headers[header] = line.substr(pos + 2);
                    }
                    else {
                        headers[header] += line;
                    }
                    lineComplete = false; 
                    if (header == "Transfer-Encoding" && headers[header] == "chunked") {
                        chunked = true;
                    }
                    if (header == "Content-Length") {
                        content_length = stoi(headers[header]);
                    }
                }
            }
            if (chunked && state == BODY) {
                if (bodyComplete) {
                    string chunk;
                    getline(requestStream, chunk);
                    int chunkSize = stoi(chunk, 0, 16);
                    if (chunkSize == 0) {
                        state = REQUEST_LINE;
                        bodyComplete = false;
                        return ;
                    }
                    bodyComplete = false;
                }
                ofstream bodyFile(file, ios::out | ios::app);
                while (getline(requestStream, body)) {
                    if (body == "\r") {
                        bodyComplete = true;
                        break;
                    }
                    bodyFile << body;
                }
                bodyFile.close();
            }
            else if (!chunked && state == BODY) {
                ofstream bodyFile(file, ios::out | ios::app);
                while (getline(requestStream, body)) {
                    if (body == "\r" || body.empty()) {
                        break;
                    }
                    bodyFile << body;
                    if (content_length != INT_MAX)
                        content_length -= body.length();
                    if (content_length <= 0) {
                        bodyComplete = false;
                        state = REQUEST_LINE;
                        break;
                    }
                }
                bodyFile.close();
            }
        }

};