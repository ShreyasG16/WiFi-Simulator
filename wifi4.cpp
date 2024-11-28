#include <iostream>
#include <vector>
#include <memory>
#include <random>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <thread>
#include <mutex>
#include <stdexcept>

namespace Constants {
    const int PACKET_SIZE_BYTES = 1500;
    const int DIFS_US = 34;
    const int SLOT_TIME_US = 9;
    const int BANDWIDTH_MHZ = 20;
}

// Template for validating numeric inputs
template <typename T>
T validateInput(T value, T min, T max, const std::string& errorMessage) {
    if (value < min || value > max) {
        throw std::out_of_range(errorMessage);
    }
    return value;
}

class NetworkElement {
protected:
    int id;
    double backoffTime;

public:
    NetworkElement(int id, double backoffTime) : id(id), backoffTime(backoffTime) {}
    virtual ~NetworkElement() = default;

    virtual bool transmit(int channel) = 0;
    virtual void updateBackoffCounter() = 0;
};

class AccessPoint : public NetworkElement {
public:
    int mainChannel;

    AccessPoint(int id) : NetworkElement(id, 0) {
        mainChannel = 1;
    }

    bool transmit(int channel) override {
        return rand() % 2 == 0;
    }

    void updateBackoffCounter() override {
        // No backoff for access point itself
    }
};

class Packet {
public:
    int size;
    double transmissionTime;

    Packet(int packetSize) : size(packetSize) {
        transmissionTime = size * 8.0 / (Constants::BANDWIDTH_MHZ * 1e6);
    }
};

class Channel {
private:
    int channelNumber;

public:
    Channel(int channel) : channelNumber(channel) {}

    int getChannel() const {
        return channelNumber;
    }
};

class User : public NetworkElement {
public:
    User(int id, double backoffTime) : NetworkElement(id, backoffTime) {}

    bool transmit(int channel) override {
        return rand() % 2 == 0;
    }

    void updateBackoffCounter() override {
        backoffTime += Constants::SLOT_TIME_US * 1e-6;
    }
};

class WiFi4 {
private:
    std::vector<std::shared_ptr<User>> users;
    AccessPoint ap;
    std::vector<double> latencies;
    double totalThroughput = 0.0;
    int collisionCount = 0;
    std::vector<double> backoffTimes;
    std::mutex mtx;

    double calculateTransmissionTime(int packetSize) const {
        return packetSize * 8.0 / (Constants::BANDWIDTH_MHZ * 1e6);
    }

    void handleCollision(User& user) {
        std::lock_guard<std::mutex> lock(mtx);
        user.updateBackoffCounter();
        collisionCount++;
    }

    void updateThroughput(int numUsers) {
        std::lock_guard<std::mutex> lock(mtx);
        double penaltyFactor = 1.0 - (0.01 * numUsers);
        penaltyFactor = std::max(0.67, penaltyFactor);
        double adjustedThroughput = 15.0 * penaltyFactor;
        totalThroughput = adjustedThroughput * 1e6;  // Convert to bps
    }

public:
    WiFi4() : ap(1) {}

    void simulate(int numUsers, double duration) {
        try {
            validateInput(numUsers, 1, 1000, "Number of users must be between 1 and 1000");
            validateInput(duration, 1.0, 10000.0, "Simulation duration must be between 1 and 10000 milliseconds");
        } catch (const std::exception& e) {
            std::cerr << "Input validation error: " << e.what() << "\n";
            return;
        }

        users.clear();
        collisionCount = 0;
        backoffTimes.clear();

        for (int i = 0; i < numUsers; i++) {
            users.push_back(std::make_shared<User>(i, 20.0 + (rand() % 20)));
        }

        double currentTime = 0;
        std::vector<std::thread> threads;

        while (currentTime < duration / 1000.0) {
            for (auto& user : users) {
                threads.push_back(std::thread([this, &user, &currentTime, numUsers]() {
                    bool success = user->transmit(ap.mainChannel);
                    double txTime = calculateTransmissionTime(Constants::PACKET_SIZE_BYTES);

                    if (success) {
                        double latency = txTime * 1000 * (1.0 + 0.002 * numUsers) + (0.1 * (rand() % 20));
                        latency = std::max(0.8, latency);

                        std::lock_guard<std::mutex> lock(mtx);
                        latencies.push_back(latency);
                        totalThroughput += Constants::PACKET_SIZE_BYTES * 8.0;
                    } else {
                        handleCollision(*user);
                    }
                    currentTime += 0.00001;
                }));
            }

            for (auto& t : threads) {
                if (t.joinable()) {
                    t.join();
                }
            }
        }
        updateThroughput(numUsers);
    }

    double calculateThroughput() const {
        return totalThroughput / 1e6;
    }

    double getAverageLatency() const {
        return latencies.empty() ? 0.0 : std::accumulate(latencies.begin(), latencies.end(), 0.0) / latencies.size();
    }

    double getMaxLatency() const {
        return latencies.empty() ? 0.0 : *std::max_element(latencies.begin(), latencies.end());
    }

    int getCollisionCount() const {
        return collisionCount;
    }
};

int main() {
    std::vector<int> userCounts = {1, 10, 100};
    double duration = 100; // in milliseconds

    for (int numUsers : userCounts) {
        std::cout << "\nRunning simulation with " << numUsers << " user(s) for " << duration << " ms\n";
        WiFi4 wifiSimulator;
        wifiSimulator.simulate(numUsers, duration);
        std::cout << "WiFi 4 Throughput: " << wifiSimulator.calculateThroughput() << " Mbps\n";
        std::cout << "WiFi 4 Average Latency: " << wifiSimulator.getAverageLatency() << " ms\n";
        std::cout << "WiFi 4 Max Latency: " << wifiSimulator.getMaxLatency() << " ms\n";
        std::cout << "Total Collisions: " << wifiSimulator.getCollisionCount() << "\n";
    }

    return 0;
}
