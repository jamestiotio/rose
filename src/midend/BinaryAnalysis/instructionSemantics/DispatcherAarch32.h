#ifndef ROSE_BinaryAnalysis_InstructionSemantics2_DispatcherAarch32_H
#define ROSE_BinaryAnalysis_InstructionSemantics2_DispatcherAarch32_H
#include <featureTests.h>
#ifdef ROSE_ENABLE_ASM_AARCH32

#include <BaseSemantics2.h>

#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/split_member.hpp>

namespace Rose {
namespace BinaryAnalysis {
namespace InstructionSemantics2 {

/** Shared-ownership pointer to an A32/T32 instruction dispatcher. See @ref heap_object_shared_ownership. */
using DispatcherAarch32Ptr = boost::shared_ptr<class DispatcherAarch32>;

class DispatcherAarch32: public BaseSemantics::Dispatcher {
public:
    using Super = BaseSemantics::Dispatcher;

#ifdef ROSE_HAVE_BOOST_SERIALIZATION_LIB
private:
    friend class boost::serialization::access;

    template<class S>
    void save(S &s, const unsigned /*version*/) const {
        s & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Super);
    }

    template<class S>
    void load(S &s, const unsigned /*version*/) {
        s & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Super);
        initializeRegisterDescriptors();
        initializeInsnDispatchTable();
        initializeMemory();
    }

    BOOST_SERIALIZATION_SPLIT_MEMBER();
#endif

protected:
    // prototypical constructor
    DispatcherAarch32()
        :BaseSemantics::Dispatcher(64, RegisterDictionary::dictionary_aarch32()) {}

    DispatcherAarch32(const BaseSemantics::RiscOperatorsPtr &ops, const RegisterDictionary *regs)
        : BaseSemantics::Dispatcher(ops, 32, regs ? regs : RegisterDictionary::dictionary_aarch32()) {
        initializeRegisterDescriptors();
        initializeInsnDispatchTable();
        initializeMemory();
        initializeState(ops->currentState());
    }

public:
    /** Construct a prototypical dispatcher.
     *
     *  The only thing this dispatcher can be used for is to create another dispatcher with the virtual @ref create method. */
    static DispatcherAarch32Ptr instance() {
        return DispatcherAarch32Ptr(new DispatcherAarch32);
    }

    /** Allocating constructor. */
    static DispatcherAarch32Ptr instance(const BaseSemantics::RiscOperatorsPtr &ops, const RegisterDictionary *regs = nullptr) {
        return DispatcherAarch32Ptr(new DispatcherAarch32(ops, regs));
    }

    /** Virtual constructor. */
    virtual BaseSemantics::DispatcherPtr create(const BaseSemantics::RiscOperatorsPtr &ops, size_t addrWidth = 0,
                                                const RegisterDictionary *regs = nullptr) const override {
        ASSERT_require(0 == addrWidth || 32 == addrWidth);
        return instance(ops, regs);
    }

    /** Dynamic cast to DispatcherAarch32 with assertion. */
    static DispatcherAarch32Ptr promote(const BaseSemantics::DispatcherPtr &d) {
        DispatcherAarch32Ptr retval = boost::dynamic_pointer_cast<DispatcherAarch32>(d);
        ASSERT_not_null(retval);
        return retval;
    }

protected:
    /** Initialized cached register descriptors from the register dictionary. */
    void initializeRegisterDescriptors();

    /** Initializes the instruction dispatch table.
     *
     *  This is called from the constructor. */
    void initializeInsnDispatchTable();

    /** Make sure memory is configured correctly, such as setting the byte order. */
    void initializeMemory();

protected:
    int iproc_key(SgAsmInstruction*) const override;
    RegisterDescriptor instructionPointerRegister() const override;
    RegisterDescriptor stackPointerRegister() const override;
    RegisterDescriptor callReturnRegister() const override;
    void set_register_dictionary(const RegisterDictionary*) override;
};

} // namespace
} // namespace
} // namespace

#ifdef ROSE_HAVE_BOOST_SERIALIZATION_LIB
BOOST_CLASS_EXPORT_KEY(Rose::BinaryAnalysis::InstructionSemantics2::DispatcherAarch32);
#endif

#endif
#endif
