/*! \file
    \brief Code to load .dsc files and store the information in objects
*/

#include "general.hpp"
#include "database.hpp"
#include "tokenizer.hpp"
#include "random.hpp"

#include <fstream>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <string>
#include <assert.h>
#include <iomanip>

using std::ifstream;
using std::find;
using std::string;
using std::ostringstream;
using std::cout;
using std::setw;
using std::for_each;
using std::fill;

void Relation::calc(Database & d)
{
  System & s = d.system;

  // amount of space that the relation would use if its occupancy were 1.0
  int packedRelationSize = divCeil(numRecords,
    s.blockSize / recordSize);

  blockSize = (int)rint(packedRelationSize / occupancy);
}

int Relation::recordsInBuf(System & s, double bufferFrac, int reserve)
{
  // buffer size to use (unit = 1 block)  
  int bufferSize = (int) ((double)(s.numBlocks - reserve) * bufferFrac);
  int buffer = bufferSize * numRecords / blockSize;

  if (buffer <= 0)
    THROW_STREAM("Fraction of buffer to use on scan of relation " << fileNum
      << " is " << bufferFrac << ", which is not even big enough to hold a "
      " single record.");
  return buffer;
}

void Index::calc(Database & d)
{
  System & s = d.system;
  Relation & r = d.relations[relationNum];

  if (conceptualType == Index::PRIMARY && physicalType == Index::BPLUS)
  {
    numKeys = d.relations[relationNum].blockSize;
  }
  else
  {
    numKeys = r.numRecords;
  }

  if (physicalType == Index::HASH)
  {
    keysPerNode = s.blockSize / (keySize + pointerSize);
    if (keysPerNode < 1)
      THROW_STREAM("Key size or pointer size is too big in index " << fileNum 
        << ". If a node can't hold at least one pointer and one key it is "
        "impossible to have a hash table.");

    // parameter not used for hash tables
    keysPerLeafNode = -1;

    // amount of space that the index would use if its occupancy were 1.0
    int packedIndexSize = divCeil(numKeys, keysPerNode);

    blockSize = (int)rint((double)packedIndexSize / occupancy);
  }
  else if (physicalType == Index::BPLUS)
  {
    // assuming sizeof(pointer to node) == sizeof(pointer to tuple)
    //
    // subtract pointerSize because in internal nodes there is an extra pointer
    keysPerNode = (s.blockSize - pointerSize) / (keySize + pointerSize);
    if ((double)(1 + keysPerNode) * occupancy < 2.0)
      THROW_STREAM("Key size or pointer size is too big or occupancy is too small in index " << fileNum 
        << ". If at least two pointers can't fit in a node, it is impossible to have "
        "a tree.");

    // subtract 2 * pointersize to make room for doubly linked list pointers
    keysPerLeafNode = (s.blockSize - 2 * pointerSize) / (keySize + pointerSize);
    if (keysPerNode < 1)
      THROW_STREAM("Key size or pointer size is too big in index " << fileNum 
        << ". If three pointers and a key can't fit in a node, it is "
        "impossible to have a B+ tree leaf.");

    int numChildren = (int)rint((double)divCeil(numKeys, keysPerLeafNode) / occupancy);
      
    treeLevels.push_back(numChildren);
    blockSize = numChildren;

    do
    {
      numChildren = (int)ceil((double)numChildren / (double) (keysPerNode+1) / occupancy);
      treeLevels.push_back(numChildren);
      blockSize += numChildren;   
    }
    while (numChildren > 1);

    // using do-while instead of while because I'm assuming there must be at 
    // least one non-leaf node even if all keys fit in one leaf
    assert(treeLevels.size() > 1);
  }
  else
    assert(false);
}


ParseException::ParseException(string inputFile_, Tokenizer & t, string description_,
    char const * file_, int line_)
: inputFile(inputFile_), inputLine(t.lineNo), inputToken(t.token),
  description(description_), Exception(file_, line_)
{
  ostringstream s;
  s << "Error parsing " << inputFile_ << " near ";

  if (t.token.length() == 0 && t.in.peek() == EOF)
    s << "EOF";
  else
    s << "'" << t.token << "'";
  s << " at line " << inputLine << endl << description << endl;
  message = s.str();
};


System::System(char const * filename)
{
  ifstream f(filename);
  if (!f.is_open()) THROW_STREAM("Failed to open " << filename << endl);

  Tokenizer t(f);
  bool eof;

  eof = !t.next();
  blockSize = t.getInt();
  if (blockSize <= 0 || eof)
    throw ParseException(filename, t, "Expecting block size > 0.", __FILE__, __LINE__);

  eof = !t.next();
  transferTime = t.getDouble();
  if (transferTime < 0 || eof)
    throw ParseException(filename, t, "Expecting block transfer time >= 0.", __FILE__, __LINE__);

  eof = !t.next();
  rotationalLatency = t.getDouble();
  if (rotationalLatency < 0 || eof)
    throw ParseException(filename, t, "Expecting rotational latency time >= 0.", __FILE__, __LINE__);

  eof = !t.next();
  seekTime = t.getDouble();
  if (seekTime < 0 || eof)
    throw ParseException(filename, t, "Expecting seek time >= 0.", __FILE__, __LINE__);

  eof = !t.next();
  numBlocks = t.getInt();
  if (numBlocks < 0 || eof)
    throw ParseException(filename, t, "Expecting buffer size >= 0.", __FILE__, __LINE__);

  if (t.next())
    throw ParseException(filename, t, "Expecting end of file.", __FILE__, __LINE__);
}

