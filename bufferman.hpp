#ifndef bufferman_hpp
#define bufferman_hpp

#include <map>
#include <vector>
#include <cassert>

using namespace std;

//! Holds information about the position of a file on disk
struct File
{
  //! starts on this sector
  int startingBlock;
  
  //! number of records in the file
  int numRecords;

  //! number of blocks in the file
  int numBlocks;

  //! constructor
  File(int startingBlock_, int numRecords_, int numBlocks_) : startingBlock(
    startingBlock_), numRecords(numRecords_), numBlocks(numBlocks_) {}

  //! default constructor
  File()  : startingBlock(-1) {}
};

//! Holds information about one page (block) in the buffer
struct BufferPage
{
  //! the sector number stored in this buffer page
  int sector;

  //! last time this page was read
  int lastRead;

  //! priority level of this page
  int priority;
  
  //! default constructor
  BufferPage() : sector(-1), priority(0) {}
};

typedef vector<BufferPage> Buffer;
typedef Buffer::iterator BufferIterator;
typedef map<int, BufferIterator> SectorMap;
typedef SectorMap::iterator SectorIterator;
typedef SectorMap::value_type SectorPair;

//! Implements buffer management algorithm and measures disk accesses
class BufferManager
{
public:
  //! how many reads so far (number of times BufferManager::read() called)
  int readNumber;

  //! number of blocks read from disk so far 
  int fetchNumber;

  //! sector number which was read from the disk most recently, used to detect consecutive reads
  int lastSector;

  //! total time in milliseconds of disk accesses so far
  double time;  

  //! next page of buffer to try to read into
  int nextPage;

  //! vector of pages
  Buffer buffer;

  //! map of sector addresses that are currently loaded in the buffer
  SectorMap mapped;  

  //! System object
  System & system;

  //! Databse object
  Database & database;

  //! list of files in the database indexed by file number
  map<int, File> files;

  //! constructor
  BufferManager(System & system_, Database & database_)
  : system(system_), database(database_), readNumber(0), fetchNumber(0),
    lastSector(-1), time(0), nextPage(0)
  {
    buffer.resize(system.numBlocks);
    int lastBlock(0);
    
    for(int a = 0; a < database.relations.size(); ++a)
    {
      Relation & r = database.relations[a];
      files[r.fileNum] = File(lastBlock, r.numRecords, r.blockSize);
      lastBlock +=r.blockSize;
    }

    for(int b = 0; b < database.indices.size(); ++b)
    {
      Index & i = database.indices[b];
      files[i.fileNum] = File(lastBlock, i.blockSize, i.blockSize);
      lastBlock += i.blockSize;
    }
  }

  //! read from a disk sector
  void fetchSector(int sector)
  {
    ++fetchNumber;
    if (sector == lastSector + 1)
      time += system.rotationalLatency + system.seekTime;
    time += system.transferTime;
    lastSector = sector; 
  }

  //! find the next low priority page to replace
  BufferIterator getFreePage(int sector, int priority)
  {
    for(;;)
    {
      BufferIterator page = buffer.begin() + nextPage;
      nextPage = (nextPage + 1) % buffer.size();
      if (page->lastRead != readNumber) page->priority -= 10;
      if (page->priority < priority)
      {
        page->lastRead = readNumber;
        return page;
      }
    }
  };

  /*!
    \brief Read a range of records from a file into the buffer
    Returns the number of disk accesses needed to perform the read
    \param fileNum      file number to read from
    \param startRecord  first record to read
    \param endRecord    last record to read
    \param priority     priority value of this read operation
    \param startSector  output parameter that holds the sector number corresponding to startRecord
    \param endSector    output parameter that holds the sector number corresponding to endRecord
  */
  int read(int fileNum, int startRecord, int endRecord, int priority, int & startSector, int & endSector)
  {
    ++readNumber;
    int oldFetchNumber = fetchNumber;

    File & f = files[fileNum];
    if (f.startingBlock < 0) THROW_STREAM("Invalid File number " << fileNum << endl);
    startSector = f.startingBlock + (startRecord-1) * f.numBlocks / f.numRecords;
    endSector = f.startingBlock + (endRecord-1) * f.numBlocks / f.numRecords;

    if (endSector - startSector > buffer.size()) THROW_STREAM("Buffer is not big "
      "enough to hold records " << startRecord << "-" << endRecord << " from "
      "file " << fileNum);

    for(int r = startSector; r <= endSector; ++r)
    {
      BufferIterator page; 
      pair<SectorIterator, bool> f = mapped.insert(SectorPair(r, page));
      
      if (!f.second) // no need to insert page, it's already in buffer
        page = f.first->second;
      else
      {
        fetchSector(r); // simulate disk read
        page = f.first->second = getFreePage(r, priority); // find a spot in memory
        if (page->sector >= 0) mapped.erase(page->sector); // erase old mapping
        page->sector = r;
      }
      page->lastRead = readNumber;
      if (page->priority < priority) page->priority = priority;
    }
    return fetchNumber - oldFetchNumber;
  }
};

#endif