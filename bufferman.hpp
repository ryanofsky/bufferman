#ifndef bufferman_hpp
#define bufferman_hpp

#include <map>
#include <vector>
#include <cassert>

using std::vector;
using std::map;
using std::pair;

struct File
{
  //! starts on this sector
  int startingBlock;
  
  //! number of records in the file
  int numRecords;

  //! number of blocks in the file
  int numBlocks;

  File() : startingBlock(-1), numRecords(0), numBlocks(0) {}
};

class BufferManager
{
private:

  // define some types used internally
  struct Page;
  typedef vector<Page> Buffer;
  typedef Buffer::iterator Bi;
  typedef map<int, Bi> SectorMap;
  typedef SectorMap::iterator Si;
  typedef SectorMap::value_type Sp;
  struct Page
  {
    Si si;
    int priority;
    int lastRead;
  };

public:

  //! last sector read
  int lastPos;

  //! how many reads so far
  int readNumber;

  //! total time (in milliseconds)
  int time;

  //! vector of pages
  Buffer buffer;

  //! map of sector addresses that are currently loaded in the buffer
  SectorMap mapped;  

  System & system;
  Database & database;

  //! list of files indexed by file number
  map<int, File> files;

  BufferManager(System & system_, Database & database_)
    : system(system_), database(database_), lastPos(-1), readNumber(0), time(0)
  {
  
  }

  enum { EMPTY = -1, MIN_PRIORITY = 0, MAX_PRIORITY = 99, 
    DEFAULT_PRIORITY = 10 };

  //! map a file number and offset to a disk sector 
  int getSector(int fileNum, int offset)
  {
    File & f = files[fileNum];
    if (f.startingBlock < 0)
      THROW_STREAM("Invalid File number " << fileNum << endl);

    return files[fileNum].startingBlock + (offset - 1)
      * f.numBlocks / f.numRecords;
  }

  //! number of the disk sector read last, used to detect
  // consecutive reads
  int lastSector;

  //! read from a disk sector
  int readSector(int sector)
  {
    if (sector == lastSector)
    {
      // xxx: what is rotational latency (why isn't it just a part of seekTime)
      time += system.rotationalLatency + system.seekTime;
    } 
    time += system.transferTime;
    lastSector = sector; 
  }

  int lowestPriority;

  bool usePage(Bi page, int priority)
  {
    if (page->lastRead == readNumber)
    {
      return false;
    }
    else if (page->priority >= priority)
    {
      lowerPagePriority(page); 
      return false;
    }
    return true;
  }

  void lowerPagePriority(Bi page)
  {
    page->priority -= 10;
  }


  Bi getFreePage(int sector, int priority)
  {
    for(;;)
    {
      ++lastPos;
      lastPos = lastPos % buffer.size();

      Bi page = buffer.begin() + lastPos;
      if (usePage(page,priority))
      {
        page->lastRead = readNumber;
        return page;
      }
    }
  };


  void read(int fileNum, int startRecord, int endRecord, int priority)
  {
    ++readNumber;

    if (priority == EMPTY)
      priority = DEFAULT_PRIORITY;
    else
      assert(MIN_PRIORITY <= priority && priority <= MAX_PRIORITY);

    int start = getSector(fileNum, startRecord);
    int end = getSector(fileNum, endRecord + 1);

    for(int r = start; r < end; ++r)
    {
      Bi page; 
      pair<Si, bool> f = mapped.insert(Sp(r, page));
      
      if (!f.second) // no need to insert page is already in buffer
        page = f.first->second;
      else
      {
        f.first->second = page = getFreePage(r, priority);
        //xxx: 
        mapped.erase(page->si);
        page->si = f.first;
      }
      page->lastRead = readNumber;
      if (page->priority < priority) page->priority = priority;
    }
  }
};

#endif
