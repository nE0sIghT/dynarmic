/* This file is part of the dynarmic project.
 * Copyright (c) 2018 MerryMage
 * This software may be used and distributed according to the terms of the GNU
 * General Public License version 2 or any later version.
 */

#pragma once

#include <cstdint>
#include <map>
#include <optional>
#include <vector>

namespace Dynarmic {
namespace A64 {
namespace HLE {

enum class ArgumentType {
    Integer,
    Float,
};

struct Function {
    void* pointer;
    std::optional<ArgumentType> return_type;
    std::vector<ArgumentType> argument_types;
};

using FunctionMap = std::map<std::uint64_t, Function>;

} // namespace HLE
} // namespace A64
} // namespace Dynarmic
