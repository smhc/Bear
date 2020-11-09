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

#pragma once

#include "libresult/Result.h"

#include <climits>
#include <string_view>

namespace el {

    /**
     * This class implements the logic how the program execution resolves the
     * executable path from the system environment.
     *
     * The resolution logic implemented as a class to be able to unit test
     * the code and avoid memory allocation.
     */
    class Resolver {
    public:
        Resolver() noexcept;
        virtual ~Resolver() noexcept = default;

        /**
         * Resolve the executable from system environments.
         *
         * @return resolved executable path as absolute path.
         */
        [[nodiscard]]
        virtual rust::Result<const char*, int> from_current_directory(std::string_view const &file);

        [[nodiscard]]
        virtual rust::Result<const char*, int> from_path(std::string_view const &file, char *const *envp);

        [[nodiscard]]
        virtual rust::Result<const char*, int> from_search_path(std::string_view const &file, const char *search_path);

        Resolver(Resolver const &) = delete;
        Resolver(Resolver &&) noexcept = delete;

        Resolver &operator=(Resolver const &) = delete;
        Resolver &&operator=(Resolver &&) noexcept = delete;

    private:
        char result_[PATH_MAX];
    };
}
