/*! \file
    \brief Definitions for Operation class and its descendants
*/

#include "operations.hpp"
#include "database.hpp"
#include "general.hpp"
#include "random.hpp"
#include <assert.h>

void Scan::operator()(Database & d)
{
  System & s = d.system;
  Relation & r = d.relations[relationNum];

  // amount of buffer to use in each read (unit is 1 record)
  int buffer((int)rint(s.blockSize * s.numBlocks * useBuffer / r.recordSize));

  int firstRec(1), lastRec(buffer);
  bool done = false;

  for(;;)
  {
    if (lastRec >= r.numRecords)
    {
      done = true;
      lastRec = r.numRecords;
    }
    d.read(r.fileNum, firstRec, lastRec, -1);
    if (done) break;
    firstRec = lastRec + 1;
    lastRec += buffer;
  }
};

void Lookup::operator()(Database & d)
{
  // assumes exactly one match found ( https://www1.columbia.edu/sec/bboard/021/coms4111-001/msg00303.html )

  System & s = d.system;
  Index & i = d.indices[indexNum];
  Relation & r = d.relations[i.relationNum];

  // size of relation in blocks
  int packedRelationSize = divCeil(r.numRecords,
    s.blockSize / r.recordSize);

  int relationSize = (int)rint(packedRelationSize / r.occupancy);

  if (i.physicalType == Index::HASH)
  {
    int pointersPerBucket = s.blockSize / i.pointerSize;
    // hash table size in blocks
    int hashSize = (int)rint((double)r.numRecords / (double)pointersPerBucket
      / i.occupancy);

    int whichBlock = RandomInt(hashSize);
  }
  else if (i.physicalType == Index::BPLUS)
  {}
  else
    assert(false);
};

void BlockJoin::operator()(Database & d)
{
};

void IndexLoop::operator()(Database & d)
{
};