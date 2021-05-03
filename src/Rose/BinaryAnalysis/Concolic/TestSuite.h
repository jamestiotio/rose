#ifndef ROSE_BinaryAnalysis_Concolic_TestSuite_H
#define ROSE_BinaryAnalysis_Concolic_TestSuite_H
#include <featureTests.h>
#ifdef ROSE_ENABLE_CONCOLIC_TESTING
#include <Rose/BinaryAnalysis/Concolic/BasicTypes.h>

#include <Sawyer/SharedObject.h>
#include <Sawyer/SharedPointer.h>
#include <string>

namespace Rose {
namespace BinaryAnalysis {
namespace Concolic {

/** Test suite.
 *
 *  A <em>test suite</em> is a coherent collection of test cases. The test suite usually starts with a single "seed" test case
 *  and contains additional test cases generated by the concolic executor. All test cases within a test suite use the same
 *  concrete executor and measure the same user-defined execution properties. For example, the database might contain one test
 *  suite based on "/bin/grep" and another test suite running "/bin/cat".  Or it might have two test suites both running
 *  "/bin/grep" but one always using "--extended-regexp" and the other always using "--basic-regexp".  Or it might have two
 *  test suites both running "/bin/cat" but one measures exit status and the other measures code coverage.
 *
 *  A @ref Database has a "current test suite" set/queried by its @ref Database::testSuite "testSuite" method. Inserting
 *  new objects will insert them into the current test suite, and queries will return objects that belong to the current
 *  test suite. */
class TestSuite: public Sawyer::SharedObject, public Sawyer::SharedFromThis<TestSuite> {
public:
    /** Reference counting pointer to @ref TestSuite. */
    typedef Sawyer::SharedPointer<TestSuite> Ptr;

private:
    mutable SAWYER_THREAD_TRAITS::Mutex mutex_;         // protects the following data members
    std::string name_;                                  // unique and non-empty within a database
    std::string timestamp_;                             // time of creation

protected:
    TestSuite();

public:
    ~TestSuite();

    /** Allocating constructor. */
    static Ptr instance(const std::string &name = "");

    /** Property: Name.
     *
     *  Within a database, a test suite must have a unique non-empty name. However this is not a requirement when the test
     *  suite exists only in memory. The constraints are enforced when the test suite is added to the database.
     *
     *  Thread safety: This method is thread safe.
     *
     * @{ */
    std::string name() const;                           // value return is intentional for thread safety
    void name(const std::string&);
    /** @} */

    /** Returns printable name of test suite for diagnostic output.
     *
     *  Returns a string suitable for printing to a terminal, containing the words "test suite", the database ID if
     *  appropriate, and the test suite name using C-style double-quoted string literal syntax if not empty.  The database ID
     *  is shown if a non-null database is specified and this test suite exists in that database. */
    std::string printableName(const DatabasePtr &db = DatabasePtr());

    /** Property: Database creation timestamp string.
     *
     *  Time stamp string describing when this object was created in the database, initialized the first time the object is
     *  written to the database. If a value is assigned prior to writing to the database, then the assigned value is used
     *  instead. The value is typically specified in ISO-8601 format (except a space is used to separate the date and time for
     *  better readability, as in RFC 3339). This allows dates to be sorted chronologically as strings.
     *
     *  Thread safety: This method is thread safe.
     *
     * @{ */
    std::string timestamp() const;
    void timestamp(const std::string&);
    /** @} */
};

} // namespace
} // namespace
} // namespace

#endif
#endif
