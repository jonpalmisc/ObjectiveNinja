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

#include "GlobalState.hpp"

#include <mutex>
#include <set>
#include <unordered_map>

static std::unordered_map<BinaryViewID, AnalysisRecords> g_analysisRecords;
static std::set<BinaryViewID> g_ignoredViews;

std::uintptr_t GlobalState::id(BinaryViewRef bv)
{
    return reinterpret_cast<uintptr_t>(bv->GetObject());
}

void GlobalState::storeAnalysisRecords(BinaryViewRef bv, AnalysisRecords records)
{
    g_analysisRecords[id(bv)] = records;
}

AnalysisRecords* GlobalState::analysisRecords(BinaryViewRef bv)
{
    if (hasAnalysisRecords(bv))
        return &g_analysisRecords[id(bv)];

    return nullptr;
}

bool GlobalState::hasAnalysisRecords(BinaryViewRef bv)
{
    return g_analysisRecords.count(id(bv)) > 0;
}

void GlobalState::addIgnoredView(BinaryViewRef bv)
{
    g_ignoredViews.insert(id(bv));
}

bool GlobalState::viewIsIgnored(BinaryViewRef bv)
{
    return g_ignoredViews.count(id(bv)) > 0;
}

bool GlobalState::hasFlag(BinaryViewRef bv, const std::string& flag)
{
    return bv->QueryMetadata(flag);
}

void GlobalState::setFlag(BinaryViewRef bv, const std::string& flag)
{
    bv->StoreMetadata(flag, new BinaryNinja::Metadata("YES"));
}
