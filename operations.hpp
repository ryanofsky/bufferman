#ifndef operations_hpp
#define operations_hpp

/*! \file
    \brief Declarations for Operation class and its descendants
*/

class Database;

//! Abstract class to represent an operation.
struct Operation
{
  //! Execute the operation on the given database
  virtual void operator()(Database & d) = 0;

  virtual ~Operation() {}
};

//! Performs a table scan
struct Scan : public Operation
{
  //! which relation to scan (index into Database::relations)
  int relationNum; 
  
  //! fraction of buffer to use
  double useBuffer; 

  void operator()(Database & d);
};

//! Performs a single table lookup
struct Lookup : public Operation
{
  //! which index to search (index into Database::indices)
  int indexNum; 

  void operator()(Database & d) { (*this)(d,1); }
  void operator()(Database & d, int numMatches);
};


//! Performs a nested block join on two tables
struct BlockJoin : public Operation
{
  //! inner relation number (index into Database::relations)
  int innerRelation;
  
  //! outer relation number (index into Database::relations)
  int outerRelation;

  //! fraction of buffer to use for inner loop
  double innerBuffer; 

  //! fraction of buffer to use for outer loop
  double outerBuffer; 

  void operator()(Database & d);
};


//! Performs nested-loop join of two tables with an index on the inner table
struct IndexLoop : public Lookup
{
  //! outer relation number (index into Database::relations)
  int relationNum;
  
  //! average number of matches found in the inner relation for each record in outer relation
  double matches; 

  void operator()(Database & d);
};

#endif
