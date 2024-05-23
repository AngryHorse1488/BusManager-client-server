#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>
#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

const int PORT = 8888;
const int BUFFER_SIZE = 1024;
const std::string LOG_FILE = "log.txt";

class BusManager {
public:
    void ReadDataFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (file.is_open()) {
            std::string line;
            while (std::getline(file, line)) {
                std::istringstream iss(line);
                std::string command;
                iss >> command;
                if (command == "NEW_BUS") {
                    std::string bus;
                    int stop_count;
                    iss >> bus >> stop_count;
                    std::vector<std::string> stops(stop_count);
                    for (std::string& stop : stops) {
                        iss >> stop;
                    }
                    AddBus(bus, stops);
                }
            }
            file.close();
        } else {
            std::ofstream new_file(filename); // Создание файла, если он не существует
            if (!new_file.is_open()) {
                std::cerr << "Failed to create data file: " << filename << std::endl;
            } else {
                std::cout << "Data file created: " << filename << std::endl;
                new_file.close();
            }
        }
    }

    void SaveDataToFile(const std::string& filename) const {
        std::ofstream file(filename, std::ios::trunc);
        if (file.is_open()) {
            for (const auto& bus_pair : buses_to_stops) {
                const std::string& bus = bus_pair.first;
                const std::vector<std::string>& stops = bus_pair.second;
                file << "NEW_BUS " << bus << " " << stops.size();
                for (const std::string& stop : stops) {
                    file << " " << stop;
                }
                file << "\n";
            }
            file.close();
        } else {
            std::cerr << "Unable to open file: " << filename << std::endl;
        }
    }

    std::vector<std::string> GetBusesForStop(const std::string& stop) const {
        if (stops_to_buses.find(stop) != stops_to_buses.end()) {
            return stops_to_buses.at(stop);
        }
        return {};
    }

    std::vector<std::string> GetStopsForBus(const std::string& bus) const {
        if (buses_to_stops.find(bus) != buses_to_stops.end()) {
            return buses_to_stops.at(bus);
        }
        return {};
    }

    std::vector<std::string> GetAllBuses() const {
        std::vector<std::string> all_buses;
        for (const auto& bus_pair : buses_to_stops) {
            all_buses.push_back(bus_pair.first);
        }
        return all_buses;
    }

    void AddBus(const std::string& bus, const std::vector<std::string>& stops) {
        buses_to_stops[bus] = stops;
        for (const std::string& stop : stops) {
            stops_to_buses[stop].push_back(bus);
        }
    }

private:
    std::map<std::string, std::vector<std::string>> buses_to_stops;
    std::map<std::string, std::vector<std::string>> stops_to_buses;
};

void LogMessage(const std::string& message) {
    std::ofstream log_file(LOG_FILE, std::ios::app);
    if (log_file.is_open()) {
        log_file << message << std::endl;
        log_file.close();
    } else {
        std::cerr << "Unable to open log file: " << LOG_FILE << std::endl;
    }
}

void PrintCommand(const std::string& command) {
    std::string log_message = "Received command: " + command;
    std::cout << log_message << std::endl;
    LogMessage(log_message);
}

void PrintDataSaved() {
    std::string log_message = "Data saved to file.";
    std::cout << log_message << std::endl;
    LogMessage(log_message);
}

std::string HandleHelpCommand() {
    std::stringstream ss;
    ss << "Available commands:\n";
    ss << "HELP - Show this help message.\n";
    ss << "BUSES_FOR_STOP <stop_name> - Show buses passing through the specified stop.\n";
    ss << "STOPS_FOR_BUS <bus_number> - Show stops for the specified bus along with transfers.\n";
    ss << "ALL_BUSES - Show all buses and their stops.\n";
    ss << "NEW_BUS <bus_number> <stop_count> <stop1> <stop2> ... <stopN> - Add a new bus with the specified stops.\n";
    ss << "SHUTDOWN - Shutdown the server.\n";
    return ss.str();
}

int main() {
    // Инициализация Winsock
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << "WSAStartup failed: " << iResult << std::endl;
        return 1;
    }

    // Создание сокета
    SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == INVALID_SOCKET) {
        std::cerr << "Failed to create socket: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Указание адреса сервера
    sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    // Привязка сокета к адресу и порту
    if (bind(sockfd, (sockaddr*)&servaddr, sizeof(servaddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    std::cout << "Server is running on port " << PORT << std::endl;

    sockaddr_in cliaddr;
    int len = sizeof(cliaddr);
    char buffer[BUFFER_SIZE];
    BusManager busManager;
    busManager.ReadDataFromFile("data.txt");

    while (true) {
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (sockaddr*)&cliaddr, &len);
        buffer[n] = '\0';
        std::string command(buffer);

        PrintCommand(command);

        std::istringstream iss(command);
        std::string action;
        iss >> action;

        std::string response;
        if (action == "BUSES_FOR_STOP") {
            std::string stop;
            iss >> stop;
            auto buses = busManager.GetBusesForStop(stop);
            if (buses.empty()) {
                response = "No stop";
            } else {
                for (const auto& bus : buses) {
                    response += bus + " ";
                }
            }
        } else if (action == "STOPS_FOR_BUS") {
            std::string bus;
            iss >> bus;
            auto stops = busManager.GetStopsForBus(bus);
            if (stops.empty()) {
                response = "No bus";
            } else {
                for (const auto& stop : stops) {
                    response += "Stop " + stop + ": ";
                    auto buses_for_stop = busManager.GetBusesForStop(stop);
                    if (buses_for_stop.size() == 1) {
                        response += "no interchange";
                    } else {
                        for (const auto& other_bus : buses_for_stop) {
                            if (bus != other_bus) {
                                response += other_bus + " ";
                            }
                        }
                    }
                    response += "\n";
                }
            }
        } else if (action == "ALL_BUSES") {
            auto buses = busManager.GetAllBuses();
            if (buses.empty()) {
                response = "No buses";
            } else {
                for (const auto& bus : buses) {
                    response += "Bus " + bus + ": ";
                    auto stops = busManager.GetStopsForBus(bus);
                    for (const auto& stop : stops) {
                        response += stop + " ";
                    }
                    response += "\n";
                }
            }
        } else if (action == "NEW_BUS") {
            std::string bus;
            int stop_count;
            iss >> bus >> stop_count;
            std::vector<std::string> stops(stop_count);
            for (std::string& stop : stops) {
                iss >> stop;
            }
            busManager.AddBus(bus, stops);
            busManager.SaveDataToFile("data.txt");
            PrintDataSaved();
            response = "Bus added";
        } else if (action == "HELP") {
            response = HandleHelpCommand();
        } else if (action == "SHUTDOWN") {
            break;
        } else {
            response = "Unknown command";
        }

        LogMessage("Sent response: " + response);
        sendto(sockfd, response.c_str(), response.size(), 0, (sockaddr*)&cliaddr, len);
    }

    closesocket(sockfd);
    WSACleanup();
    return 0;
}
