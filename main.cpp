/*! \file
    \brief holds main() function
*/

#define PATH ""

#include "database.hpp"
#include "general.hpp"
#include "random.hpp"

#include <iostream>
using std::cerr;
using std::endl;

int main(int argc, char * argv[])
{
  try
  {
    RandomSeed(4);
    System system(PATH "system.dsc");
    Database database(PATH "database.dsc", system);
    Access access(PATH "access.dsc", database);
    access.simulateOperations(system);
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
  
   \section intro Introduction
  
   This is the introduction.
  
   \section install Installation
  
   \subsection step1 Step 1: Opening the box
  
 */

