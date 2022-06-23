/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#include "Analyzer.h"

using namespace ObjectiveNinja;

Analyzer::Analyzer(SharedAnalysisInfo info, SharedAbstractFile file)
    : m_info(std::move(info))
    , m_file(std::move(file))
{
}
