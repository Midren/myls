#include <iostream>
#include <boost/program_options.hpp>
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
    std::vector<std::string> targets;
    SortOrder sort_order;
    SortCriteria sort_criteria;
    bool is_directories_first;
    bool is_special_outside;
    bool is_recursive;
    bool is_verbose;
    bool classify;
};

Config cfg;
std::vector<std::vector<std::tuple<std::string, struct stat, int>>> dirs;

bool is_special(const __mode_t &st_mode) {
    return !S_ISDIR(st_mode) && (S_IEXEC & st_mode || !S_ISREG(st_mode));
}

bool sort_comparator(const std::tuple<std::string, struct stat, int> &x,
                     const std::tuple<std::string, struct stat, int> &y) {
    if (cfg.is_special_outside && is_special(std::get<1>(x).st_mode)) {
        if (!is_special(std::get<1>(y).st_mode))
            return false;
    } else if (cfg.is_special_outside && is_special(std::get<1>(y).st_mode)) {
        return true;
    }

    if (cfg.is_directories_first && S_ISDIR(std::get<1>(x).st_mode)) {
        if (!S_ISDIR(std::get<1>(y).st_mode))
            return true;
    } else if (cfg.is_directories_first && S_ISDIR(std::get<1>(y).st_mode)) {
        return false;
    }

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

// From https://stackoverflow.com/questions/5475693/using-as-end-of-options-marker-with-boostprogram-options
std::vector<boost::program_options::option> end_of_opts_parser(std::vector<std::string> &args) {
    std::vector<boost::program_options::option> result;
    std::vector<std::string>::const_iterator i(args.begin());
    if (i != args.end() && *i == "--") {
        for (++i; i != args.end(); ++i) {
            boost::program_options::option opt;
            opt.string_key = "targets";
            opt.value.push_back(*i);
            opt.original_tokens.push_back(*i);
            result.push_back(opt);
        }
        args.clear();
    }
    return result;
}

void parse_config(int argc, char **argv) {
    boost::program_options::options_description optionsDescription("Options");
    optionsDescription.add_options()("help,h", "print help message")
            (",l", boost::program_options::value<std::vector<bool>>()->zero_tokens(),
             "more wide information: name, size, date and time of last modification")
            (",r", boost::program_options::value<std::vector<bool>>()->zero_tokens(), "reverse sorted output")
            (",R", boost::program_options::value<std::vector<bool>>()->zero_tokens(), "list subdirectories recursively")
            (",F", boost::program_options::value<std::vector<bool>>()->zero_tokens(),
             "states the types of special files:\n"
             "\t* - executable\n"
             "\t@ - symlink\n"
             "\t| - named channel\n"
             "\t= - socket\n"
             "\t? - other\n")
            ("sort", boost::program_options::value<std::vector<std::string>>()->composing()->default_value(
                    std::vector<std::string>{"N"}, "N"), "specify sort strategy:\n"
                                                         "\tU - unsorted\n"
                                                         "\tS - size\n"
                                                         "\tt - last modified\n"
                                                         "\tX - extension\n"
                                                         "\tN - name (default)\n"
                                                         "\tD - directories first (can be used with first 5)\n"
                                                         "\ts - special files separate (can be used with first 5)\n");
    boost::program_options::options_description hidden("hidden");
    hidden.add_options()
            ("targets", boost::program_options::value<std::vector<std::string>>()->composing()->default_value(
                    std::vector<std::string>{"."}, "."));
    boost::program_options::options_description commandLineOptions;
    commandLineOptions.add(optionsDescription).add(hidden);
    boost::program_options::positional_options_description positional;
    positional.add("targets", -1);

    boost::program_options::variables_map vm;
    auto parsed = boost::program_options::command_line_parser(argc, argv).extra_style_parser(
            end_of_opts_parser).options(commandLineOptions).positional(
            positional).run();
    boost::program_options::store(parsed, vm);

    if (vm.count("help")) {
        std::cout << optionsDescription << std::endl;
        exit(0);
    }

    if (vm.count("sort")) {
        auto sortingStrategies = vm["sort"].as<std::vector<std::string>>();
        for (const auto &argument:sortingStrategies)
            for (const auto &strategy:argument)
                switch (strategy) {
                    case 'U':
                        cfg.sort_criteria = SortCriteria::Unsorted;
                        break;
                    case 'S':
                        cfg.sort_criteria = SortCriteria::Size;
                        break;
                    case 't':
                        cfg.sort_criteria = SortCriteria::Time;
                        break;
                    case 'X':
                        cfg.sort_criteria = SortCriteria::Extension;
                        break;
                    case 'N':
                        cfg.sort_criteria = SortCriteria::Name;
                        break;
                    case 'D':
                        cfg.is_directories_first = true;
                        break;
                    case 's':
                        cfg.is_special_outside = true;
                        break;
                    default:
                        continue;
                }
    }
    cfg.is_verbose = vm.count("-l") != 0;
    cfg.sort_order = vm.count("-r") ? SortOrder::Reversed : SortOrder::Straightforward;
    cfg.is_recursive = vm.count("-R") != 0;
    cfg.classify = vm.count("-F") != 0;
    for (const auto &target : vm["targets"].as<std::vector<std::string>>())
        cfg.targets.push_back(target);
}

void print_file(const std::tuple<std::string, struct stat, int> &f) {
    if (std::get<2>(f) == FTW_D)
        std::cout << "/";
    else if (cfg.classify && is_special(std::get<1>(f).st_mode))
        if (S_IEXEC & std::get<1>(f).st_mode)
            std::cout << "*";
        else if (S_ISLNK(std::get<1>(f).st_mode))
            std::cout << "@";
        else if (S_ISSOCK(std::get<1>(f).st_mode))
            std::cout << "=";
        else if (S_ISFIFO(std::get<1>(f).st_mode))
            std::cout << "|";
        else
            std::cout << "?";
    std::cout << basename(std::get<0>(f).c_str());
    if (cfg.is_verbose) {
        struct tm *my_tm = localtime(&std::get<1>(f).st_mtim.tv_sec);
        std::cout << "\t\t" << std::get<1>(f).st_size << " " << 1900 + my_tm->tm_year << "-"
                  << my_tm->tm_mon
                  << "-"
                  << my_tm->tm_mday << " " << my_tm->tm_hour << ":" << my_tm->tm_min;
    }
    std::cout << std::endl;
}

int outputTarget(const std::string &target, bool multipleTargets) {
    if (nftw(target.c_str(), dir_handler, 1, FTW_MOUNT | FTW_PHYS | FTW_ACTIONRETVAL) != -1) {
        sort_files();
        for (const auto &dir: dirs) {
            for (const auto &f: dir) {
                if (multipleTargets && std::get<0>(f) == std::get<0>(dir[0])) {
                    std::cout << target << (S_ISDIR(std::get<1>(f).st_mode) ? ": " : " ") << std::endl;
                } else {
                    print_file(f);
                }
            }
            std::cout << std::endl;
        }
    } else {
        std::cerr << target << "Not a directory!" << std::endl;
        return 1;
    }
    return 0;
}

int main(int argc, char **argv) {
    parse_config(argc, argv);
    bool multi = cfg.targets.size() > 1;
    for (const auto &target:cfg.targets) {
        dirs.clear();
        if (outputTarget(target, multi))
            return 1;
    }
    return 0;
}
