#include <iostream>
#include <boost/program_options.hpp>
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

int dir_handler(const char *filename, const struct stat *st, int info, struct FTW *ftw) {
    if (info == FTW_D) {
        std::cout << std::endl << std::endl
                  << filename << ":" << std::endl;
    }
    std::cout << filename << " ";
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
            ("long-listing,l", "more wide information: name, size, date and time of last modification")
            ("reverse,r", "reverse sorted output")
            ("recursive,R", "list subdirectories recursively")
            ("classify,F", "states the types of special files:\n"
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
    cfg.is_verbose = vm.count("long-listing") != 0;
    cfg.sort_order = vm.count("reverse") ? SortOrder::Reversed : SortOrder::Straightforward;
    cfg.is_recursive = vm.count("recursive") != 0;
    cfg.classify = vm.count("classify") != 0;
    for (const auto &target : vm["targets"].as<std::vector<std::string>>())
        cfg.targets.push_back(target);
}

int main(int argc, char **argv) {
    parse_config(argc, argv);
    int err = nftw(".", dir_handler, 0, FTW_CHDIR | FTW_MOUNT);
    return 0;
}
