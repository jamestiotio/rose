#ifndef ROSE_BinaryAnalysis_BinaryLoaderPe_H
#define ROSE_BinaryAnalysis_BinaryLoaderPe_H

#include "BinaryLoader.h"

namespace Rose {
namespace BinaryAnalysis {

/** Reference counting pointer to @ref BinaryLoaderPe. */
typedef Sawyer::SharedPointer<class BinaryLoaderPe> BinaryLoaderPePtr;

/** Loader for Windows PE files. */
class BinaryLoaderPe: public BinaryLoader {
public:
    /** Reference counting pointer to @ref BinaryLoaderPe. */
    typedef Sawyer::SharedPointer<BinaryLoaderPe> Ptr;

protected:
    BinaryLoaderPe() {}

    BinaryLoaderPe(const BinaryLoaderPe &other)
        : BinaryLoader(other)
        {}

public:
    /** Allocating constructor. */
    static Ptr instance() {
        return Ptr(new BinaryLoaderPe);
    }

    virtual BinaryLoaderPtr clone() const {
        return BinaryLoaderPtr(new BinaryLoaderPe(*this));
    }

    virtual ~BinaryLoaderPe() {}

    /* Override virtual methods from BinaryLoader */
public:

    virtual bool can_load(SgAsmGenericHeader*) const;

    /** Returns sections in order of their definition in the PE Section Table. */
    virtual SgAsmGenericSectionPtrList get_remap_sections(SgAsmGenericHeader*);

    /** Windows-specific PE section alignment. */
    virtual MappingContribution align_values(SgAsmGenericSection*, const MemoryMap::Ptr&,
                                             rose_addr_t *malign_lo, rose_addr_t *malign_hi,
                                             rose_addr_t *va, rose_addr_t *mem_size,
                                             rose_addr_t *offset, rose_addr_t *file_size, bool *map_private,
                                             rose_addr_t *va_offset, bool *anon_lo, bool *anon_hi,
                                             ConflictResolution *resolve);
};

} // namespace
} // namespace

#endif /*ROSE_BINARYLOADERPE_H*/
