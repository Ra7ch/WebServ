#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h> // Include epoll header
#include <cstring>
#include <sys/stat.h>
#include <climits>
#include <fcntl.h>
#include <boost/asio.hpp>
#include "config.hpp"

using namespace std;

const int MAX_CLIENTS = 5;
const int BUFFER_SIZE = 1024;

#include "Request.hpp"

#endif