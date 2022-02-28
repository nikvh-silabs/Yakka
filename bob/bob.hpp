#pragma once

#include "utilities.hpp"
#include "yaml-cpp/yaml.h"
#include <set>
#include <string>
#include <functional>
#include <unordered_set>
#include <future>

namespace bob {
    const std::string bob_component_extension   = ".bob";
    const char data_dependency_identifier = ':';
    const char data_wildcard_identifier = '*';

    #if defined(_WIN64) || defined(_WIN32) || defined(__CYGWIN__)
    const std::string host_os_string       = "windows";
    const std::string executable_extension = ".exe";
    const std::string host_os_path_seperator = ";";
    const auto async_launch_option = std::launch::async|std::launch::deferred;
    #elif defined(__APPLE__)
    const std::string host_os_string      = "macos";
    const std::string executable_extension = "";
    const std::string host_os_path_seperator = ":";
    const auto async_launch_option = std::launch::async|std::launch::deferred; // Unsure
    #elif defined (__linux__)
    const std::string host_os_string       = "linux";
    const std::string executable_extension = "";
    const std::string host_os_path_seperator = ":";
    const auto async_launch_option = std::launch::deferred;
    #endif

    static std::string component_dotname_to_id(const std::string dotname)
    {
        return dotname.find_last_of(".") != std::string::npos ? dotname.substr(dotname.find_last_of(".")+1) : dotname;
    }

    void fetch_component(const std::string& name, YAML::Node node, std::function<void(size_t)> progress_handler);
}