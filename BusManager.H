#ifndef BUS_MANAGER_H
#define BUS_MANAGER_H

#include <map>
#include <string>
#include <vector>
#include <iostream>

struct Query {
    std::string type;
    std::string bus;
    std::string stop;
    int stop_count;
    std::vector<std::string> stops;
};

class BusManager {
public:
    void AddBus(const std::string& bus, const std::vector<std::string>& stops);

    struct BusesForStopResponse {
        std::vector<std::string> buses;
    };

    BusesForStopResponse GetBusesForStop(const std::string& stop) const;

    struct StopsForBusResponse {
        std::vector<std::pair<std::string, std::vector<std::string>>> stops;
    };

    StopsForBusResponse GetStopsForBus(const std::string& bus) const;

    struct AllBusesResponse {
        std::map<std::string, std::vector<std::string>> buses_to_stops;
    };

    AllBusesResponse GetAllBuses() const;

private:
    std::map<std::string, std::vector<std::string>> buses_to_stops;
    std::map<std::string, std::vector<std::string>> stops_to_buses;
};

std::istream& operator>>(std::istream& is, Query& q);

std::ostream& operator<<(std::ostream& os, const BusManager::BusesForStopResponse& r);

std::ostream& operator<<(std::ostream& os, const BusManager::StopsForBusResponse& r);

std::ostream& operator<<(std::ostream& os, const BusManager::AllBusesResponse& r);

#endif // BUS_MANAGER_H
