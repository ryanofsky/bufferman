#ifndef general_hpp
#define general_hpp

/*! \file
    \brief Miscellaneous functions and classes
*/

#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <math.h>

using std::unary_function;
using std::cerr;
using std::ostream;
using std::string;
using std::endl;

//! returns ceiling of a/b
int inline divCeil(int a, int b)
{
  return a / b + (a % b == 0 ? 0 : 1);
};

//! returns true if |a - b| < 1e-6
bool inline equals(double a, double b)
{
  double c = a - b;
  return -1e-6 < c && c < 1e-6;
};

#if (defined(_MSC_VER) || (defined(WIN32) && defined(__GNUC__))) && !defined(rint)
//! microsoft's c library doesn't have any rounding functions
#define rint(x) floor((x) + 0.5)
#endif


//! Functor that deletes a pointer
template<class T> struct DeletePointer : public unary_function<T, void>
{
  void operator()(T x) { if (!x) delete x; }
};

//! General purpose exception class
class Exception
{
public:
  const char * file;
  int line;
  string message;

  Exception(const char * file_, int line_);
  Exception(string message_, const char * file_, int line_);

  //! Print a description of the exception to a character stream
  virtual void write(ostream & o) const;

  //! Return class name
  virtual char const * className() const;
};

//! More natural way to print an exception
ostream & operator<<(ostream & o, Exception const & e);

#define THROW_STREAM(x)                             \
  do {                                              \
    std::ostringstream str;                         \
    str << x;                                       \
    throw Exception(str.str(), __FILE__, __LINE__); \
  } while (false)

#define WARN_STREAM(x)                              \
   std::cerr << "Warning from " << __FILE__ << ":"  \
     << __LINE__ << ".\n" << x;

#endif
