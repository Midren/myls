#ifndef MYLS_UTIL_H
#define MYLS_UTIL_H

#include <tuple>
#include <string>

#include <sys/stat.h>

#include "config.h"
#include "file.h"

bool is_special_file(const __mode_t &st_mode);

bool sort_comparator(const file &x, const file &y, const Config &cfg);

#endif //MYLS_UTIL_H
