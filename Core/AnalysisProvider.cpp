/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#include <ObjectiveNinjaCore/AnalysisProvider.h>

#include <ObjectiveNinjaCore/Analyzers/CFStringAnalyzer.h>
#include <ObjectiveNinjaCore/Analyzers/ClassAnalyzer.h>
#include <ObjectiveNinjaCore/Analyzers/SelectorAnalyzer.h>

namespace ObjectiveNinja {

SharedAnalysisInfo AnalysisProvider::infoForFile(SharedAbstractFile file)
{
    auto info = std::make_shared<ObjectiveNinja::AnalysisInfo>();

    std::vector<std::unique_ptr<ObjectiveNinja::Analyzer>> analyzers;
    analyzers.emplace_back(new SelectorAnalyzer(info, file));
    analyzers.emplace_back(new ClassAnalyzer(info, file));
    analyzers.emplace_back(new CFStringAnalyzer(info, file));

    for (const auto& analyzer : analyzers)
        analyzer->run();

    return info;
}

}
