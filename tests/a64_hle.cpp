/* This file is part of the dynarmic project.
 * Copyright (c) 2018 MerryMage
 * This software may be used and distributed according to the terms of the GNU
 * General Public License version 2 or any later version.
 */

#include <catch.hpp>

#include "A64/testenv.h"

void Print() {
    printf("mew!\n");
}

TEST_CASE("A64: HLE", "[a64]") {
    A64TestEnv env;
    Dynarmic::A64::Jit jit{Dynarmic::A64::UserConfig{&env}};

    jit.AddHLEFunctions({
        {0xDEAD042, {(void*)&Print, {}, {}}}
    });

    env.code_mem.emplace_back(0x94000002); // BL #0x8
    env.code_mem.emplace_back(0x14000000); // B .

    env.code_mem.emplace_back(0xB006F570); // ADRP X16, #0xDEAD000
    env.code_mem.emplace_back(0xF8442211); // LDR X17, [X16, 0x42]
    env.code_mem.emplace_back(0x91010A10); // ADD X16, X16, 0x42
    env.code_mem.emplace_back(0xD61F0220); // BR X17

    env.ticks_left = 6;
    jit.Run();
}
