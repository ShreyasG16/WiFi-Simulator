# WiFi Simulator

## Project Overview
This project simulates the performance of various WiFi standards (WiFi 4, WiFi 5, WiFi 6) under different conditions, measuring throughput, latency, and collision rates. The simulation evaluates the effects of varying the number of users (1, 10, and 100) on the WiFi network's performance. It is designed to help understand how different WiFi generations handle network traffic under different loads.

### Key Features:
- Simulates the performance of WiFi 4, WiFi 5, and WiFi 6
- Measures throughput, average latency, maximum latency, and total collisions
- Adjustable user loads (1, 10, 100 users)
- Optimized and debug versions for performance analysis

## Build Instructions
To build the project, use the following commands:

### Build in Debug Mode:
make debug

### Clean Up Build Files:
make clean

### Running the Program

Once the project is built, you can run the simulation for each WiFi standard:
For WiFi 4:

./wifi4

For WiFi 5:

./wifi5

For WiFi 6:

./wifi6

### Input Files Format

The program does not require any external input files. All parameters are simulated within the code itself for each of the WiFi standards.
Dependencies

    g++ compiler (C++17 or higher)
    pthread library for multithreading support
    
