#include <iostream>

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
    SortOrder sort_order;
    SortCriteria sort_criteria;
    bool is_directories_first;
    bool is_special_outside;
    bool is_recursive;
    bool is_verbose;
};

int dir_handler(const char *filename, const struct stat *st, int info, struct FTW *ftw) {
    if (info == FTW_D) {
        std::cout << std::endl << std::endl
                  << filename << ":" << std::endl;
    }
    std::cout << filename << " ";
    return 0;
}

int main(int argc, char **argv) {
    int err = nftw(".", dir_handler, 0, FTW_CHDIR | FTW_MOUNT);
    return 0;
}
