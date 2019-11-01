#ifndef MYLS_CONFIG_H
#define MYLS_CONFIG_H

#include <vector>
#include <string>

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
    SortOrder sort_order = SortOrder::Straightforward;
    SortCriteria sort_criteria = SortCriteria::Name;
    bool is_directories_first = false;
    bool is_special_outside = false;
    bool is_recursive = false;
    bool is_verbose = false;
    bool classify = false;
};

Config parse_config(int argc, char **argv);

#endif //MYLS_CONFIG_H
