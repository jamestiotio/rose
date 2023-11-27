#ifndef ROSE_BinaryAnalysis_Unparser_X86_H
#define ROSE_BinaryAnalysis_Unparser_X86_H
#include <featureTests.h>
#ifdef ROSE_ENABLE_BINARY_ANALYSIS
#include <Rose/BinaryAnalysis/Unparser/Base.h>

#include <Rose/BinaryAnalysis/BasicTypes.h>

namespace Rose {
namespace BinaryAnalysis {
namespace Unparser {

std::string unparseX86Mnemonic(SgAsmX86Instruction*);
std::string unparseX86Register(SgAsmInstruction*, RegisterDescriptor, RegisterDictionaryPtr);
std::string unparseX86Register(RegisterDescriptor, const RegisterDictionaryPtr&);
std::string unparseX86Expression(SgAsmExpression*, const LabelMap*, const RegisterDictionaryPtr&, bool leaMode);
std::string unparseX86Expression(SgAsmExpression*, const LabelMap*, const RegisterDictionaryPtr&);

/** %Settings specific to the x86 unparser. */
struct X86Settings: public Settings {};

/** %Unparser for x86 instruction sets. */
class X86: public Base {
    X86Settings settings_;

protected:
    explicit X86(const Architecture::BaseConstPtr&, const X86Settings&);

public:
    ~X86();

public:
    static Ptr instance(const Architecture::BaseConstPtr&, const X86Settings& = X86Settings());

    Ptr copy() const override;

    const X86Settings& settings() const override { return settings_; }
    X86Settings& settings() override { return settings_; }

protected:
    void emitInstructionMnemonic(std::ostream&, SgAsmInstruction*, State&) const override;
    void emitOperandBody(std::ostream&, SgAsmExpression*, State&) const override;
    void emitTypeName(std::ostream&, SgAsmType*, State&) const override;

private:
    void outputExpr(std::ostream&, SgAsmExpression*, State&) const;
};

} // namespace
} // namespace
} // namespace

#endif
#endif
