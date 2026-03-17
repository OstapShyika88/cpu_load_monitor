#include "config.hpp"
#include "cpu_monitor.hpp"

#include <unistd.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <csignal>

#define SLEEP_MCS 500000

volatile std::sig_atomic_t g_stop = 0;

void handle_sigint(int)
{
    g_stop = 1;
}

int main(int argc, char* argv[])
{
    // -------- INIT --------
    std::signal(SIGINT, handle_sigint);

    Config cfg;
    if (parse_args(argc, argv, cfg) != 0)
    {
        std::cerr << "Usage: " << argv[0] << " [-f <output_file> -n <interval_seconds>]" << std::endl;
        return 1;
    }

    long cores = sysconf(_SC_NPROCESSORS_ONLN);
    try
    {
        CpuMonitor monitor(cores);

        monitor.sample();

        if (cfg.interval < 0)
        {
            usleep(SLEEP_MCS);
            monitor.sample();
            std::cout << monitor.format();
            return 0;
        }

        std::ofstream writer(cfg.file);

        if (!writer.is_open())
        {
            std::cerr << "Error: Could not open output file: " << cfg.file << std::endl;
            return 1;
        }

        // -------- RUN --------
        auto last = std::chrono::steady_clock::now();

        while (!g_stop)
        {
            monitor.sample();

            auto now = std::chrono::steady_clock::now();
            auto sec = std::chrono::duration_cast<std::chrono::seconds>(now - last).count();

            if (sec >= cfg.interval)
            {
                std::string cpu_stats = std::move(monitor.format());
                writer << cpu_stats << std::flush;
                std::cout << cpu_stats;
                last = now;
            }

            usleep(SLEEP_MCS);
        }

    }
    catch (const std::runtime_error& e)
    {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

}