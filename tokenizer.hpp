#ifndef tokenizer_hpp
#define tokenizer_hpp

/*! \file
    \brief Tokenizer class for parsing .dsc files
*/


#include <iostream>
#include <string>

using std::istream;
using std::string;

//! Tokenizer class for parsing .dsc files
class Tokenizer
{
public:
  
  //! input stream
  istream & in;
  
  //! current line number
  int lineNo;

  //! current token
  string token;

  //! Construct with an input stream
  Tokenizer(istream & in_) : in(in_), lineNo(1) { }

  //! retrieve current token as an integer
  int getInt()
  {
    return atoi(token.c_str());
  }

  //! retrieve current token as a double
  double getDouble()
  {
    return atof(token.c_str());
  }

  //! skip over whitespace and comments
  bool eatWhite()
  {
    bool inComment = false;
    for(;;)
    {
      int c = in.peek();
      bool isEof = c == EOF;
      bool isSpace = c <= 32;
      bool isLine = c == '\n';
      bool isComment = c == '#';

      if (isLine) ++lineNo;

      if (isEof)
        return false;
      else if (inComment)
      {
        if (isLine) inComment = false;
        in.get();
      }
      else if (isComment)
      {
        inComment = true;
        in.get();
      }
      else if (isSpace)
        in.get();
      else
        return true;
    }
  }

  //! get next token value, or return false if at the end of the file
  bool next()
  {
    token = "";

    if (!eatWhite()) return false;
    for(;;)
    {
      int c = in.peek();
      bool isSpace = c <= 32;

      if (isSpace) return true;
      
      token += (char)c;
      in.get();
    }
  }
};

#endif
