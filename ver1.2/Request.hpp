#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "webServ.hpp"
using namespace std;

class Request2 {
    enum State {
        REQUEST_LINE,
        HEADERS,
        BODY
    };
    enum ChunkState {
        CHUNK_SIZE,
        CHUNK_DATA,
        CHUNK_END
    };
    public:
        int client;
        char *buffer;
        std::string method;
        std::string path;
        std::string version;
        std::map<std::string, std::string> headers;
        std::string body;
        State state = REQUEST_LINE;
        bool lineComplete = true;
        bool headersComplete = false;
        std::string header;
        string file;
        bool bodyComplete = true;
        bool chunked = false;
        long content_length = LONG_MIN;
        int header_length = 0;
        ofstream bodyFile;
        long chunkSize = 0;
        string chunkSizeStr;
        ChunkState chunkState = CHUNK_SIZE;
        char lastchar = 0;
        int limit = 0;
        size_t i;
        std::string request;
        string requestPath;
        GlobalConfig *config;
        bool fileCreated = false;



        Request2(int& client, GlobalConfig *config) {
            this->client = client;
            this->config = config;
            // file = ".data" + std::to_string(client);
            //file = "video.mp4";
            //ofstream bodyFile(file, ios::out | ios::trunc);
            //bodyFile.open(file, ios::out | ios::trunc);
            //bodyFile.close();
        }

        string getrequest() {
            return request;
        }

        void setrequest(const string req) {
            request = req;
        }

        bool containsCRLF(const char* str) {
            return strstr(str, "\r\n") != nullptr;
        }

        std::string generateUniqueFilename() {
            static int counter = 0; // Static variable to keep track of the counter
            std::time_t currentTime = std::time(nullptr);

            std::ostringstream filenameStream;
            filenameStream << "file_" << currentTime << "_" << counter;
            counter++;

            return filenameStream.str();
        }

        string getMimeType() {
            std::map<std::string, std::string> mimeTypes = getMimeTypes();
            string extension;

            if (headers.find("Content-Type") != headers.end()) {
                extension = headers["Content-Type"];
            }
            if (mimeTypes.find(extension) != mimeTypes.end()) {
                return mimeTypes[extension];
            }
            return "";
        }

        void handleBody(const char* buffer, size_t bufferLength, size_t pos) {
            if (chunked) {
                if (fileCreated == false) {
                    file = generateUniqueFilename() + getMimeType();
                    file = getPath() + "/" + file;
                    fileCreated = true;
                    bodyFile.open(file, ios::out | ios::trunc);
                    bodyFile.close();
                }
                bodyFile.open(file, ios::out | ios::app);
                //size_t i = pos; // the fault is here, somehow pos is 9 instead of 0????
                if (chunkState == CHUNK_END) {i = pos; chunkState = CHUNK_SIZE;}
                else i = 0;

                while (i < bufferLength) {
                    if (chunkState == CHUNK_SIZE) {
                        if (buffer[i] == '\r') {
                            // Skip '\r'.
                            i++;
                            continue;
                        } else if (buffer[i] == '\n') {
                            // Process chunk size after reaching '\n'.
                            if (!chunkSizeStr.empty()) {
                                chunkSize = std::stol(chunkSizeStr, nullptr, 16);
                                chunkSizeStr.clear();
                                if (chunkSize == 0) {
                                    // Handle end of all chunks.
                                    state = REQUEST_LINE;
                                    fileCreated = false;
                                    chunkState = CHUNK_END;
                                    bodyFile.close();
                                    return;
                                }
                                chunkState = CHUNK_DATA;
                            }
                            i++; // Move past '\n'.
                            continue;
                        } else {
                            // Accumulate hex digits for chunk size.
                            chunkSizeStr += buffer[i];
                        }
                    } else if (chunkState == CHUNK_DATA) {
                        size_t remainingDataInBuffer = bufferLength - i;
                        size_t dataToWrite = std::min(static_cast<size_t>(chunkSize), remainingDataInBuffer);

                        // Write the chunk data to the file.
                        bodyFile.write(buffer + i, dataToWrite);
                        chunkSize -= dataToWrite;
                        i += dataToWrite;
                        bodyFile.flush();

                        // Check if the end of the chunk was reached.
                        if (chunkSize == 0) {
                            // Expect the next chunk size after an additional CRLF.
                            if (buffer[i] == '\n' && lastchar == '\r') {
                                chunkState = CHUNK_SIZE; // Prepare for the next chunk.
                                lastchar = 0;
                                i++; // Skip the '\n' following '\r'.
                                continue;
                            }
                            lastchar = buffer[i];
                        }
                        else i--; // Adjust because of i++ in the loop.
                    }
                    i++;
                }
                bodyFile.close();
            }

            else if (content_length > 0 && !chunked) {
                if (fileCreated == false) {
                    file = generateUniqueFilename() + getMimeType();
                    file = getPath() + "/" + file;
                    fileCreated = true;
                    bodyFile.open(file, ios::out | ios::trunc);
                    bodyFile.close();
                }
                // Non-chunked transfer, content_length must be set beforehand
                if (bodyFile) {
                    size_t writeSize = std::min(static_cast<size_t>(content_length), bufferLength - pos);
                    //cout << "\ncontent_length = " << content_length << " -pos = " << pos << " -bufferLength = " << bufferLength << " -writsize = " << writeSize << " -BufferStart = " << buffer[0] <<  endl;                    
                    bodyFile.open(file, ios::out | ios::app);
                    bodyFile.write(buffer + pos, writeSize);
                    
                    bodyFile.flush();
                    bodyFile.close();
                    content_length -= writeSize;
                }
                //bodyFile.close();
            }
            if (content_length <= 0 && !chunked) {
                // End of message
                usleep(100);
                state = REQUEST_LINE;
                fileCreated = false;
            }
        }

