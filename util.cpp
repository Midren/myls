#include "util.h"

bool is_special_file(const __mode_t &st_mode) {
    return !S_ISDIR(st_mode) && (S_IEXEC & st_mode || !S_ISREG(st_mode));
}

bool sort_comparator(const file &x, const file &y, const Config &cfg) {

    auto x_file_type = get_stat(x).st_mode, y_file_type = get_stat(y).st_mode;

    if (cfg.is_special_outside && is_special_file(x_file_type)) {
        if (!is_special_file(y_file_type))
            return false;
    } else if (cfg.is_special_outside && is_special_file(y_file_type)) {
        return true;
    }

    if (cfg.is_directories_first && S_ISDIR(x_file_type)) {
        if (!S_ISDIR(y_file_type))
            return true;
    } else if (cfg.is_directories_first && S_ISDIR(y_file_type)) {
        return false;
    }

    bool ret = false;
    bool is_equal = false;
    switch (cfg.sort_criteria) {
        case SortCriteria::Unsorted:
            ret = false;
            break;
        case SortCriteria::Size:
            ret = get_stat(x).st_size < get_stat(y).st_size;
            if (get_stat(x).st_size == get_stat(y).st_size)
                is_equal = true;
            break;
        case SortCriteria::Time:
            ret = get_stat(x).st_mtim.tv_sec > get_stat(y).st_mtim.tv_sec;
            if (get_stat(x).st_mtim.tv_sec == get_stat(y).st_mtim.tv_sec)
                is_equal = true;
            break;
        case SortCriteria::Name:
            ret = get_file_name(x) < get_file_name(y);
            break;
        case SortCriteria::Extension:
            size_t x_pos = get_file_name(x).find_last_of('.'), y_pos = get_file_name(y).find_last_of('.');
            ret = get_file_name(x).substr(x_pos + 1) < get_file_name(y).substr(y_pos + 1);
            if (get_file_name(x).substr(x_pos + 1) == get_file_name(y).substr(y_pos + 1))
                is_equal = true;
            break;
    }
    if (is_equal)
        ret = get_file_name(x) < get_file_name(y);
    return cfg.sort_order == SortOrder::Reversed ? !ret : ret;
}
