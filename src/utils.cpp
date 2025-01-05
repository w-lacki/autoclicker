#include "utils.h"
#include <Windows.h>
#include <cmath>
#include <chrono>
#include <bits/random.h>
#include <random>

long long current_time_millis() {
    // Get the current time point
    auto now = std::chrono::system_clock::now();

    // Convert to time since epoch
    auto epoch = now.time_since_epoch();

    // Convert to milliseconds
    return std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count();
}


void sleep_for(const int millis) {
    auto seconds = millis / 1000.0;
    using namespace std::chrono;

    static HANDLE timer = CreateWaitableTimer(NULL, FALSE, NULL);
    static double estimate = 5e-3;
    static double mean = 5e-3;
    static double m2 = 0;
    static int64_t count = 1;

    while (seconds - estimate > 1e-7) {
        double toWait = seconds - estimate;
        LARGE_INTEGER due;
        due.QuadPart = -static_cast<int64_t>(toWait * 1e7);
        auto start = high_resolution_clock::now();
        SetWaitableTimerEx(timer, &due, 0, NULL, NULL, NULL, 0);
        WaitForSingleObject(timer, INFINITE);
        auto end = high_resolution_clock::now();

        double observed = (end - start).count() / 1e9;
        seconds -= observed;

        ++count;
        double error = observed - toWait;
        double delta = error - mean;
        mean += delta / count;
        m2 += delta * (error - mean);
        double stddev = sqrt(m2 / (count - 1));
        estimate = mean + stddev;
    }

    // spin lock
    auto start = high_resolution_clock::now();
    while ((high_resolution_clock::now() - start).count() / 1e9 < seconds);
}

double random_between(const double low, const double high) {
    static std::random_device rd;
    static std::mt19937 mt(rd());
    std::uniform_real_distribution dist(low, high);
    return dist(mt);
}
