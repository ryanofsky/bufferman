/*! \file
    \brief holds main() function
*/

#define PATH ""
#define DEBUGMODE 0

#include "database.hpp"
#include "general.hpp"
#include "bufferman.hpp"
#include "tokenizer.hpp"

#include <iostream>
#include <iomanip>
#include <fstream>

using namespace std;

int main(int argc, char * argv[])
{
  if (argc > 1)
  {
    cerr << "Usage: " << argv[0] << endl;
    return 0;
  }

  char * filename = "stdin";

  //ifstream in("E:/Documents/2002-02/2/Database/bufferman/out.txt");
  istream & in = cin;

  try
  {
    System system(PATH "system.dsc");
    Database database(PATH "database.dsc", system);
    BufferManager bm(system, database);
    Tokenizer t(in);
    while(t.next())
    {
      if (t.token != "read")
        throw ParseException(filename, t, "Expecting 'read'", __FILE__, __LINE__);
      
      int fileNum, startRecord, endRecord, priority, startSector, endSector;

      bool eof = !t.next();
      fileNum = t.getInt();
      if (eof) throw ParseException(filename, t, "Expecting file number.", __FILE__, __LINE__);

      eof = !t.next();
      startRecord = t.getInt();
      if (eof || startRecord <= 0) 
        throw ParseException(filename, t, "Expecting positive starting record number", __FILE__, __LINE__);

      eof = !t.next();
      endRecord = t.getInt();
      if (eof || endRecord < startRecord)
        throw ParseException(filename, t, "Expecting ending record after the starting record", __FILE__, __LINE__);

      eof = !t.next();
      priority = t.getInt();
      if (eof || priority < -1 || priority > 99)
        throw ParseException(filename, t, "Expecting priority in [-1,99]", __FILE__, __LINE__);

      try
      {
        int reads = bm.read(fileNum, startRecord, endRecord, priority == -1 ? 10 : priority, startSector, endSector);
#if debugmode
        cerr << "Reading " << setw(2) << fileNum << " " << setw(5) 
          << startRecord << " " << setw(5) << endRecord << " blocks ["
          << startSector << "," << endSector << "] " << reads << " fetches" << endl;
#endif
        if (bm.readNumber % 1000 == 0) cout << bm.time << endl;
      }
      catch(...)
      {
        cerr << "Error simulating read at " << filename << ":" << t.lineNo << endl;
        throw;
      }
    };
    cout << bm.time << endl;
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
