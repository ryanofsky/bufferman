#ifndef bufferman_hpp
#define bufferman_hpp

struct File
{
  //! starts on this sector
  int startingBlock;
  
  //! number of records in the file
  int numRecords;

  //! number of blocks in the file
  int numBlocks;
};

class BufferManager
{
public:
  System & system;
  Database & database;

  enum { EMPTY = -1, MIN_PRIORITY = 0, MAX_PRIORITY = 99, 
    DEFAULT_PRIORITY = 10 };
    
  BufferManager()
  {
    buffer.resize(s.numBlocks, EMPTYPAGE);
    filledSectors = 0;
    {
    for(int i = 0; i < database.relations.size(); ++i)
    {
      Relation & r = database.relations[i];
      File f(filledSectors, r.numRecords, r.relationSize);
      filledSectors += r.relationSize;
    }
    }

    {
    for(int i = 0; i < database.indices.size(); ++i)
    {
      Index & in = database.indices[i];
      File f(filledSectors, 1, 1);
      files[i.fileNum] = in;
    }
    }
    assert(buffer.size() > 0);
  }

  //! maps a file
  map<int, int> files;

  //vector of prorities
  vector<int> buffer;

  struct Page;

  typedef vector<Page> Buffer;
  typedef Buffer::iterator Bi;
  typedef hash_map<int, Bi> SectorMap;
  typedef SectorMap::iterator Si;

  struct Page
  {
    Si si;
    int priority;
    int lastRead;
  };

  SectorMap mapped;

  int getSector(int fileNum, int record)
  {
    File & f = files[fileNum];
    return files[fileNum].startingBlock + (record - 1)
      * f.numBlocks / f.numRecords;
  }

  //! map from a priority number to the count of blocks that have that priority
  map<int, int> priorities;
  int lastSkipped;
  int lowestPriority;

  bool usePage(Bi page, int priority)
  {
    if (page->lastRead == readNumber)
    {
      lastSkipped = lastPos;
      return false;
    }
    return yourmom;
  }

  void lowerPagePriority(Bi page)
  {
    page->priority -= 10;
  }

  int lastPos = -1;
  lastSkipped = lastPos;
  Bi getFreePage(int sector, int priority)
  {
    int lastSkipped = priorities.begin()->first;
    for(;;)
    {
      ++lastPos;
      lastPos = lastPos % buffer.size();

      if (lastPos == lastSkipped)
        THROW_STREAM("Not enough room in the buffer to store sector " 
          << sector);

      Bi page = buffer.begin() + lastPos;
      if (usePage(priority, page))
      {
        page->lastRead = readNumber;
        return page;
      }
    }
  };

  int readNumber = 0;
  void read(int fileNum, int startRecord, int endRecord, int priority)
  {
    ++readNumber;

    if (priority == EMPTY)
      priority = DEFAULT_PRIORITY;
    else
      assert(MIN_PRIORITY <= priority && priority <= MAX_PRIORITY);

    int start = getSector(startRecord);
    int end = getSector(endRecord + 1);

    for(int r = start; r < end; ++r)
    {
      Bi page; 
      pair<Si, bool> f = mapped.insert(r, page);
      
      if (!f.second) // no need to insert page is already in buffer
        page = f.first->second;
      else
      {
        f.first->second = page = getFreePage(r, priority);
        if (page->si) mapped.erase(page->si);
        page->si = f.first;
      }
      page->lastRead = readNumber;
      if (page->priority < priority) page->priority = priority;
    }
  }
};

#endif