#include "Cycles.h"
#include <iostream>
#include <chrono>
#include <ctime>

class TimeUtils {
public:
    static void initialize() {
        if (!initialized) {
            calibrate();
            calculateMidnight();
            initialized = true;
            std::cout<<" Initialized "<<std::endl ;
        }
    }
    static uint64_t get_timestamp_today() {
        //initialize(); // Ensure initialization has been done
        return getCurrentNanoseconds() - midnightNanosecondsSinceEpoch;
    }

    static uint64_t get_timestamp() {
        //initialize(); // Ensure initialization has been done
        return getCurrentNanoseconds();
    }

private:
private:
    static void calibrate() {
        // Initialize the Cycles class
        PerfUtils::Cycles::init();

        // Get the current number of cycles
        initialCycles = PerfUtils::Cycles::rdtsc();

        // Get the current wall-clock time in nanoseconds since the epoch
        auto now = std::chrono::system_clock::now();
        initialNanosecondsSinceEpoch = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();

        // Calculate cycles per nanosecond
        cyclesPerNanosecond_1 = 1e9/PerfUtils::Cycles::perSecond();
    }

    static void calculateMidnight() {
        // Get the current time and convert to a tm struct
        time_t now_time_t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::tm now_tm = *std::localtime(&now_time_t);

        // Reset hours, minutes, and seconds to zero for midnight
        now_tm.tm_hour = 0; now_tm.tm_min = 0; now_tm.tm_sec = 0;

        // Convert back to time_t for midnight
        auto midnight_time_t = std::mktime(&now_tm);

        // Convert midnight time_t to a time_point
        auto midnight = std::chrono::system_clock::from_time_t(midnight_time_t);

        // Calculate the nanoseconds since epoch for midnight
        midnightNanosecondsSinceEpoch = std::chrono::duration_cast<std::chrono::nanoseconds>(midnight.time_since_epoch()).count();
    }

    static uint64_t getCurrentNanoseconds() {
        uint64_t currentCycles = PerfUtils::Cycles::rdtsc();
        return initialNanosecondsSinceEpoch + (currentCycles - initialCycles) * cyclesPerNanosecond_1;
    }

    static bool initialized;
    static uint64_t initialCycles;
    static uint64_t initialNanosecondsSinceEpoch;
    static uint64_t midnightNanosecondsSinceEpoch;
    static long double cyclesPerNanosecond_1;
};

// Static member initialization
bool TimeUtils::initialized = false;
uint64_t TimeUtils::initialCycles = 0;
uint64_t TimeUtils::initialNanosecondsSinceEpoch = 0;
uint64_t TimeUtils::midnightNanosecondsSinceEpoch = 0;
long double TimeUtils::cyclesPerNanosecond_1 = 0.0;
