#pragma once
#include <string>

struct Config
{
    std::string file;
    int interval = -1;
};

bool parse_args(int argc, char* argv[], Config& cfg);