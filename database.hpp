#ifndef database_hpp
#define database_hpp

/*! \file
    \brief Classes that hold parameters describing tables, indices, and databases and load this information from files.
*/

#include "operations.hpp"

#include<vector>
using std::vector;

//! Loads and stores system parameters
class System
{
public:
  
  //! block size in bytes
  int blockSize;
  
  //! block transfer type in milliseconds
  double transferTime; 
  
  //! average rotational latency in millisecond
  double rotationalLatency; 

  //! average seek time in milliseconds
  double seekTime; 
  
  //! number of blocks in the buffer
  double numBlocks;

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
  
  // Notes:
  // for B+ Index, one record = one block = one node
  // for hash index, one record = one block = one bucket
  // a primary B+ index is assumed to be sparse (https://www1.columbia.edu/sec/bboard/021/coms4111-001/msg00319.html)

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

  //! reference to the database object
  Database & database;

  //! load operation parameters from a description file
  Access(char const * filename, Database & database);

  //! perform simulation
  void simulateOperations(System & system);

  ~Access();
};

#endif
