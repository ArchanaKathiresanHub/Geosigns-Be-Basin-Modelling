#ifndef PARSE_H
#define PARSE_H

#ifdef sgi
#ifdef _STANDARD_C_PLUS_PLUS
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
using namespace std;
#else // !_STANDARD_C_PLUS_PLUS
#include <iostream.h>
#include <fstream.h>
#include <iomanip.h>
#include<strstream.h>
typedef strstream ostringstream;
#endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
using namespace std;
#endif // sgi

#include <string>
#include <vector>
#include <exception>


///provides functionality for parsing a Table
namespace fileIO
{
  void parseLine(const std::string &theString, const std::string &theDelimiter, std::vector<std::string> &theTokens);
   
  ///A Table is represented by its name, the column-names and the entries (cells)
  /**These data can be accessed by getTableName(), getColNames(), getEntry(). 
     
  A table can be parsed from a file
  with readFile(), where the first (non-whitespace) row contains the table name with the TABLE_NAME_PREFIX, 
  the second line contains the column names (comma separated), the following lines contain the entries (comma separated)
  and the table is finalized with a line containing the TABLE_END. The table can be output by the << operator.  
  
  Two tables can be concatenated with concatTable(), if their first column is identical*/
  class Table
    {
      
      static const std::string TABLE_NAME_PREFIX;
      static const std::string TABLE_END;
      
    private:
      std::vector<std::vector<std::string> > m_table;
      void readFile(ifstream& inFile);
      bool m_readSuccessful;
      
    public:
      Table(ifstream& inFile);
      Table() 								{m_readSuccessful=false;}
      
      const std::string& 					getTableName()	const					      {return m_table[0][0];		}
      const std::vector<std::string>& 	getColNames()	const							{return m_table[1];			}
      ///the first data entry (first line after column names, first column) has the index [0][0]
      const std::string& 					getEntry(int iRow, int iCol)	      {return m_table[iRow+2][iCol];}
      bool  									isReadSuccessful()                  {return m_readSuccessful;}
      
      void concatTable(const Table& table);
      friend ostream& operator<<(ostream& os, Table& table);
    };
	




/*
class ParseException : public std::exception
{
	private:
		std::string m_what;

	public:
		ParseException(std::string inWhat)	{   m_what = std::string( std::exception.what() ) + endl + inWhat ; }
	   virtual const char* what()				{   return  m_what.c_str(); }

	   virtual ~ParseException(){} 
	   };
*/
    }
#endif

