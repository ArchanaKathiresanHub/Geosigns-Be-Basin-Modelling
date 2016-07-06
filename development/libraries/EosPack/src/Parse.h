#ifndef PARSE_H
#define PARSE_H

#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

///provides functionality for parsing a Table
namespace fileIO
{
  
   /// A Table is represented by its name, the column-names and the entries (cells)
   /** These data can be accessed by getTableName(), getColNames(), getEntry(). 
     
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
      std::vector< std::vector<std::string> > m_table;
      void readFile(std::ifstream& inFile);
      bool m_readSuccessful;
      
   public:
      Table(std::ifstream& inFile);
      Table() : m_readSuccessful( false ) {;}
      
      const std::string&               getTableName() const { return m_table[0][0]; }
      const std::vector<std::string>&  getColNames()  const	{ return m_table[1];	}

      ///the first data entry (first line after column names, first column) has the index [0][0]
      const std::string&               getEntry(int iRow, int iCol) { return m_table[iRow+2][iCol]; }
      bool                             isReadSuccessful()           { return m_readSuccessful; }
      
      void concatTable(const Table& table);
      friend std::ostream& operator<<(std::ostream& os, Table& table);
   };

   std::ostream& operator<<(std::ostream& os, Table& table);
	
}
#endif

