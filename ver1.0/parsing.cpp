#include "webServ.hpp"

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
        std::map < std::string, serverConfig > servers;
        std::string currentLocation;
        std::string currentServer;

    public :
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
                    } else if (section.find("location ") == 0) {
                        // Location block
                        currentLocation = section.substr(9);  // Remove "location " prefix
                    }
                }
                else if (std::getline(iss, key, '=') && std::getline(iss, value)) {
                    if (currentServer.empty()) {
                        // Global configuration
                        config[key] = value;
                    } else if (currentLocation.empty()) {
                        // Server-wide configuration
                        servers[currentServer].config[key] = value;
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

};
int main (int argc, char **argv) {
    if (argc != 2) {std::cout << "Check your input again..." << std::endl; return 1;}

    try {
        GlobalConfig config (argv[1]);
        std::string str = "log_level";
        const std::map<std::string, std::string>& configMap = config.getGlobalConfig();

        try {
            std::cout << configMap.at("log_level ") << std::endl;
        } catch (const std::out_of_range& e) {
            std::cout << "Key not found: " << str << std::endl;
        }

        ///////////////////////////////////
        
                // Example: Access and print global configuration
        // const std::map<std::string, std::string>& globalConfig = config.getGlobalConfig();
        // std::cout << "Global Configuration:\n";
        // for (const auto& entry : globalConfig) {
        //     std::cout << entry.first << " = " << entry.second << "\n";
        // }

        // // Example: Access and print server configurations
        // const std::map<std::string, serverConfig>& serverConfigs = config.getServerConfigs();
        // std::cout << "\nServer Configurations:\n";
        // for (const auto& server : serverConfigs) {
        //     std::cout << "Server Name: " << server.first << "\n";

        //     // Example: Access and print server-wide configuration
        //     const std::map<std::string, std::string>& serverWideConfig = server.second.config;
        //     std::cout << "Server-wide Configuration:\n";
        //     for (const auto& entry : serverWideConfig) {
        //         std::cout << entry.first << " = " << entry.second << "\n";
        //     }

        //     // Example: Access and print location configurations
        //     const std::map<std::string, locationConfig>& locationConfigs = server.second.locations;
        //     for (const auto& location : locationConfigs) {
        //         std::cout << "Location Path: " << location.first << "\n";

        //         // Example: Access and print location-specific configuration
        //         const std::map<std::string, std::string>& locationSpecificConfig = location.second.config;
        //         std::cout << "Location-specific Configuration:\n";
        //         for (const auto& entry : locationSpecificConfig) {
        //             std::cout << entry.first << " = " << entry.second << "\n";
        //         }
        //     }

        //     std::cout << std::endl;
        // }
        //////////////////////////////////////////////

    } catch(std::exception const& e) {
        std::cout << e.what() << std::endl;
        return 1;
    } 

}