#include <iostream>
#include <vector>
#include <tuple>
#include <algorithm>

#include <ftw.h>

enum SortCriteria {
    Unsorted,
    Size,
    Time,
    Extension,
    Name
};

enum SortOrder {
    Straightforward,
    Reversed
};

struct Config {
    SortOrder sort_order = SortOrder::Straightforward;
    SortCriteria sort_criteria = SortCriteria::Name;
    bool is_directories_first = false;
    bool is_special_outside = false;
    bool is_recursive = false;
    bool is_verbose = false;
};

Config cfg;
std::vector<std::vector<std::tuple<std::string, struct stat, int>>> dirs;

bool sort_comparator(const std::tuple<std::string, struct stat, int> &x,
                     const std::tuple<std::string, struct stat, int> &y) {
    bool ret = false;
    bool is_equal = false;
    switch (cfg.sort_criteria) {
        case SortCriteria::Unsorted:
            ret = false;
            break;
        case SortCriteria::Size:
            ret = std::get<1>(x).st_size < std::get<1>(y).st_size;
            if (std::get<1>(x).st_size == std::get<1>(y).st_size)
                is_equal = true;
            break;
        case SortCriteria::Time:
            ret = std::get<1>(x).st_mtim.tv_sec > std::get<1>(y).st_mtim.tv_sec;
            if (std::get<1>(x).st_mtim.tv_sec == std::get<1>(y).st_mtim.tv_sec)
                is_equal = true;
            break;
        case SortCriteria::Name:
            ret = std::get<0>(x) < std::get<0>(y);
            break;
            is_equal = true;
        case SortCriteria::Extension:
            size_t x_pos = std::get<0>(x).find_last_of('.'), y_pos = std::get<0>(y).find_last_of('.');
            ret = std::get<0>(x).substr(x_pos + 1) < std::get<0>(y).substr(y_pos + 1);
            if (std::get<0>(x).substr(x_pos + 1) == std::get<0>(y).substr(y_pos + 1))
                is_equal = true;
            break;
    }
    if (is_equal)
        ret = std::get<0>(x) < std::get<0>(y);
    return cfg.sort_order == SortOrder::Reversed ? !ret : ret;
}

void sort_files() {
    std::sort(dirs.begin() + 1, dirs.end(), [](const auto &x, const auto &y) {
        return sort_comparator(x.front(), y.front());
    });
    for (auto &dir: dirs) {
        std::sort(dir.begin() + 1, dir.end(), sort_comparator);
    }
}

int dir_handler(const char *filename, const struct stat *st, int info, struct FTW *ftw) {
    if (!cfg.is_recursive && ftw->level > 1)
        return FTW_SKIP_SUBTREE;
    if (cfg.is_recursive) {
        if (info == FTW_D)
            dirs.emplace_back();
    } else {
        if (dirs.empty())
            dirs.emplace_back();
    }
    dirs.back().emplace_back(std::string(filename), *st, info);
    return 0;
}

int main(int argc, char **argv) {
    cfg.is_recursive = false;
    cfg.sort_criteria = SortCriteria::Time;
    if (nftw("/home/midren/Documents", dir_handler, 1, FTW_MOUNT | FTW_PHYS | FTW_ACTIONRETVAL) != -1) {
        sort_files();
        for (const auto &dir: dirs) {
            for (const auto &f: dir)
                std::cout << std::get<0>(f) << std::endl;
            std::cout << std::endl;
        }
    }
    return 0;
}
