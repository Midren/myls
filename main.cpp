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
    SortOrder sort_order = SortOrder::Straightforward;
    SortCriteria sort_criteria = SortCriteria::Name;
    bool is_directories_first = false;
    bool is_special_outside = false;
    bool is_recursive = false;
    bool is_verbose = false;
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
    int err = nftw(".", dir_handler, 0, FTW_CHDIR | FTW_MOUNT | FTW_PHYS);
    return 0;
}
