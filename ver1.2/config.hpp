#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "webServ.hpp"
using namespace std;

class locationConfig {
    public :
        std::map < std::string, std::string > config;
};

class serverConfig {
    public :
        std::map < std::string, std::string > config;
        std::map < std::string, locationConfig > locations;
};

class GlobalConfig {
    private :
        std::map <std::string, std::string> config;
        std::map < int, std::string > serverNames;
        std::string currentLocation;
        std::string currentServer;

    public :
        std::map < std::string, serverConfig > servers;

        // Trim leading and trailing whitespaces from a string
        std::string trim(const std::string& str) {
            // Find the first non-whitespace character
            size_t start = str.find_first_not_of(" \t");
            if (start == std::string::npos) {
                // If the string is all whitespaces, return an empty string
                return "";
            }

            // Find the last non-whitespace character
            size_t end = str.find_last_not_of(" \t");
            
            // Return the substring containing non-whitespace characters
            return str.substr(start, end - start + 1);
        }

        GlobalConfig(std::string File) {
            std::ifstream configFile(File);
            std::string line;

            if (!configFile.is_open()) {
                // I ll throw an exception here/////////////
                throw std::runtime_error("Error: Unable to open configuration file: ");
                // std::cerr << "Error: Unable to open configuration file: " << filename << std::endl;
                // return ;
            }
            while (std::getline(configFile, line)) {
                std::istringstream iss(line);
                std::string key, value;

                if (line.empty() || line[0] == ';') {
                    // Skip empty lines and comments (lines starting with ';' are comment lines)
                    continue;
                }
                else if (line[0] == '[' && line[line.length() - 1] == ']') {
                    // New block
                    std::string section = line.substr(1, line.length() - 2);
                    if (section.find("server ") == 0) {
                        // Server block
                        currentServer = section.substr(7);  // Remove "server " prefix
                        currentLocation = "";
                        //cout << "1- " << currentServer << endl;
                    } else if (section.find("location ") == 0) {
                        // Location block
                        currentLocation = section.substr(9);  // Remove "location " prefix
                    }
                }
                else if (std::getline(iss, key, '=') && std::getline(iss, value)) {
                    key = trim(key);
                    value = trim(value);
                    //cout << "key: " << key << " value: " << value << endl;

                    if (currentServer.empty()) {
                        // Global configuration
                        config[key] = value;
                    } else if (currentLocation.empty()) {
                        // Server-wide configuration
                        servers[currentServer].config[key] = value;
                            //cout << "3- " << value << endl;
                        if (key == "port") {
                            serverNames[std::stoi(value)] = currentServer;
                        }
                    } else {
                        // Location-specific configuration
                        servers[currentServer].locations[currentLocation].config[key] = value;
                    }
                }
            }
            configFile.close();
        }

        // Some getters
        const std::map<std::string, std::string>& getGlobalConfig() const {
            return config;
        }

        const std::map<std::string, serverConfig>& getServerConfigs() const {
            return servers;
        }

        const std::map<int, std::string>& getServerNames() const {
            return serverNames;
        }

        const std::vector <int> getPorts() {
            std::vector <int> ports;
            for (auto it = serverNames.begin(); it != serverNames.end(); it++) {
                ports.push_back(it->first);
                //cout << "0- " << it->first << endl;
            }
            return ports;
        }

        const string getHosts() {
            return config["hosts"];
        }

};

#endif