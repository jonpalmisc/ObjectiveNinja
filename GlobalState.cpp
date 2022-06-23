/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#include "GlobalState.h"

#include <set>
#include <unordered_map>

static std::unordered_map<BinaryViewID, SharedAnalysisInfo> g_analysisRecords;
static std::set<BinaryViewID> g_ignoredViews;

std::uintptr_t GlobalState::id(BinaryViewRef bv)
{
    return reinterpret_cast<uintptr_t>(bv->GetObject());
}

void GlobalState::storeAnalysisInfo(BinaryViewRef bv, SharedAnalysisInfo records)
{
    g_analysisRecords[id(std::move(bv))] = std::move(records);
}

SharedAnalysisInfo GlobalState::analysisInfo(BinaryViewRef bv)
{
    if (hasAnalysisInfo(bv))
        return g_analysisRecords[id(bv)];

    return nullptr;
}

bool GlobalState::hasAnalysisInfo(BinaryViewRef bv)
{
    return g_analysisRecords.count(id(std::move(bv))) > 0;
}

void GlobalState::addIgnoredView(BinaryViewRef bv)
{
    g_ignoredViews.insert(id(std::move(bv)));
}

bool GlobalState::viewIsIgnored(BinaryViewRef bv)
{
    return g_ignoredViews.count(id(std::move(bv))) > 0;
}

bool GlobalState::hasFlag(BinaryViewRef bv, const std::string& flag)
{
    return bv->QueryMetadata(flag);
}

void GlobalState::setFlag(BinaryViewRef bv, const std::string& flag)
{
    bv->StoreMetadata(flag, new BinaryNinja::Metadata("YES"));
}
