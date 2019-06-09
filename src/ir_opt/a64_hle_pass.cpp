/* This file is part of the dynarmic project.
 * Copyright (c) 2016 MerryMage
 * This software may be used and distributed according to the terms of the GNU
 * General Public License version 2 or any later version.
 */

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <optional>

#include <dynarmic/A64/config.h>

#include "common/common_types.h"
#include "frontend/A64/location_descriptor.h"
#include "frontend/A64/translate/translate.h"
#include "frontend/A64/types.h"
#include "frontend/ir/basic_block.h"
#include "frontend/ir/opcodes.h"
#include "ir_opt/passes.h"

namespace Dynarmic::Optimization {

static std::optional<u64> DoesDestinationMatchStub(IR::Block& caller, const A64::UserConfig& conf) {
    const auto caller_terminal = caller.GetTerminal();
    if (auto term = boost::get<IR::Term::LinkBlock>(&caller_terminal)) {
        const auto get_code = [&conf](u64 vaddr) { return conf.callbacks->MemoryReadCode(vaddr); };
        IR::Block callee = A64::Translate(A64::LocationDescriptor{term->next}, get_code, {conf.define_unpredictable_behaviour});
        Optimization::A64GetSetElimination(callee);
        Optimization::ConstantPropagation(callee);
        Optimization::DeadCodeElimination(callee);
        Optimization::VerificationPass(callee);

        const auto callee_terminal = callee.GetTerminal();
        if (!boost::get<IR::Term::FastDispatchHint>(&callee_terminal)) {
            return {};
        }

        if (callee.empty()) {
            return {};
        }

        const auto set_pc = &callee.back();
        if (set_pc->GetOpcode() != IR::Opcode::A64SetPC) {
            return {};
        }

        if (set_pc->GetArg(0).IsImmediate()) {
            return {};
        }

        const auto read_memory = set_pc->GetArg(0).GetInstIgnoreIdentity();
        if (read_memory->GetOpcode() != IR::Opcode::A64ReadMemory64) {
            return {};
        }

        if (!read_memory->GetArg(0).IsImmediate()) {
            return {};
        }

        const u64 read_location = read_memory->GetArg(0).GetU64();

        fmt::print("function stub for {} at {}:\n", read_location, A64::LocationDescriptor{term->next});
        fmt::print("{}\n", IR::DumpBlock(callee));

        for (auto& inst : callee) {
            if (!inst.MayHaveSideEffects()) {
                continue;
            }
            if (&inst == set_pc || &inst == read_memory) {
                continue;
            }
            switch (inst.GetOpcode()) {
            case IR::Opcode::A64SetW:
            case IR::Opcode::A64SetX:
                // Intra-procedure-call temporary registers (AArch64 ABI)
                if (inst.GetArg(0).GetA64RegRef() == A64::Reg::R16 || inst.GetArg(0).GetA64RegRef() == A64::Reg::R17) {
                    continue;
                }
            default:
                break;
            }

            fmt::print("FAILED!\n");
            return {};
        }

        fmt::print("PASSED!\n");
        return read_location;
    }
    return {};
}

void A64HLEPass(IR::Block& block, const A64::UserConfig& conf) {
    const auto read_location = DoesDestinationMatchStub(block, conf);
    if (!read_location) {
        return;
    }
}

} // namespace Dynarmic::Optimization
