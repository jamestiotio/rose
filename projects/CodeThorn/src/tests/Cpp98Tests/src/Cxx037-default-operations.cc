#include "test-new.h"
#include "test-main.h"

const char* description = "Tests the generation of default operations with class hierarchies";
const char* expectedout = "{A1BA1CA#abA#acA=abA=acfacfab~A~A~A~A}";


// The default operators are implemented in class A, in the other
// classes the operators need to be generated by the analysis/compiler.

struct A
{
  A() : data("a") { printf("A0"); }

  explicit
  A(const char* s) : data(s) { printf("A1"); }

  virtual ~A() { printf("~A"); }

  A(const A& other)
  : data(other.data)
  {
    printf("A#%s", data);
  }

  A& operator=(const A& other)
  {
    data = other.data;
    printf("A=%s", data);

    return *this;
  }

  const char* data;
};

struct B : A
{
  B() : A("ab") { printf("B"); }
};

struct C : A
{
  C() : A("ac") { printf("C"); }
};

struct D : B, C {};

void f(const A& obj) { printf("f%s", obj.data); }

void run()
{
  D  d0;
  C& c = d0;
  D  d1(d0);
  B& b = d1;

  d0 = d1;
  f(c);
  f(b);
}

