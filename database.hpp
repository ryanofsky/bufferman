#ifndef database_hpp
#define database_hpp

/*! \file
    \brief Classes that hold parameters describing tables, indices, and databases and load this information from files.
*/

#include "general.hpp"
#include "operations.hpp"

#include<vector>
using std::vector;

//! Loads and stores system parameters
class System
{
public:
  
  //! block size in bytes
  int blockSize;
  
  //! block transfer time in milliseconds
  double transferTime; 
  
  //! average rotational latency in millisecond
  double rotationalLatency; 

  //! average seek time in milliseconds
  double seekTime; 
  
  //! number of blocks in the buffer
  int numBlocks;

  //! load system parameters from the system description file
  System(char const * filename);
};

//! Represents a single relation
struct Relation
{
  //! file number
  int fileNum;
  
  //! record size in bytes
  int recordSize; 
  
  //! number of records
  int numRecords; 
  
  //! average fraction of each block containing data records (excluding fragmentation)
  double occupancy; 

  //! calculates the value of relationSize
  void calc(Database & d);

  //! amount of blocks used by the relation (calculated)
  int relationSize;

  //! Return the number of records that can fit in a certain fraction of the buffer
  int recordsInBuf(System & s, double bufferFrac, int reserve = 0);

  //! fileNum is the same as f. Exists for use with STL algorithms like std::find
  bool operator==(int f) const
  {
    return fileNum == f;
  }
};

//! Represents an index
struct Index
{
  //! file number
  int fileNum; 
  
  //! conceptual index type
  enum { PRIMARY, SECONDARY } conceptualType; 
  
  //! attribute/key size in bytes
  int keySize; 
  
  //! pointer size in bytes
  int pointerSize; 
  
  //! B+ tree or hash;
  enum { BPLUS, HASH } physicalType; 
  
  //! average slot occupancy in the B+ tree or hash table
  double occupancy; 
  
  //! index into Database::relations vector
  int relationNum;

  //! calculates the values of numKeys, keysPerNode, keysPerLeafNode, and indexSize
  void calc(Database & database);

  //! How many blocks the index takes up on disk
  int indexSize;

  /*! \brief number of keys in the index
      For a primary b+tree it is number of blocks in the relation,
      otherwise it is the number of records in the relation
  */
  int numKeys;
  
  //! number of keys that can fit on an internal node in a B+ tree or number of keys per bucket in a hash index
  int keysPerNode;

  /*! \brief number of keys that fit on a leaf node in a B+ tree
      Even though we can assume pointer size == RID size, this 
      number is less than keysPerNode because of the doubly linked
      list between leaf nodes in a B+ tree
      see https://www1.columbia.edu/sec/bboard/021/coms4111-001/msg00327.html
  */
  int keysPerLeafNode;

  //! Number of nodes in each tree level
  vector<int> treeLevels;

  //! fileNum is the same as f. Exists for use with STL algorithms like std::find
  bool operator==(int f) const
  {
    return fileNum == f;
  }
};

//! Represents a database (group of indices and relations)
class Database
{
public:
  //! vector of all relations in the database
  vector<Relation> relations;

  //! vector of all indices in the database
  vector<Index> indices;

  //! reference to a system object
  System & system;

  //! load database parameters from a description file
  Database(char const * filename, System & system);

  //! simulate a read from a database file
  void read(int fileNum, int startBlock, int endBlock, int priority = -1);
};

//! Represents an access pattern. Holds a list of possible operations and their probabilities
class Access
{
public:
  //! number of operations
  int numOperations; 
  
  //! vector of operation objects
  vector<Operation *> operations; 
  
  //! cummulative probabilities of operations
  vector<double> probs; 

  //! tally of operations done so far (for informational purposes only)
  vector<int> opCount; 

  //! reference to the database object
  Database & database;

  //! load operation parameters from a description file
  Access(char const * filename, Database & database);

  //! perform simulation
  void simulateOperations(System & system);

  ~Access();
};

// forward declaration
class Tokenizer;

//! Specialized exception class for holding information about parse errors
class ParseException : public Exception
{
public:
  string inputFile;
  int inputLine;
  string inputToken;
  string description;

  ParseException(string inputFile_, Tokenizer & t, string description_,
    char const * file_, int line_);

  virtual char const * className() const { return "ParseException"; };
};

#endif
