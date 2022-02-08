/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#include "Commands.h"
#include "DataRenderers.h"
#include "Workflow/Workflow.h"

#ifndef BUILD_TYPE
#define BUILD_TYPE "Unknown"
#endif

extern "C" {

BN_DECLARE_CORE_ABI_VERSION

BINARYNINJAPLUGIN bool CorePluginInit()
{
    TaggedPointerDataRenderer::Register();
    RelativePointerDataRenderer::Register();

    Workflow::registerActivities();
    Commands::registerCommands();

    BinaryNinja::LogInfo("ObjectiveNinja (%s) loaded successfully.", BUILD_TYPE);

    return true;
}
}
