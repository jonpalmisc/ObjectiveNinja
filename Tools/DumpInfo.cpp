/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#include <ObjectiveNinjaCore/AnalysisInfo.h>
#include <ObjectiveNinjaCore/AnalysisProvider.h>
#include <ObjectiveNinjaCore/Analyzer.h>

#include <ObjectiveNinjaCore/Analyzers/CFStringAnalyzer.h>
#include <ObjectiveNinjaCore/Analyzers/ClassAnalyzer.h>
#include <ObjectiveNinjaCore/Analyzers/SelectorAnalyzer.h>

#include <ObjectiveNinjaCore/Support/BinaryViewFile.h>

#include <binaryninjaapi.h>

#include <iostream>
#include <string>
#include <vector>

namespace BN = BinaryNinja;

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cout << "Usage: DumpInfo INPUT_BINARY\n";
        return 0;
    }

    BN::InitPlugins();

    // Load the input file.
    BN::Ref<BN::BinaryData> data = new BN::BinaryData(new BN::FileMetadata(), argv[1]);

    // Create a new BinaryView, automatically determining the view type.
    BN::Ref<BN::BinaryView> bv;
    for (const auto& type : BN::BinaryViewType::GetViewTypes()) {
        if (type->IsTypeValidForData(data) && type->GetName() != "Raw") {
            bv = type->Create(data);
            break;
        }
    }

    // Quit if the input binary is invalid.
    if (!bv || bv->GetTypeName() == "Raw") {
        std::cerr << "Error: Invalid input binary.\n";
        return 1;
    }

    // Create a wrapper over the BinaryView and run Objective-C analysis.
    auto file = std::make_shared<ObjectiveNinja::BinaryViewFile>(bv);
    auto info = ObjectiveNinja::AnalysisProvider::infoForFile(file);

    // Dump the analysis results as JSON.
    std::cout << info->dump() << "\n";

    BNShutdown();
}
