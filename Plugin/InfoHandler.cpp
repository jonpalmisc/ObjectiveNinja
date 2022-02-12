/*
 * Copyright (c) 2022 Jon Palmisciano. All rights reserved.
 *
 * Use of this source code is governed by the BSD 3-Clause license; the full
 * terms of the license can be found in the LICENSE.txt file.
 */

#include "InfoHandler.h"

#include "CustomTypes.h"

#include <algorithm>
#include <regex>

using namespace BinaryNinja;

std::string InfoHandler::sanitizeText(const std::string& text)
{
    std::string result;
    std::string input = text.substr(0, 24);

    std::regex re("[a-zA-Z0-9]+");
    std::smatch sm;
    while (std::regex_search(input, sm, re)) {
        std::string part = sm[0];
        part[0] = static_cast<char>(std::toupper(part[0]));

        result += part;
        input = sm.suffix();
    }

    return result;
}

std::string InfoHandler::sanitizeSelector(const std::string& text)
{
    auto result = text;
    std::replace(result.begin(), result.end(), ':', '_');

    return result;
}

TypeRef InfoHandler::namedType(BinaryViewRef bv, const std::string& name)
{
    return Type::NamedType(bv, name);
}

TypeRef InfoHandler::stringType(size_t size)
{
    return Type::ArrayType(Type::IntegerType(1, true), size + 1);
}

void InfoHandler::defineVariable(BinaryViewRef bv, uint64_t address, TypeRef type)
{
    bv->DefineUserDataVariable(address, type);
}

void InfoHandler::defineSymbol(BinaryViewRef bv, uint64_t address, const std::string& name,
    const std::string& prefix, BNSymbolType symbolType)
{
    bv->DefineUserSymbol(new Symbol(symbolType, prefix + name, address));
}

void InfoHandler::defineReference(BinaryViewRef bv, uint64_t from, uint64_t to)
{
    bv->AddUserDataReference(from, to);
}

void InfoHandler::applyMethodType(BinaryViewRef bv, const ObjectiveNinja::ClassInfo& ci,
    const ObjectiveNinja::MethodInfo& mi)
{
    auto selectorTokens = mi.selectorTokens();
    auto typeTokens = mi.decodedTypeTokens();

    // Shorthand for formatting an individual "part" of the type signature.
    auto partForIndex = [selectorTokens, typeTokens](size_t i) {
        std::string argName;

        if (i == 0)
            argName = "";
        else if (i == 1)
            argName = "self";
        else if (i == 2)
            argName = "sel";
        else if (i - 3 < selectorTokens.size())
            argName = selectorTokens[i - 3];

        return typeTokens[i] + " " + argName;
    };

    // Build the type string for the method.
    std::string typeString;
    for (size_t i = 0; i < typeTokens.size(); ++i) {
        auto part = partForIndex(i);

        std::string suffix;
        if (i == 0)
            suffix = " (";
        else if (i == typeTokens.size() - 1)
            suffix = ")";
        else
            suffix = ", ";

        typeString += part + suffix;
    }

    // Attempt to parse the type string that was just built.
    QualifiedNameAndType functionNat;
    std::string errors;
    if (!bv->ParseTypeString(typeString, functionNat, errors))
        return;

    // Search for the method's implementation function; apply the type if found.
    if (auto f = bv->GetAnalysisFunction(bv->GetDefaultPlatform(), mi.implAddress))
        f->SetUserType(functionNat.type);

    auto name = ci.name + "_" + sanitizeSelector(mi.selector);
    defineSymbol(bv, mi.implAddress, name, "", FunctionSymbol);
}

void InfoHandler::applyInfoToView(SharedAnalysisInfo info, BinaryViewRef bv)
{
    BinaryReader reader(bv);

    auto taggedPointerType = namedType(bv, CustomTypes::TaggedPointer);
    auto cfStringType = namedType(bv, CustomTypes::CFString);
    auto classType = namedType(bv, CustomTypes::Class);
    auto classDataType = namedType(bv, CustomTypes::ClassRO);
    auto methodListType = namedType(bv, CustomTypes::MethodList);

    // Create data variables and symbols for all CFString instances.
    for (const auto& csi : info->cfStrings) {
        reader.Seek(csi.dataAddress);
        auto text = reader.ReadString(csi.size + 1);
        auto sanitizedText = sanitizeText(text);

        defineVariable(bv, csi.address, cfStringType);
        defineVariable(bv, csi.dataAddress, stringType(csi.size));
        defineSymbol(bv, csi.address, sanitizedText, "cf_");
        defineSymbol(bv, csi.dataAddress, sanitizedText, "as_");

        defineReference(bv, csi.address, csi.dataAddress);
    }

    // Create data variables and symbols for selectors and selector references.
    for (const auto& sr : info->selectorRefs) {
        auto sanitizedSelector = sanitizeSelector(sr->name);

        defineVariable(bv, sr->address, taggedPointerType);
        defineVariable(bv, sr->nameAddress, stringType(sr->name.size()));
        defineSymbol(bv, sr->address, sanitizedSelector, "sr_");
        defineSymbol(bv, sr->nameAddress, sanitizedSelector, "sl_");

        defineReference(bv, sr->address, sr->nameAddress);
    }

    // Create data variables and symbols for the analyzed classes.
    for (const auto& ci : info->classes) {
        defineVariable(bv, ci.listPointer, taggedPointerType);
        defineVariable(bv, ci.address, classType);
        defineVariable(bv, ci.dataAddress, classDataType);
        defineVariable(bv, ci.nameAddress, stringType(ci.name.size()));
        defineSymbol(bv, ci.listPointer, ci.name, "cp_");
        defineSymbol(bv, ci.address, ci.name, "cl_");
        defineSymbol(bv, ci.dataAddress, ci.name, "ro_");
        defineSymbol(bv, ci.nameAddress, ci.name, "nm_");

        defineReference(bv, ci.listPointer, ci.address);
        defineReference(bv, ci.address, ci.dataAddress);
        defineReference(bv, ci.dataAddress, ci.nameAddress);
        defineReference(bv, ci.dataAddress, ci.methodListAddress);

        if (ci.methodList.address == 0 || ci.methodList.methods.empty())
            continue;

        auto methodType = ci.methodList.hasRelativeOffsets()
            ? bv->GetTypeByName(std::string("objc_method_entry_t"))
            : bv->GetTypeByName(std::string("objc_method_t"));

        // Create data variables for each method in the method list.
        for (const auto& mi : ci.methodList.methods) {
            defineVariable(bv, mi.address, methodType);
            defineSymbol(bv, mi.address, sanitizeSelector(mi.selector), "mt_");
            defineVariable(bv, mi.typeAddress, stringType(mi.type.size()));

            defineReference(bv, ci.methodList.address, mi.address);
            defineReference(bv, mi.address, mi.nameAddress);
            defineReference(bv, mi.address, mi.typeAddress);
            defineReference(bv, mi.address, mi.implAddress);

            applyMethodType(bv, ci, mi);
        }

        // Create a data variable and symbol for the method list header.
        defineVariable(bv, ci.methodListAddress, methodListType);
        defineSymbol(bv, ci.methodListAddress, ci.name, "ml_");
    }
}
