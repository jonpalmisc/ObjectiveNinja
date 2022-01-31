/*
 * Copyright (c) 2022 Jon Palmisciano
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include <binaryninjaapi.h>

using AnalysisContextRef = BinaryNinja::Ref<BinaryNinja::AnalysisContext>;
using BinaryViewRef = BinaryNinja::Ref<BinaryNinja::BinaryView>;
using BinaryViewID = std::uintptr_t;
using LLILFunctionRef = BinaryNinja::Ref<BinaryNinja::LowLevelILFunction>;

/// Namespace to hold activity ID constants.
namespace ActivityID {
constexpr auto InlineMethodCalls = "objectiveNinja.function.inlineMethodCalls";
};

/// Workflow-related procedures.
class Workflow {

    /// Get the addresses of all usable `_objc_msgSend` functions.
    ///
    /// Not all `_objc_msgSend` functions in the binary are usable, and there
    /// may also be more than one usable candidate. For additional details and
    /// specifics, see this function's implementation.
    static std::set<uint64_t> findMsgSendFunctions(BinaryViewRef);

    /// Attempt to rewrite the `objc_msgSend` call at `insnIndex` with a direct
    /// call to the requested method's implementation.
    ///
    /// \param insnIndex The index of the `LLIL_CALL` instruction to rewrite
    static void rewriteMethodCall(LLILFunctionRef, size_t insnIndex);

public:
    /// Attempt to inline all `objc_msgSend` calls in the given analysis context.
    static void inlineMethodCalls(AnalysisContextRef);

    /// Register the Objective Ninja workflow and all activities.
    ///
    /// This is named a bit strangely because `register` is a keyword in C++ and
    /// therefore an invalid method name, and I refuse to misspell it to appease
    /// the compiler and avoid the conflict.
    static void registerActivities();
};
