#pragma warning( push )
#pragma warning( disable : 4786 )

/*! \file
    \brief holds main() function
*/

#define PATH ""

#include "database.hpp"
#include "general.hpp"
#include "bufferman.hpp"
#include "tokenizer.hpp"

#include <iostream>
using std::cin;
using std::cout;
using std::endl;

int main(int argc, char * argv[])
{
  if (argc > 1)
  {
    cerr << "Usage: " << argv[0] << endl;
    return 0;
  }

  char * filename = "stdin";

  try
  {
    System system(PATH "system.dsc");
    Database database(PATH "database.dsc", system);
    BufferManager bm(system, database);
    Tokenizer t(cin);
    while(t.next())
    {
      if (t.token != "read")
        throw ParseException(filename, t, "Expecting 'read'", __FILE__, __LINE__);
      
      int fileNum, startRecord, endRecord, priority;

      bool eof = t.next();
      fileNum = t.getInt();
      if (eof) throw ParseException(filename, t, "Expecting file number.", __FILE__, __LINE__);

      eof = t.next();
      startRecord = t.getInt();
      if (eof || startRecord <= 0) 
        throw ParseException(filename, t, "Expecting positive starting record number", __FILE__, __LINE__);

      eof = t.next();
      endRecord = t.getInt();
      if (eof || endRecord < startRecord)
        throw ParseException(filename, t, "Expecting ending record after the starting record", __FILE__, __LINE__);

      eof = t.next();
      priority = t.getInt();
      if (eof || priority < -1 || priority > 99)
        throw ParseException(filename, t, "Expecting priority in [-1,99]", __FILE__, __LINE__);

      try
      {
        bm.read(fileNum, startRecord, endRecord, priority);
      }
      catch(...)
      {
        cerr << "Error simulating read at " << filename << ":" << t.lineNo << endl;
        throw;
      }
    };
    return 0;
  }
  catch(Exception e)
  {
    cerr << e << endl;
  }
  catch(...)
  {
    cerr << "Unknown exception caught" << endl;
  }
  return 1;
}


/*! 
  
\mainpage Project 2, Part 1: %Access Pattern Generator
  
Russell Yanofsky (rey4@columbia.edu)<br>
Felix Candelario (frc9@columbia.edu)

\section s0 Instructions

To compile this code on CUNIX you have to the newer version of G++. Do this by pasting
this code before running the makefile

<pre>
export PATH="/opt/gcc-2.95.3/bin:$PATH" 
export LD_LIBRARY_PATH="/opt/gcc-2.95.3/lib:$LD_LIBRARY_PATH"
</pre>

\section s1 Documentation

These pages are automatically generated from comments in our source. Using them you can get
descriptions that we provided for every class, class member, function and file
in our submission. They also have extra things like inheritance diagrams and
colorized, hyperlinked source listings.

\section s2 Code organization

Our program starts in the main() method. It instantiates a System class, a Database class, and
an Access class and these classes parse the input files system.dsc, database.dsc, and
access.dsc (respectively) and store information from these files in their members. To aid parsing
they all use a class called Tokenizer which makes it easy for them all to deal with the file
format being used in the project.

The Database object holds vectors of Relation objects and Index objects. These objects have members 
to hold all the parameters describing relations and indices that were given in the database.dsc file.
They also have store some additional information that is not provided as input, such the number of
blocks used to hold indices and relations. This extra information is calculated in the Relation::calc()
and Index::calc() functions. The calc() functions are called during parsing, and the information they
provide is used during the simulation.

The individual operations specified in access.dsc wind up in a vector in the Access class. Each
type of operation is represented in a class of its own. These classes are Scan, Lookup, BlockJoin, 
and IndexLoop. All four classes are descendants of an abstract base class called Operation. (For
our own convenience we made IndexLoop a descendant of Lookup).

After all of this information has been loaded the main() method calls Access::simulateOperations().
Simulate operation performs a loop from 1 to the number of operations specifies in access.dsc, and
on each iteration will choose a random operation to perform (based on the probabilities given in access.dsc)

After choosing an operation, it invokes it (Operations are functors).

\section s3 Miscellaneous

Our program uses the Boost random number library from http://www.boost.org/ . It is included in this submission
in the boost/ directory. The normal C rand() function is a pain to use because the random numbers it gives
are smaller than random numbers we need for this assignment.

Most of the relevant source in in operations.cpp (where the operations are simulated) and at the top of
database.cpp (where some other calculations are done). The rest of it is parsing code and other basic stuff.


\subsection Files

  
*/

// \subsection step1 Step 1: Opening the box

#pragma warning( pop )