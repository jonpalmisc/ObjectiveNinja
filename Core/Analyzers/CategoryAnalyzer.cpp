#include <ObjectiveNinjaCore/Analyzers/ClassAnalyzer.h>
#include <ObjectiveNinjaCore/Analyzers/CategoryAnalyzer.h>

#include <ObjectiveNinjaCore/TypeParser.h>

using namespace ObjectiveNinja;


CategoryAnalyzer::CategoryAnalyzer(SharedAnalysisInfo info,
    SharedAbstractFile file)
    : Analyzer(std::move(info), std::move(file))
{
}

MethodListInfo CategoryAnalyzer::analyzeMethodList(uint64_t address)
{
    MethodListInfo mli;
    mli.address = address;
    mli.flags = m_file->readInt(mli.address);

    auto methodCount = m_file->readInt(mli.address + 0x4);
    auto methodSize = mli.hasRelativeOffsets() ? 12 : 24;

    for (unsigned i = 0; i < methodCount; ++i) {
        MethodInfo mi;
        mi.address = mli.address + 8 + (i * methodSize);

        m_file->seek(mi.address);

        if (mli.hasRelativeOffsets()) {
            mi.nameAddress = mi.address + static_cast<int32_t>(m_file->readInt());
            mi.typeAddress = mi.address + 4 + static_cast<int32_t>(m_file->readInt());
            mi.implAddress = mi.address + 8 + static_cast<int32_t>(m_file->readInt());
        } else {
            mi.nameAddress = arp(m_file->readLong());
            mi.typeAddress = arp(m_file->readLong());
            mi.implAddress = arp(m_file->readLong());
        }

        if (!mli.hasRelativeOffsets() || mli.hasDirectSelectors()) {
            mi.selector = m_file->readStringAt(mi.nameAddress);
        } else {
            auto selectorNamePointer = arp(m_file->readLong(mi.nameAddress));
            mi.selector = m_file->readStringAt(selectorNamePointer);
        }

        mi.type = m_file->readStringAt(mi.typeAddress);

        m_info->methodImpls[mi.nameAddress] = mi.implAddress;

        mli.methods.emplace_back(mi);
    }

    return mli;
}

void CategoryAnalyzer::run()
{
    const auto sectionStart = m_file->sectionStart("__objc_catlist");
    const auto sectionEnd = m_file->sectionEnd("__objc_catlist");
    if (sectionStart == 0 || sectionEnd == 0)
        return;

    for (auto address = sectionStart; address < sectionEnd; address += 8) {
        CategoryInfo ci;
        ci.listPointer = address;
        ci.address = arp(m_file->readLong(address));
        ci.nameAddress = arp(m_file->readLong(ci.address));
        ci.name = m_file->readStringAt(ci.nameAddress);
        ci.instanceMethodListAddress = arp(m_file->readLong(ci.address + 0x10));
        ci.classMethodListAddress = arp(m_file->readLong(ci.address + 0x18));

        if (ci.instanceMethodListAddress)
            ci.instanceMethods = analyzeMethodList(ci.instanceMethodListAddress);

        if (ci.classMethodListAddress)
            ci.classMethods = analyzeMethodList(ci.classMethodListAddress);

        m_info->categories.emplace_back(ci);
    }
}
