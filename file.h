#ifndef MYLS_FILE_H
#define MYLS_FILE_H

#include <tuple>
#include <string>
#include <iostream>

#include <sys/stat.h>
#include <memory.h>

#include "config.h"

typedef std::tuple<std::string, struct stat, int> file;

inline struct stat get_stat(const file &f) {
    return std::get<1>(f);
}

inline std::string get_file_name(const file &f) {
    return std::get<0>(f);
}

void print_file(const file &f, const Config &config);

#endif //MYLS_FILE_H
