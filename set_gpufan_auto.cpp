#include <iostream>
#include <nvml.h>
#include <cstdlib>
#include <csignal>
#include <thread>
#include <chrono>


#define MAX_FAN_SPEED 100
#define AUTO_FAN_SPEED 0

// Forward declarations
void resetFansToAuto();
void signalHandler(int signum);

unsigned int getFanSpeed(int current_temp, int max_temp) {
    int difference = max_temp - current_temp;
    if (difference >= 40) return AUTO_FAN_SPEED;
    if (difference >= 35) return 30;
    if (difference >= 30) return 40;
    if (difference >= 25) return 50;
    if (difference >= 20) return 60;
    if (difference >= 15) return 70;
    if (difference >= 10) return 80;
    if (difference >= 5) return 90;
    return MAX_FAN_SPEED;
}


// Global variable to store device count and a flag for graceful shutdown
unsigned int device_count;
bool gracefulShutdown = false;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./set_fan_curve <max_temperature>" << std::endl;
        return 1;
    }

    int max_temp = std::atoi(argv[1]);
    if (max_temp < 50 || max_temp > 90) {
        std::cerr << "Max temperature must be a value between 50 and 90" << std::endl;
        return 1;
    }

    nvmlReturn_t result;
    unsigned int device_count, i;

    result = nvmlInit();
    if (NVML_SUCCESS != result) {
        std::cerr << "Failed to initialize NVML: " << nvmlErrorString(result) << std::endl;
        return 1;
    }

    result = nvmlDeviceGetCount(&device_count);
    if (NVML_SUCCESS != result) {
        std::cerr << "Failed to query device count: " << nvmlErrorString(result) << std::endl;
        return 1;
    }

    // Register signal handler for SIGINT and SIGTERM
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);
 
   
    resetFansToAuto();
    nvmlShutdown();

    return 0;
}

void resetFansToAuto() {
    for (unsigned int i = 0; i < device_count; i++) {
        nvmlDevice_t device;
        nvmlReturn_t result = nvmlDeviceGetHandleByIndex(i, &device);
        if (result != NVML_SUCCESS) continue;

        unsigned int fanCount;
        result = nvmlDeviceGetNumFans(device, &fanCount);
        if (result != NVML_SUCCESS) continue;

        for (unsigned int fanIdx = 0; fanIdx < fanCount; fanIdx++) {
            // Reset fan speed to automatic
            nvmlDeviceSetFanSpeed_v2(device, fanIdx, AUTO_FAN_SPEED);
        }
    }
}

void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\nReset Fans to Auto\n";

    // Set flag for graceful shutdown
    gracefulShutdown = true;

    // Reset fans to automatic control
    resetFansToAuto();

    // Cleanup and close up stuff here

    // Terminate program
    exit(signum);
}
