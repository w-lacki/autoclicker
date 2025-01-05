#pragma once
#include <vector>

struct record {
    std::vector<int> clicks;
    std::vector<int> releases;
};

void run_recorder();
