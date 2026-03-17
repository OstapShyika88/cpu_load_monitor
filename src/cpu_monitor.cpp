#include "cpu_monitor.hpp"
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <sstream>
#include <string_view>
#include <charconv>

uint64_t CpuTimes::total() const
{
    return user + nice + system + idle + iowait + irq + softirq + steal;
}

uint64_t CpuTimes::idle_all() const
{
    return idle + iowait;
}

CpuMonitor::CpuMonitor(size_t cores)
    : prev_(cores), curr_(cores), usage_(cores), procstat_(PROC_STAT_PATH)
{
    if (!procstat_.is_open())
    {
        throw std::runtime_error("Failed to open /proc/stat");
    }
    sample();
}

void CpuMonitor::sample()
{
    procstat_.seekg(0, std::ios::beg);

    char buf[PROC_STAT_BUFFER_SIZE];
    procstat_.read(buf, sizeof(buf) - 1);
    ssize_t n = procstat_.gcount();
    if (n <= 0) throw std::runtime_error("Failed to read CPU stats from /proc/stat");
    buf[n] = '\0';

    parse(buf);
    compute();
    prev_ = curr_;
}

static inline bool parse_uint64(std::string_view& sv, uint64_t& out)
{
    while (!sv.empty() && sv.front() == ' ')
        sv.remove_prefix(1);

    if (sv.empty()) return false;

    const char* begin = sv.data();
    const char* end = sv.data() + sv.size();

    auto res = std::from_chars(begin, end, out);
    if (res.ec != std::errc()) return false;

    size_t parsed_len = res.ptr - begin;
    sv.remove_prefix(parsed_len);

    return true;
}

void CpuMonitor::parse(char* buf)
{
    size_t idx = 0;

    std::string_view input(buf);

    while (!input.empty())
    {
        size_t pos = input.find('\n');
        std::string_view line = input.substr(0, pos);

        if (pos == std::string_view::npos)
            input = {};
        else
            input.remove_prefix(pos + 1);

        if (line.size() < 4 || line.substr(0, 3) != "cpu")
            break;

        // skip "cpu "
        if (line[3] == ' ')
            continue;

        if (idx >= curr_.size())
            break;

        size_t space = line.find(' ');
        if (space == std::string_view::npos)
            continue;

        std::string_view fields = line.substr(space);

        CpuTimes t{};

        if (!parse_uint64(fields, t.user)) continue;
        if (!parse_uint64(fields, t.nice)) continue;
        if (!parse_uint64(fields, t.system)) continue;
        if (!parse_uint64(fields, t.idle)) continue;
        if (!parse_uint64(fields, t.iowait)) continue;
        if (!parse_uint64(fields, t.irq)) continue;
        if (!parse_uint64(fields, t.softirq)) continue;
        if (!parse_uint64(fields, t.steal)) continue;

        curr_[idx++] = t;
    }
}

void CpuMonitor::compute()
{
    for (size_t i = 0; i < curr_.size(); ++i)
    {
        auto dt = curr_[i].total() - prev_[i].total();
        auto di = curr_[i].idle_all() - prev_[i].idle_all();

        if (dt > 0)
        {
            usage_[i] = 100.0 * (1.0 - static_cast<double>(di) / dt);
        }
    }
}

std::string CpuMonitor::format() const
{
    std::ostringstream oss;
    for (size_t i = 0; i < usage_.size(); ++i)
    {
        oss << "CPU" << i << ": " << usage_[i] << "%\n";
    }
    return oss.str();
}