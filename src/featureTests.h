// This file defines various C preprocessor macros that describe what ROSE features are enabled in this configuration. The
// advantage of doing this in a C header instead of calculating it in the configuration system and storing a result is that
// this same logic can then be used across all configuration and build systems.
//
// All ROSE feature macros start with the letters "ROSE_ENABLED_" followed by the name of the feature. These macros are defined
// if the feature is enabled, and not defined (but also not #undef) if the feature is not automatically enabled. By not
// explicitly undefining the macro, we make it possible for developers to enabled features from the C++ compiler command-line
// that would not normally be enabled.

#ifndef ROSE_FeatureTests_H
#define ROSE_FeatureTests_H

// DO NOT INCLUDE LARGE HEADERS HERE! These headers should generally be only C preprocessor directives, not any substantial
// amount of C++ code. This means no sage3basic.h or rose.h, among others. This <featureTests.h> file is meant to be as small
// and fast as possible because its purpose is to be able to quickly compile (by skipping over) source code that's not
// necessary in a particular ROSE configuration.
#include <rosePublicConfig.h>
#include <boost/version.hpp>

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Binary analysis features
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef ROSE_BUILD_BINARY_ANALYSIS_SUPPORT

// ARM AArch64 A64 instructions (Sage nodes, disassembly, unparsing, semantics, etc.)
#if !defined(ROSE_ENABLE_ASM_AARCH64) && __cplusplus >= 201103L && defined(ROSE_HAVE_CAPSTONE)
    #define ROSE_ENABLE_ASM_AARCH64
#endif

// ARM AArch32 instructions (Sage nodes, disassembly, unparsing, semantics, etc.)
#if !defined(ROSE_ENABLE_ASM_AARCH32) && __cplusplus >= 201103L && defined(ROSE_HAVE_CAPSTONE)
    #define ROSE_ENABLE_ASM_AARCH32
#endif

// Whether to enable concolic testing.
#if !defined(ROSE_ENABLE_CONCOLIC_TESTING) && \
    __cplusplus >= 201402L && \
    (defined(ROSE_HAVE_SQLITE3) || defined(ROSE_HAVE_LIBPQXX)) && \
    BOOST_VERSION >= 106400 && \
    defined(ROSE_HAVE_BOOST_SERIALIZATION_LIB)
#define ROSE_ENABLE_CONCOLIC_TESTING
#endif

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// C/C++ analysis features
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
