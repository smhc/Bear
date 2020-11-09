/*  Copyright (C) 2012-2020 by László Nagy
    This file is part of Bear.

    Bear is a tool to generate compilation database for clang tooling.

    Bear is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Bear is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "report/libexec/Resolver.h"

#include "report/libexec/Array.h"
#include "report/libexec/Environment.h"
#include "report/libexec/Paths.h"

#include <algorithm>
#include <cerrno>
#include <unistd.h>

namespace {

    constexpr char DIR_SEPARATOR = '/';

    bool contains_dir_separator(std::string_view const &candidate) {
        return std::find(candidate.begin(), candidate.end(), DIR_SEPARATOR) != candidate.end();
    }
}

namespace el {

    Resolver::Resolver() noexcept
            : result_()
    {
        result_[0] = 0;
    }

    rust::Result<const char*, int> Resolver::from_current_directory(std::string_view const &file) {
        // create absolute path to the given file.
        if (nullptr == realpath(file.begin(), result_)) {
            return rust::Err(ENOENT);
        }
        // check if it's okay to execute.
        if (0 == access(result_, X_OK)) {
            const char *ptr = result_;
            return rust::Ok(ptr);
        }
        // try to set a meaningful error value.
        if (0 == access(result_, F_OK)) {
            return rust::Err(EACCES);
        }
        return rust::Err(ENOENT);
    }

    rust::Result<const char*, int> Resolver::from_path(std::string_view const &file, char* const* envp) {
        if (contains_dir_separator(file)) {
            // the file contains a dir separator, it is treated as path.
            return from_current_directory(file);
        } else {
            // otherwise use the PATH variable to locate the executable.
            const char *paths = el::env::get_env_value(const_cast<const char **>(envp), "PATH");
            if (paths != nullptr) {
                return from_search_path(file, paths);
            }
            // fall back to `confstr` PATH value if the environment has no value.
            const size_t search_path_length = confstr(_CS_PATH, nullptr, 0);
            if (search_path_length != 0) {
                char search_path[search_path_length];
                if (confstr(_CS_PATH, search_path, search_path_length) != 0) {
                    return from_search_path(file, search_path);
                }
            }
            return rust::Err(ENOENT);
        }
    }

    rust::Result<const char*, int> Resolver::from_search_path(std::string_view const &file, const char *search_path) {
        if (contains_dir_separator(file)) {
            // the file contains a dir separator, it is treated as path.
            return from_current_directory(file);
        } else {
            // otherwise use the given search path to locate the executable.
            for (auto path : el::Paths(search_path)) {
                // ignore empty entries
                if (path.empty()) {
                    continue;
                }
                // create a path
                char candidate[PATH_MAX];
                {
                    auto it = el::array::copy(path.begin(), path.end(), candidate, candidate + PATH_MAX);
                    *it++ = DIR_SEPARATOR;
                    it = el::array::copy(file.begin(), file.end(), it, candidate + PATH_MAX);
                    *it = 0;
                }
                // check if it's okay to execute.
                if (auto result = from_current_directory(candidate); result.is_ok()) {
                    return result;
                }
            }
            // if all attempt were failing, then quit with a failure.
            return rust::Err(ENOENT);
        }
    }
}
