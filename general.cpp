/*! \file
    \brief Miscellaneous functions and classes
*/

#include "general.hpp"

ostream & operator<<(ostream & o, Exception const & e)
{
  e.write(o);
  return o;
};

Exception::Exception(const char * file_, int line_)
: file(file_), line(line_)
{}

Exception::Exception(string message_, const char * file_, int line_)
: message(message_), file(file_), line(line_)
{}

void Exception::write(ostream & o) const
{
  o << "Exception '" << className() << "' at " << file << ":" << line
    << endl << endl << message;
}

char const * Exception::className() const
{
  return "Exception";
}
