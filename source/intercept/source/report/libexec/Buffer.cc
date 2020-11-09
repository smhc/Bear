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

#include <algorithm>

#include "report/libexec/Array.h"
#include "report/libexec/Buffer.h"

namespace el {

    char const* Buffer::store(char const* const input) noexcept
    {
        if (input == nullptr)
            return nullptr;

        auto input_end = el::array::end(input) + 1; // include the zero element
        auto top = el::array::copy(input, input_end, top_, end_);
        if (top != nullptr)
            std::swap(top_, top);
        return top;
    }

}
