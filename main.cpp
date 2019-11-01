#include <iostream>
#include <map>
#include <algorithm>
#include <functional>

#include <ftw.h>

#include "config.h"
#include "file.h"
#include "util.h"

static std::map<std::string, std::vector<file>> dirs;
static Config cfg;
static int err = 0;

int dir_iterator_callback(const char *filename, const struct stat *st, int info, struct FTW *ftw) {
    if (!cfg.is_recursive && ftw->level > 1)
        return FTW_SKIP_SUBTREE;

    if (info == FTW_DNR) {
        std::cerr << "myls: cannot access '" << filename << "': Directory cannot be read" << std::endl;
        err = 2;
    }

    std::string filename_str = filename;
    if (cfg.is_recursive && info == FTW_D)
        dirs[filename].emplace_back(filename_str, *st, info);
    if (ftw->level > 0 || info == FTW_F) {
        std::string dirname = filename_str.substr(0, filename_str.find_last_of(basename(filename)) -
                                                     strlen(basename(filename)));
        dirs[dirname].emplace_back(filename_str, *st, info);
    }
    return 0;
}


int print_files(const std::string &target, bool multipleTargets) {
    dirs.clear();
    if (nftw(target.c_str(), dir_iterator_callback, 1, FTW_MOUNT | FTW_PHYS | FTW_ACTIONRETVAL | FTW_DEPTH) != -1) {
        for (auto &dir: dirs) {
            std::sort(dir.second.begin(), dir.second.end(),
                      std::bind(sort_comparator, std::placeholders::_1, std::placeholders::_2, std::cref(cfg)));
        }
        for (const auto &dir: dirs) {
            if ((multipleTargets || dirs.size() > 1) && get_file_name(dir.second[0]) != dir.first)
                std::cout << dir.first << ":" << std::endl;
            for (const auto &f: dir.second) {
                print_file(f, cfg);
            }
            std::cout << std::endl << std::endl;
        }
    } else {
        std::cerr << "myls: cannot access '" << target << "': No such file or directory" << std::endl;
        return 1;
    }
    return err;
}

int main(int argc, char **argv) {
    cfg = parse_config(argc, argv);
    bool multi = cfg.targets.size() > 1;
    int error;
    for (const auto &target:cfg.targets) {
        if ((error = print_files(target, multi)))
            return error;
    }
    return 0;
}
