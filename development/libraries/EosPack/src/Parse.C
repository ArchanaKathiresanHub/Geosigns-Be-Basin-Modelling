#include "stdafx.h"
#include "Parse.h"

#include <list>

using namespace std;

  

const std::string fileIO::Table::TABLE_NAME_PREFIX("Table:");
const std::string fileIO::Table::TABLE_END        ("EndOfTable");

fileIO::Table::Table(ifstream& inFile)
{
   m_readSuccessful = false;
   readFile(inFile);
}


void fileIO::Table::readFile(ifstream& inFile)
{
   std::string line;
   std::vector<std::string> theTokens;
   std::string delim=",";

   //temporary list to store arbitrary number of table-lines
   list<vector<string> > table; 

   // read lines until table name (==start of table) is found
   while (line.find(TABLE_NAME_PREFIX,0 ) == string::npos)
   {
   	
      //std::getline(inFile,line,'\n');
#ifdef sun
      static char buf[1<<14];
      inFile.getline (buf, 1<<14);
      line = buf;
#else
      std::getline (inFile, line, '\n');
#endif
      if (inFile.eof())
      {  
         m_readSuccessful = false;
         return;
      }
   }

   //filter first column of first line: table-name
   parseLine(line, delim, theTokens);
	
   vector<string> vec(1);
   vec[0] = theTokens[0];
   table.push_back(vec);
   
   theTokens.clear();


   //read following lines until EndOfTable
   while ( line.find(TABLE_END,0 ) == string::npos )
   {
 
      //std::getline(inFile,line,'\n'); 
#ifdef sun
      static char buf[1<<14];
      inFile.getline (buf, 1<<14);
      line = buf;
#else
      std::getline (inFile, line, '\n');
#endif
      
      if (inFile.eof())
      {  
         m_readSuccessful = false;
         return;
      }
      
      fileIO::parseLine(line, delim, theTokens);

      table.push_back(theTokens);

      theTokens.clear();
 			
   }

   //save list of lines in vector of lines
   m_table.resize(table.size());

   int iLine=0;
   for (list<vector<string> >::const_iterator iter=table.begin(); iter!=table.end(); ++iter)
   {
      m_table[iLine]=(*iter);       
      ++iLine;
   }
   
   if (m_table.size() >= 4) //table-name, column-names, at least one data line, EndOfTable
      m_readSuccessful = true;
   
   return;
}

void fileIO::Table::concatTable(const Table& table)
{
   //check if Tablenames are the same...
   if ( this->getTableName() != table.getTableName() ) 
      throw string("table names do not match.");
   
   //...and if number of rows...
   if ( this->m_table.size() != table.m_table.size() )	
      throw string("nr of table-rows do not match.");
	
   //...and key-column-entries are matching
   size_t i;
   for (i=1; i < this->m_table.size(); ++i )
   {
      if ( this->m_table[i][0] != table.m_table[i][0] )
         throw string("key entries of table-lines do not match.");
   }
   
   //attach entries
   size_t 	nColsOrig = m_table[1].size();      //number of entries in line containing column names
   for (i=1; i < table.m_table.size(); ++i) //loop over all rows
   {
      //add entries to line if not filled up, yet
      while (this->m_table[i].size() < nColsOrig) this->m_table[i].push_back(string(""));
      
      for (size_t j=1; j < table.m_table[i].size(); ++j)  //loop over all columns of table-lines to be attached
      {
         (this->m_table[i]).push_back(table.m_table[i][j]);
      }
   }
   
}

ostream& fileIO::operator<<(ostream& os, fileIO::Table& table)
{

   os << table.getTableName().c_str() << "   nRows=" <<  table.m_table.size()  << "  nCols=" << table.m_table[1].size() << endl << flush;
   for ( size_t i=1; i < table.m_table.size(); ++i )    //loop over rows
   {
      string str;
      for (size_t j=0; j < table.m_table[i].size(); ++j)//loop over columns
      {
         os << 		table.m_table[i][j]  << " --- ";
      }
      os  << endl;

   }
   return os;
}

void fileIO::parseLine(const std::string &theString, const std::string &theDelimiter, std::vector<std::string> &theTokens)
{
   std::string::size_type startPos=0;
   std::string::size_type endPos=0;

   std::string::size_type increment=0;
   std::string Token;

   if(theString.empty() || theDelimiter.empty())
   {
      return;
   }
   while(endPos!=std::string::npos)
   {
      endPos=theString.find_first_of(theDelimiter,startPos);
      increment=endPos-startPos;
      
      Token=theString.substr(startPos,increment);
      if(Token.size()!=0)
      {
         theTokens.push_back(Token);
      }
      startPos+=increment+1;
   }
}

