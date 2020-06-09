//
// Copyright (C) 2015-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "Parse.h"

#include <list>

//utilities
#include "StringHandler.h"

const std::string fileIO::Table::TABLE_NAME_PREFIX("Table:");
const std::string fileIO::Table::TABLE_END        ("EndOfTable");

fileIO::Table::Table(std::ifstream& inFile)
{
   m_readSuccessful = false;
   readFile(inFile);
}


void fileIO::Table::readFile(std::ifstream& inFile)
{
   std::string line;
   std::vector<std::string> theTokens;
   const char delim = ',';

   //temporary list to store arbitrary number of table-lines
   std::list< std::vector<std::string> > table; 

   // read lines until table name (==start of table) is found
   while (line.find(TABLE_NAME_PREFIX, 0) == std::string::npos)
   {  	
      std::getline(inFile, line, '\n');
      
      if (inFile.eof())
      {  
         m_readSuccessful = false;
         return;
      }
   }

   //filter first column of first line: table-name
   StringHandler::parseLine( line, delim, theTokens );

   std::vector<std::string> vec(1);
   vec[0] = theTokens[0];
   table.push_back(vec);
   
   theTokens.clear();

   //read following lines until EndOfTable
   while ( line.find(TABLE_END,0 ) == std::string::npos )
   {
      std::getline (inFile, line, '\n');
      
      if (inFile.eof())
      {  
         m_readSuccessful = false;
         return;
      }
      
      StringHandler::parseLine( line, delim, theTokens );

      table.push_back(theTokens);

      theTokens.clear();
   }

   //save list of lines in vector of lines
   m_table.resize(table.size());

   int iLine = 0;
   for (std::list< std::vector<std::string> >::const_iterator iter = table.begin(); iter != table.end(); ++iter)
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
      throw std::string("table names do not match.");
   
   //...and if number of rows...
   if ( this->m_table.size() != table.m_table.size() )	
      throw std::string("nr of table-rows do not match.");
	
   //...and key-column-entries are matching
   size_t i;
   for (i = 1; i < this->m_table.size(); ++i)
   {
      if (this->m_table[i][0] != table.m_table[i][0])
         throw std::string("key entries of table-lines do not match.");
   }
   
   //attach entries
   size_t 	nColsOrig = m_table[1].size();    //number of entries in line containing column names
   for (i = 1; i < table.m_table.size(); ++i) //loop over all rows
   {
      //add entries to line if not filled up, yet
      while (this->m_table[i].size() < nColsOrig) this->m_table[i].push_back(std::string(""));
      
      for (size_t j=1; j < table.m_table[i].size(); ++j)  //loop over all columns of table-lines to be attached
      {
         (this->m_table[i]).push_back(table.m_table[i][j]);
      }
   }
   
}

std::ostream& fileIO::operator<<(std::ostream& os, fileIO::Table& table)
{

   os << table.getTableName().c_str() << "   nRows=" << table.m_table.size() << "  nCols=" << table.m_table[1].size() << std::endl << std::flush;
   for ( size_t i=1; i < table.m_table.size(); ++i )    //loop over rows
   {
      std::string str;
      for (size_t j = 0; j < table.m_table[i].size(); ++j)//loop over columns
      {
         os << table.m_table[i][j] << " --- ";
      }
      os  << std::endl;

   }
   return os;
}
