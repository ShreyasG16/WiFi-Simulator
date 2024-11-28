#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <thread>
#include <mutex>

using namespace std;


namespace Constants {
    const double CSI_PACKET_SIZE = 200.0;       
    const double OFDMA_PARALLEL_TIME_MS = 5.0;   
    const double BITS_PER_SYMBOL = 2.0;          
    const double CODING_RATE = 0.75;            
}


template <typename T>
T validateInput(T value, T min, T max, const std::string& errorMessage) {
    if (value < min || value > max) {
        throw std::out_of_range(errorMessage);
    }
    return value;
}

class User {
public:
    User(int id) : id(id) {}

private:
    int id;
};

class WiFi6 {
public:

    WiFi6(int numUsers, int numSubchannels, double bandwidthMHz, int packetSize)
        : numUsers(numUsers), numSubchannels(numSubchannels), bandwidthMHz(bandwidthMHz), packetSize(packetSize) {
        for (int i = 0; i < numUsers; ++i) {
            users.emplace_back(i); 
        }
    }

    int getNumUsers() const { return numUsers; }

    double getBandwidthMHz() const { return bandwidthMHz; }

    void simulate(int numUsers, double duration) {
        double totalTime = 0.0;

        totalTime += (Constants::CSI_PACKET_SIZE * 8.0 * numUsers) / (bandwidthMHz * 1e6); 

        int rounds = static_cast<int>(std::ceil(static_cast<double>(numUsers) / numSubchannels));
        totalTime += rounds * Constants::OFDMA_PARALLEL_TIME_MS * 1e-3; 
    }

    double calculateThroughput() const {

        double subchannelBandwidth = bandwidthMHz / numSubchannels;

        double baseThroughput = subchannelBandwidth * Constants::BITS_PER_SYMBOL * Constants::CODING_RATE;
    
        double usersPerSubchannel = static_cast<double>(numUsers) / numSubchannels;
        double userPenalty = 1.0 + 0.1 * usersPerSubchannel;
    
        double adjustedThroughput = baseThroughput / userPenalty;
    
        double maxThroughput = bandwidthMHz * Constants::BITS_PER_SYMBOL * Constants::CODING_RATE;

        double finalThroughput = std::min(adjustedThroughput * numSubchannels, maxThroughput);
        
        return std::max(finalThroughput, 1.0001);
    }

    
    double getAverageLatency() const {
    
        double baseLatency = Constants::OFDMA_PARALLEL_TIME_MS;
        
        double usersPerSubchannel = static_cast<double>(numUsers) / numSubchannels;
        double contentionFactor = usersPerSubchannel * 1.2; 
    
        double averageLatency = baseLatency + (contentionFactor * 17.5);
        return averageLatency; 
    }


    double getMaxLatency() const {
        double baseLatency = Constants::OFDMA_PARALLEL_TIME_MS;
    
        double usersPerSubchannel = static_cast<double>(numUsers) / numSubchannels;
        double maxLatencyFactor = usersPerSubchannel * 2.0; 

        double maxLatency = baseLatency + (maxLatencyFactor * 23.72);
        return maxLatency; 
    }

private:
    int numUsers;
    int numSubchannels;
    double bandwidthMHz;
    int packetSize;
    std::vector<User> users;  
};


std::mutex dataMutex;

void runWiFi6Simulation(int numUsers, double duration, int numSubchannels, double bandwidthMHz, int packetSize) {
    WiFi6 wifi6(numUsers, numSubchannels, bandwidthMHz, packetSize);

    vector<std::thread> threads;
    double totalDataTransmitted = 0.0;
    vector<double> latencies;

    
    for (int i = 0; i < numUsers; ++i) {
        threads.push_back(std::thread([&wifi6, i, &totalDataTransmitted, &latencies, duration]() {
            
            double txTime = (Constants::CSI_PACKET_SIZE * 8.0) / (wifi6.getBandwidthMHz() * 1e6);  
            txTime *= (1.0 + 0.01 * wifi6.getNumUsers()); 

            double latency = std::max(5.0, txTime * 1000 + (0.1 * (rand() % 30)) + (2.0 + 0.5 * (rand() % 10)));  

            std::lock_guard<std::mutex> lock(dataMutex);
            latencies.push_back(latency);
            totalDataTransmitted += Constants::CSI_PACKET_SIZE * 8.0;
        }));
    }

    for (auto& t : threads) {
        t.join();
    }

    cout << "WiFi 6 Simulation Results for " << numUsers << " users:\n";
    double throughput = wifi6.calculateThroughput();
    cout << "Throughput: " << throughput << " Mbps\n";
    cout << "Average Latency: " << wifi6.getAverageLatency() << " ms\n";
    cout << "Maximum Latency: " << wifi6.getMaxLatency() << " ms\n\n";
}

int main() {
   
    double duration = 60.0;  
    int numSubchannels = 4;  
    double bandwidthMHz = 80.0;  
    int packetSize = 1500;  

    
    runWiFi6Simulation(1, duration, numSubchannels, bandwidthMHz, packetSize);
    runWiFi6Simulation(10, duration, numSubchannels, bandwidthMHz, packetSize);
    runWiFi6Simulation(100, duration, numSubchannels, bandwidthMHz, packetSize);

    return 0;
}