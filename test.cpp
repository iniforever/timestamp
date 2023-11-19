#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <cstdio>
#include "TimeUtils.h"

std::queue<uint64_t> timestampQueue;
std::mutex queueMutex;
std::condition_variable queueCondVar;
bool finished = false;

void producer() {
    while (!finished) {
        uint64_t timestamp = TimeUtils::get_timestamp();
        //uint64_t timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        {
            //std::lock_guard<std::mutex> lock(queueMutex);
            timestampQueue.push(timestamp);
        }
        queueCondVar.notify_one();
        //std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Adjust the rate of timestamp generation
    }
}

void consumer(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        std::cerr << "Failed to open file for writing." << std::endl;
        return;
    }

    while (!finished || !timestampQueue.empty()) {
        std::unique_lock<std::mutex> lock(queueMutex);
        queueCondVar.wait(lock, []{ return !timestampQueue.empty() || finished; });
        while (!timestampQueue.empty()) {
            uint64_t timestamp = timestampQueue.front();
            timestampQueue.pop();
            lock.unlock();
            fprintf(file, "%llu\n", timestamp);
            lock.lock();
        }
    }

    fclose(file);
}

int main() {
    TimeUtils::initialize(); // Initialize TimeUtils

    std::thread producerThread(producer);
    std::this_thread::sleep_for(std::chrono::milliseconds(1)); // Run for 10 seconds
    finished = true;

    std::thread consumerThread(consumer, "timestamps_3.txt");
    producerThread.join();
    consumerThread.join();

    return 0;
}