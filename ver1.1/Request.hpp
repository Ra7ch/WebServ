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
        long content_length = LONG_MAX;
        int header_length = 0;
        ofstream bodyFile;
        long chunkSize = 0;
        string chunkSizeStr;
        ChunkState chunkState = CHUNK_SIZE;
        char lastchar = 0;
        int limit = 0;
        size_t i;
        std::string request;



        Request2(int& client) {
            this->client = client;
            // file = ".data" + std::to_string(client);
            file = "video.mp4";
            //ofstream bodyFile(file, ios::out | ios::trunc);
            bodyFile.open(file, ios::out | ios::trunc);
            bodyFile.close();
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

        void handleBody(const char* buffer, size_t bufferLength, size_t pos) {
            // if (chunked) {
            //     bodyFile.open(file, ios::out | ios::app);
            //     int i = pos;
            //     while (i < bufferLength) {
            //         if (buffer[i] == '\r' && chunkSize == 0 && limit == 0)
            //         {
            //             limit++;
            //             i++;
            //             continue;
            //         }
            //         if (buffer[i] == '\n' && limit == 1 && chunkSize == 0)
            //         {
            //             limit++;
            //             i++;
            //             continue;
            //         }
            //         if (limit == 2) {
            //             if (chunkState == CHUNK_END || chunkState == CHUNK_DATA) {
            //                 chunkState = CHUNK_SIZE;
            //                 limit = 0;
            //                 // if (chunkState == CHUNK_DATA)
            //                 //     limit == 0;
            //             }
            //             else if (chunkState == CHUNK_SIZE) {
            //                 limit = 0;
            //                 chunkState = CHUNK_DATA;
            //                 chunkSize = stol(chunkSizeStr, 0, 16);
            //                 //cout << "chunkSize = " << chunkSize << endl;
            //                 chunkSizeStr = "";
            //                 if (chunkSize == 0) {
            //                     state = REQUEST_LINE;
            //                     chunkState = CHUNK_END;
            //                     bodyFile.close();
            //                     return;
            //                 }
            //             }
            //         }
            //         if (chunkState == CHUNK_SIZE) {
            //             chunkSizeStr += buffer[i];
            //             //cout << "bufferi[i] = " << (int)buffer[i] << " -i = " << i << endl;
            //         }
            //         if (chunkState == CHUNK_DATA) {
            //             bodyFile.write(buffer + i, 1);
            //             chunkSize--;
            //         }
            //         i++;
            //         limit = 0;
            //     }
            //     bodyFile.flush();
            //     bodyFile.close();
                
            // } 
if (chunked) {
    bodyFile.open(file, ios::out | ios::app);
    //size_t i = pos; // the fault is here, somehow pos is 9 instead of 0????
    if (chunkState == CHUNK_END) {i = pos; chunkState = CHUNK_SIZE;}
    else i = 0;

    while (i < bufferLength) {
        if (chunkState == CHUNK_SIZE) {
           // cout << "buffer[i] = " << (int)buffer[i] << " -i = " << i << endl;
            if (buffer[i] == '\r') {
                // Skip '\r'.
                i++;
                continue;
            } else if (buffer[i] == '\n') {
                // Process chunk size after reaching '\n'.
                if (!chunkSizeStr.empty()) {
               //     cout << "chunkSizeStr = " << chunkSizeStr << endl;
                    chunkSize = std::stol(chunkSizeStr, nullptr, 16);
                    chunkSizeStr.clear();
                    if (chunkSize == 0) {
                        // Handle end of all chunks.
                        state = REQUEST_LINE;
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
           // cout << "dataToWrite = " << dataToWrite << " remainingDataInBuffer = " << remainingDataInBuffer << endl;

            chunkSize -= dataToWrite;
            i += dataToWrite;
            bodyFile.flush();

            // Check if the end of the chunk was reached.
            if (chunkSize == 0) {
                // Expect the next chunk size after an additional CRLF.
              //  cout << "im in down : buffer[i] = " << (int)buffer[i] << " -i = " << i << endl;
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
                // Non-chunked transfer, content_length must be set beforehand
                //std::ofstream bodyFile(file, std::ios::app);
                //if (content_length < 5000)
                //    getchar();
                if (bodyFile) {
                    size_t writeSize = std::min(static_cast<size_t>(content_length), bufferLength - pos);
                    //cout << "\ncontent_length = " << content_length << " -pos = " << pos << " -bufferLength = " << bufferLength << " -writsize = " << writeSize << " -BufferStart = " << buffer[0] <<  endl;
                    //cout << "\ncontent_length = " << content_length << " -pos = " << pos << " -bufferLength = " << bufferLength << " -writsize = " << writeSize << " -BufferStart = " << buffer[0] <<  endl;
                    
                    bodyFile.open(file, ios::out | ios::app);
                    bodyFile.write(buffer + pos, writeSize);
                    
                    bodyFile.flush();
                    bodyFile.close();
                    content_length -= writeSize;
                    //cout << "content_length = " << content_length << endl; 
                }
                //bodyFile.close();
            }
            if (content_length <= 0 && !chunked) {
                // End of message
                //cout << "content_length =<< " << content_length << endl;
                usleep(100);
                state = REQUEST_LINE;
            }
        }

        void parse(int bufferlength) {

            std::string request;
            request.append(buffer, bufferlength);
            std::istringstream requestStream(request);
            std::string line;
            header_length = 0;
            //cout << "request: " << request << endl;
            if (state == REQUEST_LINE) {
                // while (1);
                //cout << "request = " <<  buffer << endl;
                //cout.write(buffer, BUFFER_SIZE);
                cout << "what the fuck am i doing here" << endl;
                std::getline(requestStream, line);
                std::istringstream lineStream(line);
                lineStream >> method >> path >> version;
                cout << "method = " << method << endl << "path = " << path << endl  << "version = " << version << endl;
                state = HEADERS;
                header_length = line.length() + 1;
                chunkState = CHUNK_END;
                //cout << "Method line = " << line << endl; ///////////
            }
            while (state == HEADERS && std::getline(requestStream, line) ) {
               // cout << "Headers line = " << line << endl; ///////////
                //cout << "line = " << line << " \n>1st ch = " << (int)line[0] << endl;
                if (line[0] == '\r') {
                    headersComplete = true;
                    state = BODY;
                   // cout << "headersComplete = " << headersComplete << endl;
                    break;
                }
                //cout << "2nd ch = " << (int)line[1] << endl;
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

            if (state == BODY)
                handleBody(buffer, bufferlength, bodyStart);
            // if (chunked && state == BODY) {
            //     buffer  = buffer + bodyStart;
            //     if (bodyComplete) {
            //         string chunk;
            //         getline(requestStream, chunk);
            //         int chunkSize = stoi(chunk, 0, 16);
            //         if (chunkSize == 0) {
            //             state = REQUEST_LINE;
            //             bodyComplete = false;
            //             return ;
            //         }
            //         bodyComplete = false;
            //     }
            //     ofstream bodyFile(file, ios::out | ios::app);
            //     while (getline(requestStream, body)) {
            //         if (body == "\r") {
            //             bodyComplete = true;
            //             break;
            //         }
            //         if (!requestStream.eof()) {
            //             // If we haven't reached the end of the stream, append '\n'
            //             body += '\n';
            //         }
            //         bodyFile << body;
            //     }
            //     bodyFile.close();
            // }
            // else if (!chunked && state == BODY) {
            //     buffer  = buffer + bodyStart;
            //     int size  = BUFFER_SIZE - bodyStart;
               
            //     ofstream bodyFile(file, ios::out | ios::app);

                
            //         const int chunk_size = std::min(content_length, BUFFER_SIZE);
                   
            //         cout << "size = " << size << endl;
                
            //         bodyFile.write(buffer, size);
            //         content_length -= size;

            //         if (content_length <= 0) {
            //             bodyComplete = false;
            //             state = REQUEST_LINE;
            //             //break;
            //         }
            //     bodyFile.close();
            // }
        }

            // Function to print all headers
        void printHeaders() const {
            std::cout << "Headers:" << std::endl;
            for (const auto& header : headers) {
                std::cout << header.first << ">> " << header.second << std::endl;
            }
        }

        string getHost() {
            return headers["Host"];
        }

        ~Request2() {
            bodyFile.close();
        }

};

#endif