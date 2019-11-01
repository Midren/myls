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
    SortOrder sort_order;
    SortCriteria sort_criteria;
    bool is_directories_first;
    bool is_special_outside;
    bool is_recursive;
    bool is_verbose;
    bool classify;
};

Config parse_config(int argc, char **argv);

#endif //MYLS_CONFIG_H
