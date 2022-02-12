/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#include "BuildInfo.h"
#include "Commands.h"
#include "DataRenderers.h"
#include "Workflow/Workflow.h"

extern "C" {

BN_DECLARE_CORE_ABI_VERSION

BINARYNINJAPLUGIN bool CorePluginInit()
{
    TaggedPointerDataRenderer::Register();
    FastPointerDataRenderer::Register();
    RelativePointerDataRenderer::Register();

    Workflow::registerActivities();
    Commands::registerCommands();

    BinaryNinja::LogInfo("Objective Ninja loaded successfully (%s-%s/%s)",
        GitBranch, GitCommit, BuildType);

    return true;
}
}