Database::Database(char const * filename, System & system_)
: system(system_)
{
  ifstream f(filename);
  if (!f.is_open()) THROW_STREAM("Failed to open " << filename << endl);
  bool eof = false;

  Tokenizer t(f);

  while(t.next())
  {
    int fileNum = t.getInt();

    if (fileNum <=0 || eof)
      throw ParseException(filename, t, "Expecting file number greater than 0.", __FILE__, __LINE__);

    if (find(relations.begin(), relations.end(), fileNum) != relations.end()
      || find(indices.begin(), indices.end(), fileNum) != indices.end())
      throw ParseException(filename, t, "Duplicate file number", __FILE__, __LINE__);

    bool eof = !t.next();
    if (t.token == "relation")
    {
      relations.resize(relations.size()+1);
      Relation & r = relations.back();
      
      r.fileNum = fileNum;

      eof = !t.next();
      r.recordSize = t.getInt();
      if (r.recordSize <= 0 || eof)
        throw ParseException(filename, t, "Expecting relation record size > 0", __FILE__, __LINE__);

      if (r.recordSize > system.blockSize)
        throw ParseException(filename, t, "Expecting relation record size < block size", __FILE__, __LINE__);

      eof = !t.next();
      r.numRecords = t.getInt();
      if (r.numRecords <= 0 || eof)
        throw ParseException(filename, t, "Expecting at least one record", __FILE__, __LINE__);

      eof = !t.next();
      r.occupancy = t.getDouble();
      if (r.occupancy <= 0 || r.occupancy > 1)
        throw ParseException(filename, t, "Expecting occupied fraction to be in (0,1]", __FILE__, __LINE__);

      r.calc(*this);
    }
    else if (t.token == "index")
    {
      indices.resize(indices.size()+1);
      Index & i = indices.back();
      i.fileNum = fileNum;
      i.relationNum = relations.size() - 1;

      if (i.relationNum < 0)
        throw ParseException(filename, t, "At least on relation must appear before an index", __FILE__, __LINE__);

      t.next();
      if (t.token == "primary")
        i.conceptualType = Index::PRIMARY;
      else if (t.token == "secondary")
        i.conceptualType = Index::SECONDARY;
      else
        throw ParseException(filename, t, "Expecting either 'primary' or 'secondary'", __FILE__, __LINE__);

      eof = !t.next();
      i.keySize = t.getInt();
      if (i.keySize <= 0 || eof)
        throw ParseException(filename, t, "Expecting key size > 0", __FILE__, __LINE__);

      eof = !t.next();
      i.pointerSize = t.getInt();
      if (i.pointerSize <= 0 || eof)
        throw ParseException(filename, t, "Expecting pointer size > 0", __FILE__, __LINE__);

      t.next();
      if (t.token == "b+tree")
        i.physicalType = Index::BPLUS;
      else if (t.token == "hash")
        i.physicalType = Index::HASH;
      else
        throw ParseException(filename, t, "Expecting either 'b+tree' or 'hash'", __FILE__, __LINE__);

      eof = !t.next();
      i.occupancy = t.getDouble();
      if (i.occupancy <= 0 || i.occupancy > 1)
        throw ParseException(filename, t, "Expecting occupied fraction to be in (0,1]", __FILE__, __LINE__);

      i.calc(*this);
    }
    else
      throw ParseException(filename, t, "Expecting either 'relation' or 'index'", __FILE__, __LINE__);

    eof = !t.next();
    if (eof || t.getInt() != -1)
      throw ParseException(filename, t, "Expecting sentinel -1 value", __FILE__, __LINE__);
  }
};

/*!
  \param priority 0 is low, 99 is high
*/
void Database::read(int fileNum, int startBlock, int endBlock, int priority)
{
  assert(-1 <= priority && priority <= 99);
  cout << "read " << setw(2) << fileNum
    << " " << setw(6) << startBlock
    << " " << setw(6) << endBlock
    << " " << setw(3) << priority
    << "\n";
};

