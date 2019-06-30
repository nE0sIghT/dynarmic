/* This file is part of the dynarmic project.
 * Copyright (c) 2018 MerryMage
 * This software may be used and distributed according to the terms of the GNU
 * General Public License version 2 or any later version.
 */

#include <catch.hpp>

#include "A64/testenv.h"

long Print() {
    printf("mew!\n");
    return 123;
}

double Print2(double x) {
    printf("mew! %g\n", x);
    return -1.0;
}

TEST_CASE("A64: HLE", "[a64]") {
    A64TestEnv env;
    Dynarmic::A64::Jit jit{Dynarmic::A64::UserConfig{&env}};

    jit.AddHLEFunctions({
        {0xDEAD042, {(void*)&Print, Dynarmic::A64::HLE::ArgumentType::Integer, {}}}
    });

    env.code_mem.emplace_back(0x94000002); // BL #0x8
    env.code_mem.emplace_back(0x14000000); // B .

    env.code_mem.emplace_back(0xB006F570); // ADRP X16, #0xDEAD000
    env.code_mem.emplace_back(0xF8442211); // LDR X17, [X16, 0x42]
    env.code_mem.emplace_back(0x91010A10); // ADD X16, X16, 0x42
    env.code_mem.emplace_back(0xD61F0220); // BR X17

    env.ticks_left = 6;
    jit.Run();

    REQUIRE(jit.GetRegister(0) == 123);
    REQUIRE(jit.GetPC() == 4);
}

TEST_CASE("A64: HLE 2", "[a64]") {
    A64TestEnv env;
    Dynarmic::A64::Jit jit{Dynarmic::A64::UserConfig{&env}};

    jit.AddHLEFunctions({
        {0xDEAD042, {(void*)&Print2, Dynarmic::A64::HLE::ArgumentType::Float, {Dynarmic::A64::HLE::ArgumentType::Float}}}
    });

    env.code_mem.emplace_back(0x94000002); // BL #0x8
    env.code_mem.emplace_back(0x14000000); // B .

    env.code_mem.emplace_back(0xB006F570); // ADRP X16, #0xDEAD000
    env.code_mem.emplace_back(0xF8442211); // LDR X17, [X16, 0x42]
    env.code_mem.emplace_back(0x91010A10); // ADD X16, X16, 0x42
    env.code_mem.emplace_back(0xD61F0220); // BR X17

    jit.SetVector(0, {0x4045400000000000, 0});

    env.ticks_left = 6;
    jit.Run();

    REQUIRE(jit.GetVector(0)[0] == 0xBFF0000000000000);
    REQUIRE(jit.GetPC() == 4);
}

