#pragma once

#include <boost/filesystem.hpp>
#include <string>

namespace fs = boost::filesystem;

extern int parser_error_pos;

/**
 * Initialize library path.
 */
void parser_init();

fs::path search_libs(const fs::path &localpath);
fs::path find_valid_path(
    const fs::path &sourcepath, const fs::path &localpath,
    const std::vector<std::string> *openfilenames = nullptr);