        void parse(int bufferlength) {
            std::string request;
            request.append(buffer, bufferlength);
            std::istringstream requestStream(request);
            std::string line;
            header_length = 0;
            if (state == REQUEST_LINE) {
                cout << "what the fuck am i doing here" << endl;
                std::getline(requestStream, line);
                std::istringstream lineStream(line);
                lineStream >> method >> path >> version;
                cout << "method = " << method << endl << "path = " << path << endl  << "version = " << version << endl;
                state = HEADERS;
                header_length = line.length() + 1;
                chunkState = CHUNK_END;
            }
            while (state == HEADERS && std::getline(requestStream, line) ) {
                if (line[0] == '\r') {
                    headersComplete = true;
                    requestPath = getPath();
                    state = BODY;
                    break;
                }
                if (line.back() == '\r') {
                    //line.pop_back();
                    if (lineComplete) {
                        std::string::size_type pos = line.find(':');
                        header = line.substr(0, pos);
                        headers[header] = line.substr(pos + 2);
                        headers[header].pop_back();
                    }
                    else {
                        std::string::size_type pos = line.find('\r');
                        headers[header] += line.substr(0, pos);
                        if (headers[header].back() == '\r') {
                            headers[header].pop_back();
                        }
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
                }
                //cout << "header => " << header << endl;
                if (header == "Transfer-Encoding" && headers[header] == "chunked") {
                    chunked = true;
                }
                if ( header == "Content-Length" ) { ////////////// it doesnt make it in here????
                    content_length = stol(headers[header]);
                    cout << "content_length = " << content_length << endl;
                }
                header_length += line.length() + 1;
                usleep(100);
            }
            int bodyStart = request.find("\r\n\r\n");
            if (bodyStart == -1) {
                bodyStart = 0;
            }
            else {
                if (chunked && chunkState == CHUNK_END) {
                    bodyStart += 2;
                }
                else if (!chunked)
                    bodyStart += 4;
                //cout << "bodyStart = " << bodyStart << endl;
            }

            if (state == BODY) {
                handleBody(buffer, bufferlength, bodyStart);
            }
        }

        // Function to get the Path from the config file
        string getPath() {
            string root;
            string host = getHost();
           
            if (config->servers.find(host) != config->servers.end()) {
                if (config->servers[host].locations.find(this->path) != config->servers[host].locations.end()) {
                    return config->servers[host].locations[path].config["root"];
                }
                else {
                    return config->servers[host].config["root"];
                }
            }
            else {
                //return config->getGlobalConfig().at("root");
                if (config->getGlobalConfig().find("root") != config->getGlobalConfig().end()) {
                    return config->getGlobalConfig().at("root");
                }
                else {
                    return "";
                }
            } 
        }

            // Function to print all headers
        void printHeaders() const {
            std::cout << "Headers:" << std::endl;
            for (const auto& header : headers) {
                std::cout << header.first << ">> " << header.second << std::endl;
            }
        }

        string getHost() {
            istringstream ss (headers["Host"]);
            return (getline(ss, path, ':'), path);
        }

        // Function to get the MIME type of a file
        std::map<std::string, std::string> getMimeTypes() {
            std::map<std::string, std::string> mimeTypes = {
                {"application/atom+xml", ".atom"},
                {"application/ecmascript", ".ecma"},
                {"application/json", ".json"},
                {"application/octet-stream", ".bin"},
                {"application/pdf", ".pdf"},
                {"application/xhtml+xml", ".xhtml"},
                {"application/xml", ".xml"},
                {"application/zip", ".zip"},
                {"audio/mpeg", ".mp3"},
                {"audio/ogg", ".ogg"},
                {"image/gif", ".gif"},
                {"image/jpeg", ".jpeg"},
                {"image/png", ".png"},
                {"text/css", ".css"},
                {"text/csv", ".csv"},
                {"text/html", ".html"},
                {"text/plain", ".txt"},
                {"text/xml", ".xml"},
                {"video/mp4", ".mp4"},
                {"video/mpeg", ".mpeg"},
                {"video/ogg", ".ogv"},
                {"video/quicktime", ".mov"},
                {"video/webm", ".webm"},
                {"video/x-ms-wmv", ".wmv"},
                {"video/x-flv", ".flv"},
                {"video/3gpp", ".3gp"},
                {"video/3gpp2", ".3g2"},
                {"video/avi", ".avi"},
                {"video/msvideo", ".avi"},
                {"video/x-msvideo", ".avi"},
                {"video/x-matroska", ".mkv"},
                {"video/x-m4v", ".m4v"},
                {"video/x-ms-asf", ".asf"},
                {"video/x-ms-wvx", ".wvx"},
                {"video/x-ms-wmx", ".wmx"},
                {"video/x-ms-wm", ".wm"},
                {"video/x-ms-wmp", ".wmp"},
                {"video/x-ms-wmv", ".wmv"},
                {"video/x-ms-wvx", ".wvx"},
                {"video/x-ms-wmx", ".wmx"},
                {"video/x-ms-wm", ".wm"},
                {"video/x-ms-wmp", ".wmp"},
                {"video/x-ms-wmv", ".wmv"},
                {"video/x-ms-wvx", ".wvx"},
                {"video/x-ms-wmx", ".wmx"},
                {"video/x-ms-wm", ".wm"},
                {"video/x-ms-wmp", ".wmp"},
                {"video/x-ms-wmv", ".wmv"},
                {"video/x-ms-wvx", ".wvx"},
                {"video/x-ms-wmx", ".wmx"},
                {"video/x-ms-wm", ".wm"},
                {"video/x-ms-wmp", ".wmp"},
                {"video/x-ms-wmv", ".wmv"},
                {"video/x-ms-wvx", ".wvx"},
                {"video/x-ms-wmx", ".wmx"},
                {"video/x-ms-wm", ".wm"},
                {"video/x-ms-wmp", ".wmp"},
                {"video/x-ms-wmv", ".wmv"},
                {"video/x-ms-wvx", ".wvx"},
                {"video/x-ms-wmx", ".wmx"},
                {"video/x-ms-wm", ".wm"},
                {"video/x-ms-wmp", ".wmp"},
                {"video/x-ms-wmv", ".wmv"},
                {"video/x-ms-wvx", ".wvx"},
                {"video/x-ms-wmx", ".wmx"},
                {"application/octet-stream", ""}
                // Add more MIME types here
            };
            return mimeTypes;
        }

        ~Request2() {
            bodyFile.close();
        }

};

#endif