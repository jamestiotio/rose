#ifndef ROSE_BinaryAnalysis_Architecture_Base_H
#define ROSE_BinaryAnalysis_Architecture_Base_H
#include <featureTests.h>
#ifdef ROSE_ENABLE_BINARY_ANALYSIS

#include <Rose/BinaryAnalysis/Architecture/BasicTypes.h>
#include <Rose/BinaryAnalysis/ByteOrder.h>
#include <Rose/BinaryAnalysis/CallingConvention.h>
#include <Rose/BinaryAnalysis/InstructionSemantics/BaseSemantics/BasicTypes.h>
#include <Rose/BinaryAnalysis/Unparser/Base.h>

namespace Rose {
namespace BinaryAnalysis {
namespace Architecture {

/** Base class for architecture definitions. */
class Base {
public:
    using Ptr = BasePtr;

private:
    std::string name_;                                  // name of architecture
    size_t bytesPerWord_ = 0;
    ByteOrder::Endianness byteOrder_ = ByteOrder::ORDER_UNSPECIFIED;

protected:
    Sawyer::Cached<RegisterDictionaryPtr> registerDictionary_;

protected:
    explicit Base(const std::string &name, size_t bytesPerWord, ByteOrder::Endianness byteOrder);
    virtual ~Base();

public:
    /** Property: Architecture definition name.
     *
     *  The name is used for lookups, but it need not be unique since lookups prefer the latest registered architecture. I.e., if
     *  two architectures A, and B, have the same name, and B was registered after A, then lookup by the name will return
     *  architecture B.
     *
     *  A best practice is to use only characters that are not special in shell scripts since architecture names often appear as
     *  arguments to command-line switches. Also, try to use only lower-case letters, decimal digits and hyphens for consistency
     *  across all architecture names. See the list of ROSE built-in architecture names for ideas (this list can be obtained from
     *  many binary analysis tools, or the @ref Architecture::registeredNames function).
     *
     *  Thread safety: Thread safe. The name is specified during construction and is thereafter read-only. */
    const std::string& name() const;

    /** Property: Word size.
     *
     *  This is the natural word size for the architecture, measured in bits or bytes (depending on the property name).
     *
     *  Thread safety: Thread safe. This property is set during construction and is thereafter read-only.
     *
     * @{ */
    size_t bytesPerWord() const;
    size_t bitsPerWord() const;
    /** @} */

    /** Property: Byte order for memory.
     *
     *  When multi-byte values (such as 32-bit integral values) are stored in memory, this property is the order in which the
     *  value's bytes are stored. If the order is little endian, then the least significant byte is stored at the lowest address; if
     *  the order is big endian then the most significant byte is stored at the lowest address.
     *
     *  Thread safety: Thread safe. This property is set during construction and is thereafter read-only. */
    ByteOrder::Endianness byteOrder() const;

    /** Property: Register dictionary.
     *
     *  The register dictionary defines a mapping between register names and register descriptors (@ref RegisterDescriptor), and
     *  thus how the registers map into hardware.
     *
     *  Since dictionaries are generally not modified, it is permissible for this function to return the same dictionary every time
     *  it's called. The dictionary can be constructed on the first call.
     *
     *  Thread safety: Thread safe. */
    virtual RegisterDictionaryPtr registerDictionary() const = 0;
};

} // namespace
} // namespace
} // namespace

#endif
#endif
