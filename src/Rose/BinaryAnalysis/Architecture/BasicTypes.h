#ifndef ROSE_BinaryAnalysis_Architecture_BasicTypes_H
#define ROSE_BinaryAnalysis_Architecture_BasicTypes_H
#include <featureTests.h>
#ifdef ROSE_ENABLE_BINARY_ANALYSIS

#include <Sawyer/Message.h>
#include <memory>
#include <set>
#include <string>
#include <vector>

// Basic types needed by almost all architectures

namespace Rose {
namespace BinaryAnalysis {
namespace Architecture {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward class declarations and their reference-counting pointers.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class Amd64;

/** Reference counted pointer for @ref Amd64. */
using Amd64Ptr = std::shared_ptr<Amd64>;

class ArmAarch32;

/** Reference counted pointer for @ref ArmAarch32. */
using ArmAarch32Ptr = std::shared_ptr<ArmAarch32>;

class ArmAarch64;

/** Reference counted pointer for @ref ArmAarch64. */
using ArmAarch64Ptr = std::shared_ptr<ArmAarch64>;

class Base;

/** Reference counted pointer for @ref Base. */
using BasePtr = std::shared_ptr<Base>;

class Exception;

/** Reference counted pointer for @ref Exception. */
using ExceptionPtr = std::shared_ptr<Exception>;

class Intel80286;

/** Reference counted pointer for @ref Intel80286. */
using Intel80286Ptr = std::shared_ptr<Intel80286>;

class Intel8086;

/** Reference counted pointer for @ref Intel8086. */
using Intel8086Ptr = std::shared_ptr<Intel8086>;

class Intel8088;

/** Reference counted pointer for @ref Intel8088. */
using Intel8088Ptr = std::shared_ptr<Intel8088>;

class IntelI386;

/** Reference counted pointer for @ref IntelI386. */
using IntelI386Ptr = std::shared_ptr<IntelI386>;

class IntelI486;

/** Reference counted pointer for @ref IntelI486. */
using IntelI486Ptr = std::shared_ptr<IntelI486>;

class IntelPentium;

/** Reference counted pointer for @ref IntelPentium. */
using IntelPentiumPtr = std::shared_ptr<IntelPentium>;

class IntelPentiumii;

/** Reference counted pointer for @ref IntelPentiumii. */
using IntelPentiumiiPtr = std::shared_ptr<IntelPentiumii>;

class IntelPentiumiii;

/** Reference counted pointer for @ref IntelPentiumiii. */
using IntelPentiumiiiPtr = std::shared_ptr<IntelPentiumiii>;

class IntelPentium4;

/** Reference counted pointer for @ref IntelPentium4. */
using IntelPentium4Ptr = std::shared_ptr<IntelPentium4>;

class Mips32;

/** Reference counted pointer for @ref Mips32. */
using Mips32Ptr = std::shared_ptr<Mips32>;

class Motorola68040;

/** Reference counted pointer for @ref Motorola68040. */
using Motorola68040Ptr = std::shared_ptr<Motorola68040>;

class NxpColdfire;

/** Reference counted pointer for @ref NxpColdfire. */
using NxpColdfirePtr = std::shared_ptr<NxpColdfire>;

class Powerpc32;

/** Reference counted pointer for @ref Powerpc32. */
using Powerpc32Ptr = std::shared_ptr<Powerpc32>;

class Powerpc64;

/** Reference counted pointer for @ref Powerpc64. */
using Powerpc64Ptr = std::shared_ptr<Powerpc64>;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Diagnostics
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Diagnostic facility for architecture definitions. */
extern Sawyer::Message::Facility mlog;

/** Initialize and registers architecture diagnostic streams.
 *
 *  See @ref Rose::Diagnostics::initialize. */
void initDiagnostics();

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Subclass registration functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Register a new architecture definition.
 *
 *  The specified definition is added to the ROSE library. When searching for an architecture, architectures registered later
 *  are preferred over architectures registered earlier.
 *
 *  Thread safety: This function is thread safe. */
void registerDefinition(const BasePtr&);

/** Remove the specified architecture from the list of registered architectures.
 *
 *  If the specified architecture object is found, then the latest such object is removed from the registration. This function
 *  is a no-ip if the argument is a null pointer.
 *
 *  Returns true if any architecture definition object was removed, false if the object was not found.
 *
 *  Thread safety: This function is thread safe. */
bool deregisterDefinition(const BasePtr&);

/** Registered architectures.
 *
 *  Returns the registered architectures in the order they were registered.
 *
 *  Thread safety: This function is thread safe. */
std::vector<BasePtr> registeredDefinitions();

/** Names of all registered architectures.
 *
 *  Returns the names of all registered architectures. This is returned as a set, although there is no requirement that the
 *  registered architectures have unique names.
 *
 *  Thread safety: This function is thread safe. */
std::set<std::string> registeredNames();

/** Look up a new architecture by name.
 *
 *  Returns the latest registered architecture having the specified name. If no matching architecture is found then a null pointer
 *  is returned.
 *
 *  Thread safety: This function is thread safe. */
BasePtr findByName(const std::string&);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Helper functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Architecture name free function.
 *
 *  This is a convenient way to obtain an architecture definition's name without having to include "Base.h", and is therefore useful
 *  in header files that try to include a minimal number of type definitions. Returns a null pointer if the argument is a null
 *  pointer. */
const std::string& name(const BasePtr&);

} // namespace
} // namespace
} // namespace

#endif
#endif
