/*! \file
    \brief Definitions for Operation class and its descendants
*/

#include "operations.hpp"
#include "database.hpp"
#include "general.hpp"
#include "random.hpp"

#include <assert.h>
#include <vector>
#include <algorithm>

using std::vector;
using std::min;

void Scan::operator()(Database & d)
{
  System & s = d.system;
  Relation & r = d.relations[relationNum];

  // amount of buffer to use in each read (unit is 1 record)
  int buffer = r.recordsInBuf(s,useBuffer);

  for(int rec = 1; rec <= r.numRecords; rec += buffer)
    d.read(r.fileNum, rec, min(rec+buffer-1, r.numRecords), -1);
};

void Lookup::operator()(Database & d, int numMatches)
{
  // by default assumes one match found ( https://www1.columbia.edu/sec/bboard/021/coms4111-001/msg00303.html )

  System & s = d.system;
  Index & i = d.indices[indexNum];
  Relation & r = d.relations[i.relationNum];

  if (i.conceptualType == Index::PRIMARY)
  {
    // if index is on a primary key, there will always
    // be exactly one match no matter what the argument
    // says. if the argument was not 1 then a warning
    // will have been printed during input validation in
    // Access::Access()
    numMatches = 1;
  }

  // first choose a random record from the relation
  int randomRecord = RandomInt(r.numRecords);

  if (i.physicalType == Index::HASH)
  {
    // read a random block in the hash table
    int whichBlock = RandomInt(i.blockSize);
    d.read(i.fileNum, whichBlock+1, whichBlock+1);
  }
  else if (i.physicalType == Index::BPLUS)
  {
    // SUMMARY:
    // find a leaf node corresponding to randomBlock.
    // then iterate through the levels of the tree
    // adding the block numbers of its parents to
    // a vector of reads. finally, perform the reads
    // in the opposite order in which they were added
    // to the vector
    
    // vector of reads to be performed in reverse order
    vector<int> reads;

    // holds the current depth in the tree (in reverse order so 0
    // is the leaf level and i.treeLevels.size() - 1 is the root
    // level
    int currentDepth = 0;
    
    // holds the tree node offset relative to the first node
    // of the current depth of the tree
    int currentNode;
    
    // current depth is the at the leaf level, so pick a leaf node
    if (i.conceptualType == Index::PRIMARY)
    {
      // i is a primary, sparse index and r blocks are sorted based on primary key
      currentNode = mdiv(randomRecord, i.treeLevels[currentDepth], r.numRecords);
    }
    else
    {
      // order of blocks in r is not related to order of secondary index i
      currentNode = RandomInt(i.treeLevels[currentDepth]);
    }
    
    // position of first node on the current depth
    int startingNode = i.blockSize - i.treeLevels[currentDepth];

    reads.push_back(startingNode + currentNode);

    for(currentDepth = 1; currentDepth < i.treeLevels.size(); ++currentDepth)
    {
      startingNode -= i.treeLevels[currentDepth];
      currentNode = mdiv(currentNode,  i.treeLevels[currentDepth], 
        i.treeLevels[currentDepth - 1]);
      
      assert(0 <= currentNode && currentNode < i.treeLevels[currentDepth]);

      reads.push_back(startingNode + currentNode);
    };

    assert(startingNode == 0);

    typedef vector<int> v;
    typedef v::reverse_iterator vri;
    vri ri = reads.rbegin();

    // read the root with high priority
    d.read(i.fileNum, *ri+1, *ri+1, 99);
   
    // read the blocks
    for(++ri; ri != reads.rend(); ++ri)
      d.read(i.fileNum, *ri+1, *ri+1);

    // if there are multiple matches, and not all of the
    // rid's could have fit on the leaf node. then we need
    // to read some adjacent leaves.
    int readUntil = reads[0] + divCeil(numMatches, i.keysPerLeafNode) + 1; 
    for(int l = reads[0]+ 2 ; l < readUntil; ++l)
      d.read(i.fileNum, l, l);
  }
  else
    assert(false);

  // read the results from the relation
  if (numMatches > 0)
  for(;;)
  {
    d.read(r.fileNum, randomRecord+1, randomRecord+1);
    --numMatches;
    if (numMatches <= 0) break;
    randomRecord = RandomInt(r.numRecords);
  }
};

void BlockJoin::operator()(Database & d)
{
  System & s = d.system;
  Relation & ro = d.relations[outerRelation];
  Relation & ri = d.relations[innerRelation];

  // amount of buffer to use in each read (unit is 1 record)
  int oBuffer = ro.recordsInBuf(s,outerBuffer);
  int iBuffer = ri.recordsInBuf(s,innerBuffer);

  for(int orec = 1; orec <= ro.numRecords; orec += oBuffer)
  {
    int olast = min(orec+oBuffer-1, ro.numRecords);
    
    // buffer outer records with high priority
    d.read(ro.fileNum, orec, olast, 90);

    for(int irec = 1; irec <= ri.numRecords; irec += iBuffer)
    {
      int ilast = min(irec+iBuffer-1, ri.numRecords);
      
      // buffer inner records with medium priority
      d.read(ri.fileNum, irec, ilast, 10);

      for (int o = orec; o <= olast; ++o)
      {
        // actually read outer record ...
        d.read(ro.fileNum, o, o, 90);

        // and look for matches in inner records
        for (int i = irec; i <= ilast; ++i)
        {
          d.read(ri.fileNum, i, i, 10);
        }
      }
    }
  }
};

void IndexLoop::operator()(Database & d)
{
  System & s = d.system;
  Relation & r = d.relations[relationNum];
  Index & i = d.indices[indexNum];

  // leave pages for index and retrieval from relation
  int buffer = r.recordsInBuf(s, 1.0, 3 + i.treeLevels.size() + (int)ceil(matches));

  for(int rec = 1; rec <= r.numRecords; rec += buffer)
  {
    int lastRec = min(rec+buffer-1, r.numRecords);
    d.read(r.fileNum, rec, lastRec, 99);
    for (int a = rec; a <= lastRec; ++a)
    {
      d.read(r.fileNum, a, a, 99);
      Lookup::operator()(d, RandomConsecutive(matches));
    }
  }
};