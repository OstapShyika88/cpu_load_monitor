#include "config.hpp"
#include <cstring>

bool parse_args(int argc, char* argv[], Config& cfg)
{
    int valid_arg_cnt = 0;

    for (int i = 1; i < argc; ++i)
    {
        if (std::strcmp(argv[i], "-f") == 0 && i + 1 < argc)
        {
            cfg.file = argv[++i];

            if (cfg.file.empty())
            {
                return 1;
            }

            valid_arg_cnt++;
        }
        else if (std::strcmp(argv[i], "-n") == 0 && i + 1 < argc)
        {
            std::size_t pos = 0;
            cfg.interval = std::stoi(argv[++i], &pos);

            if (pos != std::strlen(argv[i]) || cfg.interval < 0)
            {
                return 1;
            }

            valid_arg_cnt++;
        }
        else
        {
            return 1;
        }
    }

    return valid_arg_cnt != 2;
}