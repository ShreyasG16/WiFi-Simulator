#include <iostream>
#include <vector>
#include <memory>
#include <algorithm>
#include <numeric>
#include <random>
#include <cmath>
#include <thread>
#include <mutex>

namespace Constants {
    const double PACKET_SIZE_BYTES = 1500; 
    const double BANDWIDTH_MHZ = 80.0;      
    const double DIFS_US = 34.0;            
}

std::mutex dataMutex;  

template <typename T>
T validateInput(T value, T min, T max, const std::string& errorMessage) {
    if (value < min || value > max) {
        throw std::out_of_range(errorMessage);
    }
    return value;
}

class User {
public:
    User(int id, double signalStrength) : id(id), signalStrength(signalStrength) {}

    virtual bool transmit(int channel) {
        return rand() % 10 < 8; 
    }

    virtual ~User() = default;

protected:
    int id;
    double signalStrength;
};


class MIMOUser : public User {
public:
    MIMOUser(int id, double signalStrength) : User(id, signalStrength) {}
};


class WiFi {
public:
    virtual void simulate(int numUsers, double duration) = 0;
    virtual double calculateThroughput() const = 0;
    virtual double getAverageLatency() const = 0;
    virtual double getMaxLatency() const = 0;
    virtual ~WiFi() = default;
};


class WiFi5 : public WiFi {
public:
    WiFi5() : totalThroughput(0.0) {}

    void simulate(int numUsers, double duration) override {
        std::vector<std::shared_ptr<User>> users;
        for (int i = 0; i < numUsers; ++i) {
            users.push_back(std::make_shared<MIMOUser>(i, 20.0 + (rand() % 20)));
        }

        double currentTime = 0;
        double totalDataTransmitted = 0;
        double wifi5ThroughputMultiplier = 1.5;
        int roundRobinIndex = 0;
        std::vector<std::thread> threads;
        int maxThreads = 10;

        while (currentTime < duration) {
            threads.emplace_back([this, &users, roundRobinIndex, &currentTime, &totalDataTransmitted, wifi5ThroughputMultiplier, numUsers]() {
                if (users[roundRobinIndex]->transmit(0)) {
                    double txTime = Constants::PACKET_SIZE_BYTES * 8.0 / (Constants::BANDWIDTH_MHZ * 1e6);
                    txTime *= (1.0 + 0.01 * numUsers) * (1.0 / wifi5ThroughputMultiplier);
                    double latency = std::max(5.0, txTime * 1000 + (0.1 * (rand() % 30)) + (2.0 + 0.5 * (rand() % 10)));

                    std::lock_guard<std::mutex> lock(dataMutex);
                    latencies.push_back(latency);
                    totalDataTransmitted += Constants::PACKET_SIZE_BYTES * 8.0;
                    currentTime += txTime + Constants::DIFS_US * 1e-6;
                }
            });

            roundRobinIndex = (roundRobinIndex + 1) % numUsers;

            if (threads.size() >= maxThreads) {
                for (auto &t : threads) t.join();
                threads.clear();
            }
        }

        for (auto &t : threads) t.join();

        double throughputPenalty = std::max(0.5, 1.0 - (0.01 * numUsers));
        totalThroughput = totalDataTransmitted * wifi5ThroughputMultiplier * throughputPenalty / duration;
    }

    double calculateThroughput() const override {
        return totalThroughput / 1e6;
    }

    double getAverageLatency() const override {
        if (latencies.empty()) return 0.0;
        return std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
    }

    double getMaxLatency() const override {
        if (latencies.empty()) return 0.0;
        return *std::max_element(latencies.begin(), latencies.end());
    }

private:
    double totalThroughput;
    std::vector<double> latencies;
};


void runSimulation(int numUsers, double duration) {
    WiFi5 wifi5;
    wifi5.simulate(numUsers, duration);
    std::cout << "Simulation Results for " << numUsers << " users:\n";
    std::cout << "Average Latency: " << wifi5.getAverageLatency() << " ms\n";
    std::cout << "Maximum Latency: " << wifi5.getMaxLatency() << " ms\n";
    std::cout << "Throughput: " << wifi5.calculateThroughput() << " Mbps\n\n";
}

int main() {
    double duration = 5.0;
    runSimulation(1, duration);
    runSimulation(10, duration);
    runSimulation(100, duration);
    return 0;
}
