#include "file.h"
#include "config.h"
#include "util.h"

void print_file(const file &f, const Config &config) {
    const auto &stat = get_stat(f);
    if (S_ISDIR(stat.st_mode))
        std::cout << "/";
    else if (config.classify && is_special_file(stat.st_mode))
        if (S_IEXEC & stat.st_mode)
            std::cout << "*";
        else if (S_ISLNK(stat.st_mode))
            std::cout << "@";
        else if (S_ISSOCK(stat.st_mode))
            std::cout << "=";
        else if (S_ISFIFO(stat.st_mode))
            std::cout << "|";
        else
            std::cout << "?";
    std::cout << basename(get_file_name(f).c_str());
    if (config.is_verbose) {
        struct tm *my_tm = localtime(&stat.st_mtim.tv_sec);
        std::cout << "\t\t" << stat.st_size << " " << 1900 + my_tm->tm_year << "-"
                  << my_tm->tm_mon
                  << "-"
                  << my_tm->tm_mday << " " << my_tm->tm_hour << ":" << my_tm->tm_min
                  << std::endl;
    } else {
        std::cout << " ";
    }
}