Access::Access(char const * filename, Database & database_)
: database(database_)
{
  ifstream f(filename);
  if (!f.is_open()) THROW_STREAM("Failed to open " << filename << endl);

  Tokenizer t(f);

  bool eof = !t.next();
  numOperations = t.getInt();
  if (numOperations <= 0 || eof)
    throw ParseException(filename, t, "Expecting number of operations to be > 0", __FILE__, __LINE__);

  vector<Relation> const & r = database.relations;
  vector<Index> const & i = database.indices;

  double sum = 0;
  while(t.next())
  {
    double prob = t.getDouble();
    if (prob <= 0 || prob > 1)
      throw ParseException(filename, t, "Expecting probability between 0 and 1", __FILE__, __LINE__);

    sum += prob;
    probs.push_back(sum);

    t.next();

    if (t.token == "scan")
    {
      Scan * scan;
      operations.push_back(scan = new Scan());

      eof = !t.next();
      scan->relationNum = find(r.begin(), r.end(), t.getInt()) - r.begin();
      if (scan->relationNum >= r.size() || eof)
        throw ParseException(filename, t, "Relation not found", __FILE__, __LINE__);

      eof = !t.next();
      scan->useBuffer = t.getDouble();
      if (scan->useBuffer <= 0 || scan->useBuffer > 1 || eof)
        throw ParseException(filename, t, "Expecting fraction of buffer to use between 0 and 1", __FILE__, __LINE__);
    }
    else if (t.token == "lookup")
    {
      Lookup * lookup;
      operations.push_back(lookup = new Lookup());

      eof = !t.next();
      lookup->indexNum = find(i.begin(), i.end(), t.getInt()) - i.begin();
      if (lookup->indexNum >= i.size() || eof)
        throw ParseException(filename, t, "Index not found", __FILE__, __LINE__);
    }
    else if (t.token == "block")
    {
      BlockJoin * blockJoin;
      operations.push_back(blockJoin = new BlockJoin());

      eof = !t.next();
      blockJoin->outerRelation = find(r.begin(), r.end(), t.getInt()) - r.begin();
      if (blockJoin->outerRelation >= r.size() || eof)
        throw ParseException(filename, t, "Relation not found", __FILE__, __LINE__);

      eof = !t.next();
      blockJoin->innerRelation = find(r.begin(), r.end(), t.getInt()) - r.begin();
      if (blockJoin->innerRelation >= r.size() || eof)
        throw ParseException(filename, t, "Relation not found", __FILE__, __LINE__);

      eof = !t.next();
      blockJoin->outerBuffer = t.getDouble();
      if (blockJoin->outerBuffer < 0 || blockJoin->outerBuffer > 1 || eof)
        throw ParseException(filename, t, "Expecting fraction of buffer to use between 0 and 1", __FILE__, __LINE__);

      eof = !t.next();
      blockJoin->innerBuffer = t.getDouble();
      double total = blockJoin->innerBuffer + blockJoin->outerBuffer;
      if (total < 0 || total > 1 || eof)
        throw ParseException(filename, t, "Expected inner buffer + outer buffer usage to be between 0 and 1", __FILE__, __LINE__);
    }
    else if (t.token == "indexloop")
    {
      IndexLoop * indexLoop;
      operations.push_back(indexLoop = new IndexLoop());

      eof = !t.next();
      indexLoop->relationNum = find(r.begin(), r.end(), t.getInt()) - r.begin();
      if (indexLoop->relationNum >= r.size() || eof)
        throw ParseException(filename, t, "Relation not found", __FILE__, __LINE__);

      eof = !t.next();
      indexLoop->indexNum = find(i.begin(), i.end(), t.getInt()) - i.begin();
      if (indexLoop->indexNum >= i.size() || eof)
        throw ParseException(filename, t, "Index not found", __FILE__, __LINE__);

      eof = !t.next();
      indexLoop->matches = t.getDouble();
      if (indexLoop->matches < 0 || eof)
        throw ParseException(filename, t, "Expecting average number of matches not less than zero", __FILE__, __LINE__);

      Index const & in = i[indexLoop->indexNum];

      if (in.conceptualType == Index::PRIMARY && !equals(indexLoop->matches, 1.0))
        WARN_STREAM("Index " << in.fileNum << " is a primary index, so you can "
          "not expect the average number of matches to be " << indexLoop->matches
          << ". This value will be ignored." << endl);
    }
    else
      throw ParseException(filename, t, "Expecting 'scan', 'lookup', 'block', or 'indexloop'", __FILE__, __LINE__);
  };

  if (!equals(sum,1.0))
    THROW_STREAM("Sum of probabilities for operations is " << sum << ", should be 1.0" << endl);

  assert(operations.size() == probs.size());
}

void Access::simulateOperations(System & system)
{
  opCount.resize(operations.size());
  fill(opCount.begin(), opCount.end(), 0);
  for (int i = 0; i < numOperations; ++i)
  {
    int r = RandomDiscrete(probs);
    assert(0 <= r && r < operations.size());
    //
    ++opCount[r];
    Operation & op = *operations[r];
    op(database);
  }
  //for(int j = 0; j < opCount.size(); ++j)
  //  cerr << "op " << j << " count = " << opCount[j] << endl;
}

Access::~Access()
{
  for_each(operations.begin(), operations.end(), DeletePointer<Operation *>());
}
