#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <fstream>

#define PROC_STAT_PATH "/proc/stat"
#define PROC_STAT_BUFFER_SIZE 4096

struct CpuTimes
{
    uint64_t user, nice, system, idle, iowait, irq, softirq, steal;

    uint64_t total() const;
    uint64_t idle_all() const;
};

class CpuMonitor
{
public:
    explicit CpuMonitor(size_t cores);

    void sample();
    std::string format() const;

private:
    void parse(char* buf);
    void compute();

private:
    std::vector<CpuTimes> prev_;
    std::vector<CpuTimes> curr_;
    std::vector<double> usage_;
    std::ifstream procstat_;
};