#include "BusManager.h"

void BusManager::AddBus(const std::string& bus, const std::vector<std::string>& stops) {
    buses_to_stops[bus] = stops;
    for (const auto& stop : stops) {
        stops_to_buses[stop].push_back(bus);
    }
}

BusManager::BusesForStopResponse BusManager::GetBusesForStop(const std::string& stop) const {
    if (stops_to_buses.count(stop)) {
        return {stops_to_buses.at(stop)};
    } else {
        return {};
    }
}

BusManager::StopsForBusResponse BusManager::GetStopsForBus(const std::string& bus) const {
    if (buses_to_stops.count(bus)) {
        StopsForBusResponse response;
        for (const auto& stop : buses_to_stops.at(bus)) {
            std::vector<std::string> buses_for_stop;
            for (const auto& other_bus : stops_to_buses.at(stop)) {
                if (bus != other_bus) {
                    buses_for_stop.push_back(other_bus);
                }
            }
            response.stops.push_back({stop, buses_for_stop});
        }
        return response;
    } else {
        return {};
    }
}

BusManager::AllBusesResponse BusManager::GetAllBuses() const {
    return {buses_to_stops};
}

std::istream& operator>>(std::istream& is, Query& q) {
    is >> q.type;
    if (q.type == "NEW_BUS") {
        is >> q.bus >> q.stop_count;
        q.stops.resize(q.stop_count);
        for (auto& stop : q.stops) {
            is >> stop;
        }
    } else if (q.type == "BUSES_FOR_STOP") {
        is >> q.stop;
    } else if (q.type == "STOPS_FOR_BUS") {
        is >> q.bus;
    } else if (q.type == "ALL_BUSES") {
        // не нуждается в дополнительных инпутах
    }
    return is;
}

std::ostream& operator<<(std::ostream& os, const BusManager::BusesForStopResponse& r) {
    if (r.buses.empty()) {
        os << "No stop";
    } else {
        for (const auto& bus : r.buses) {
            os << bus << " ";
        }
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const BusManager::StopsForBusResponse& r) {
    if (r.stops.empty()) {
        os << "No bus";
    } else {
        for (const auto& stop_and_buses : r.stops) {
            os << "Stop " << stop_and_buses.first << ":";
            if (stop_and_buses.second.empty()) {
                os << " no interchange";
            } else {
                for (const auto& bus : stop_and_buses.second) {
                    os << " " << bus;
                }
            }
            os << std::endl;
        }
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const BusManager::AllBusesResponse& r) {
    if (r.buses_to_stops.empty()) {
        os << "No buses";
    } else {
        for (const auto& bus_and_stops : r.buses_to_stops) {
            os << "Bus " << bus_and_stops.first << ":";
            for (const auto& stop : bus_and_stops.second) {
                os << " " << stop;
            }
            os << std::endl;
        }
    }
    return os;
}
