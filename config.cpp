#include "config.h"
#include <boost/program_options.hpp>
#include <iostream>

Config get_config(const boost::program_options::variables_map &vm) {
    Config cfg;
    if (vm.count("sort")) {
        auto sortingStrategies = vm["sort"].as<std::vector<std::string >>();
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
    for (const auto &target : vm["targets"].as<std::vector<std::string >>())
        cfg.targets.push_back(target);

    return cfg;
}

Config parse_config(int argc, char **argv) {
    boost::program_options::options_description optionsDescription("Options");
    optionsDescription.add_options()("help,h", "print help message")
            (",l", boost::program_options::value<std::vector<bool >>()->zero_tokens(),
             "more wide information: name, size, date and time of last modification")
            (",r", boost::program_options::value<std::vector<bool >>()->zero_tokens(), "reverse sorted output")
            (",R", boost::program_options::value<std::vector<bool >>()->zero_tokens(),
             "list subdirectories recursively")
            (",F", boost::program_options::value<std::vector<bool >>()->zero_tokens(),
             "states the types of special files:\n"
             "\t* - executable\n"
             "\t@ - symlink\n"
             "\t| - named channel\n"
             "\t= - socket\n"
             "\t? - other\n")
            ("sort", boost::program_options::value<std::vector<std::string >>()->composing()->default_value(
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
            ("targets", boost::program_options::value<std::vector<std::string >>()->composing()->default_value(
                    std::vector<std::string>{"."}, "."));
    boost::program_options::options_description commandLineOptions;
    commandLineOptions.add(optionsDescription).add(hidden);
    boost::program_options::positional_options_description positional;
    positional.add("targets", -1);

    boost::program_options::variables_map vm;
    auto parsed = boost::program_options::command_line_parser(argc, argv).options(commandLineOptions).positional(
            positional).run();
    boost::program_options::store(parsed, vm);

    if (vm.count("help")) {
        std::cout << optionsDescription << std::endl;
        exit(0);
    }

    return get_config(vm);
}
