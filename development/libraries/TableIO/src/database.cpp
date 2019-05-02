//
// Copyright (C) 2012-2015 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
// This utility allow to load and then compare table by table 2 .project3d files

#include <stdlib.h>
#include <assert.h>
#include <string>
#include <map>

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
using namespace std;

#include <string.h>

#include <sys/types.h>
#include <time.h>

#include "database.h"
#include "dataschema.h"

#include "datautils.h"

using namespace std;
using namespace datatype;

namespace database {

   int Database::s_maxFieldsPerLine = 200;		// to support the trapperiotbl
   int Database::s_fieldWidth = 4;
   int Database::s_precision = 15;



   Database::Database( DataSchema& dataSchema ) : m_dataSchema( dataSchema ), m_fileName( "" )
   {
      char *fieldWidthStr = getenv( "IBS_FIELDWIDTH" );

      if ( fieldWidthStr && strlen( fieldWidthStr ) != 0 )
      {
         Database::s_fieldWidth = atoi( fieldWidthStr );
      }


      for ( size_t i = 0; i < m_dataSchema.size(); i++ )
      {
         TableDefinition *tableDef = m_dataSchema.getTableDefinition( i );

         Table *aTable = new Table( *tableDef );

         m_tables.push_back( aTable );
      }
   }

   Database::~Database( void )
   {
      for ( TableListIterator iter = m_tables.begin(); iter != m_tables.end(); ++iter )
      {
         Table *table = *iter;
         delete table;
      }

      delete & m_dataSchema;
   }

   void Database::addHeader( const string & text )
   {
      m_header += text;
      m_header += "\n";

      cout << "Header: " << endl << m_header << endl;
   }

   void Database::clearHeader( void )
   {
      m_header = "";
   }

   Table *Database::getTable( const string & name ) const
   {
      int index = m_dataSchema.getIndex( name );

      if ( index == -1 )
         return 0;
      return m_tables[ index ];
   }

   bool Database::addTableDefinition( TableDefinition* tblDef ) {

      bool status = false;

      if ( m_dataSchema.getTableDefinition( tblDef->name() ) == nullptr ) {
         m_dataSchema.addTableDefinition( tblDef );
         Table *aTable = new Table( *tblDef );
         m_tables.push_back( aTable );
         status = true;
      }

      return status;
   }

   void Database::removeTable( Table* table ) {

      if ( table != nullptr and hasTable( table ) ) {
         m_dataSchema.removeTableDefinition( m_dataSchema.getTableDefinition( table->name() ) );
         m_tables.erase( std::remove( m_tables.begin(), m_tables.end(), table ), m_tables.end() );
      }

   }

   void Database::deleteTable( const std::string& tableName ) {

      Table* table = getTable( tableName );

      if ( table != nullptr ) {
         removeTable( table );
         delete table;
      }

   }

   bool Database::hasTable( const std::string& name ) const {
      return m_dataSchema.getIndex( name ) != -1;
   }

   bool Database::hasTable( const Table* table ) const {
      return std::find( m_tables.begin(), m_tables.end(), table ) != m_tables.end();
   }


   Table *Database::getTable( int index ) const
   {
      return index < 0 || index >= static_cast<int>(m_tables.size()) ? nullptr : m_tables[ index ];
   }

   bool Database::clearTable( const string & name, bool deleteRecords )
   {
      Table *tbl = getTable( name );

      if ( !tbl )
         return false;

      tbl->clear( deleteRecords );
      return true;
   }

   bool Database::resave( void )
   {
      if ( m_fileName != "" )
      {
         return saveToFile( m_fileName );
      }
      else
      {
         return false;
      }
   }

   bool Database::saveToFile( const string & filename )
   {
      bool result;

      if ( filename.length() == 0 )
         return false;

      ofstream outfile;

      outfile.open( filename.c_str(), ios::out );

      if ( outfile.fail() )
      {
         cerr << "Error occurred during opening file " << filename << endl;
         return false;
      }

      outfile << ";" << endl;
      time_t secs = time( 0 );

      outfile << "; START - " << filename << " " << ctime( &secs ) << ";" << endl;

      result = saveToStream( outfile );

      outfile.close();
      return result;
   }

   bool Database::saveToStream( ostream & ofile )
   {
      bool result = true;

      saveHeader( ofile );

      for ( TableListIterator iter = m_tables.begin(); iter != m_tables.end(); ++iter )
      {
         Table *table = *iter;

         result = table->saveToStream( ofile );

         if ( !result )
            return false;
      }

      return true;
   }

   bool Database::saveHeader( ostream & ofile )
   {
      if ( m_header.length() == 0 ) return true;

      string modHeader = m_header;

      size_t pos = 0;
      do
      {
         modHeader.insert( pos, ";! " );
         // cerr << "modHeader" << modHeader << endl;
      } while ( (pos = modHeader.find( "\n", pos + 4 )) != string::npos &&
                pos < (modHeader.length() - 2) 
              );

      ofile << modHeader;
      if ( modHeader[ modHeader.length() - 1 ] != '\n' )
         cerr << endl;

      return true;
   }

   Database *Database::CreateFromSchema( const DataSchema & dataSchema )
   {
      DataSchema *copy = dataSchema.deepCopy();

      return new Database( *copy );
   }

   Database *Database::CreateFromFile( const string & filename, const DataSchema & dataSchema )
   {
      Database *database = Database::CreateFromSchema( dataSchema );

      if ( database->loadFromFile( filename ) )
         return database;
      else
      {
         // delete database;
         return 0;
      }
   }

   bool Database::loadFromFile( const string & filename )
   {
      bool result;

      if ( filename.length() == 0 )
         return false;

      ifstream infile;

      infile.open( filename.c_str(), ios::in );

      if ( infile.fail() )
      {
         cerr << "Error occurred during opening file " << filename << endl;
         return false;
      }

      result = loadFromStream( infile );
      infile.close();

      if ( result ) 
      {
         m_fileName = filename;
      }

      return result;
   }

   bool Database::reload( std::string tableName )
   {
      if ( m_fileName.empty() ) { return false; }
 
      if ( tableName.empty() )
      {
         m_tablesInFile.clear();
      
         for ( auto tbl : m_tables ) { delete tbl; }
         m_tables.clear();

         return loadFromFile( m_fileName );
      }
      
      Table* table = getTable( tableName );
      if ( table != nullptr ) // delete records in table 
      {  
         table->clear();
      }

      // reopen file
      ifstream infile( m_fileName.c_str(), ios::in );
      if ( infile.fail() ) { cerr << "Error occurred during opening file " << m_fileName << endl; return false; }

      while ( true )
      {
         string line;
         if ( !loadLine( infile, line, false ) ) break;
         findAndRemoveDelimiters( line, "[]" );

         if ( line != tableName ) // skip all tables with different name
         {
            if ( !skipTableFromStream( infile ) ) { return false; }
         }
         else
         {
            Table * table = getTable( line );
            if ( table == nullptr ) { return false; }
            if ( !table->loadFromStream( infile ) ) { return false; }
            break;
         }
      }
      infile.close();
      return true;
   }

   bool Database::loadFromStream( istream & infile )
   {
      string line;

      bool checkForHeaders = true;
      while ( true )
      {
         if ( !loadLine( infile, line, checkForHeaders ) )
            return true;

         if ( line.find( ";! " ) == 0 )
         {
            addHeader( line.substr( 3, string::npos ) + "\n" );
            continue;
         }

         findAndRemoveDelimiters( line, "[]" );
         checkForHeaders = false;

         Table *table = getTable( line );

         if ( !table )               // skip over specification
         {
            if ( !skipTableFromStream( infile ) ) { return false; }
         }
         else
         {
            if ( !table->loadFromStream( infile ) ) { return false; }
         }
         m_tablesInFile[line] = table != nullptr ? true : false;
      }
   }

   bool Database::skipTableFromStream( istream & infile )
   {
      string line;

      while ( true )
      {
         if ( !loadLine( infile, line ) )
            return false;
         if ( findAndRemoveDelimiters( line, "[]" ) && line == "End" )
            return true;
      }
   }


   Table::Table( const TableDefinition & tableDefinition ) :
      m_tableDefinition( tableDefinition )
   {
      setVersion( tableDefinition.version() );
   }

   Table::~Table( void )
   {
      for ( RecordListIterator iter = m_records.begin(); iter != m_records.end(); ++iter )
      {
         Record *record = *iter;
         delete record;
      }
   }

   void Table::copyTo( Table* table ) const {

      if ( table != nullptr and table->name() == m_tableDefinition.name() ) {

         for ( size_t i = 0; i < m_records.size(); ++i ) {
            table->addRecord( m_records[ i ]->deepCopy( table ) );
         }

      }

   }

   void Table::clear( bool deleteRecords )
   {
      if ( deleteRecords )
      {
         for ( RecordListIterator iter = m_records.begin(); iter != m_records.end(); ++iter )
         {
            Record *record = *iter;
            delete record;
         }
      }

      m_records.clear();
   }


   Record *Table::createRecord( bool addToFile )
   {
      Record *record = new Record( m_tableDefinition, this );

      if ( addToFile )
         addRecord( record );

      return record;
   }

   Table::iterator Table::findRecordPosition( Record * record )
   {
      if ( !record ) return end();

      Table::iterator iter;
      for ( iter = begin(); iter != end(); ++iter )
      {
         if ( *iter == record )
         {
            break;
         }
      }

      return iter;
   }

   bool Table::moveRecord( Record * record1, Record * record2 )
   {
      if ( record1 == record2 ) return true;

      bool success = false;
      success = eraseRecord( record1 );

      if ( success ) success = insertRecord( record1, record2 );

      return success;
   }

   bool Table::eraseRecord( Record * record )
   {
      assert( record );

      Table::iterator iter = findRecordPosition( record );

      if ( iter != end() )
      {
         m_records.erase( iter );
         return true;
      }
      else
      {
         return false;
      }
   }

   bool Table::insertRecord( Record * record1, Record * record2 )
   {
      assert( record1 );

      Table::iterator iter = findRecordPosition( record2 );

      m_records.insert( iter, record1 );
      return true;
   }

   void Table::addRecord( Record * record )
   {
      // assert (record->getTable () == this);
      m_records.push_back( record );
   }



   bool Table::deleteRecord( Record * record )
   {

      Table::iterator iter = findRecordPosition( record );

      if ( iter != end() )
      {
         m_records.erase( iter );
         delete record;
      }
      return true;
   }

   Record * Table::removeRecord( Record * record )
   {
      Table::iterator iter = findRecordPosition( record );

      iter = removeRecord( iter );
      return getRecord( iter );
   }


   Table::iterator Table::removeRecord( Table::iterator & iter )
   {
      if ( iter != end() )
      {
         return ( m_records.erase( iter ) );
      }

      return iter;
   }

   Record * Table::findRecord( const std::string & fieldName, const std::string & value )
   {
      int index = getIndex( fieldName );
      if ( index < 0 ) return 0;

      Table::iterator iter;
      for ( iter = begin(); iter != end(); ++iter )
      {
         Record * record = *iter;
         const std::string & foundValue = record->getValue<std::string>( index );

         if ( foundValue == value ) return record;
      }
      return 0;
   }

   Record * Table::findRecord( const std::string & field1, const std::string & value1, const std::string & field2, const std::string & value2, Record * other )
   {
      int index1 = getIndex( field1 );
      int index2 = getIndex( field2 );
      if ( index1 < 0 || index2 < 0 ) return 0;

      Table::iterator iter;
      for ( iter = begin(); iter != end(); ++iter )
      {
         Record * record = *iter;
         if ( record != other &&
              value1 == record->getValue<std::string>( index1 ) &&
              value2 == record->getValue<std::string>( index2 ) )
            return record;
      }
      return 0;
   }

   void Table::sort( OrderingFunc func )
   {
      std::sort( m_records.begin(), m_records.end(), func );
   }

   void Table::stable_sort( OrderingFunc func )
   {
      std::stable_sort( m_records.begin(), m_records.end(), func );
   }

   struct LocalTableSorter
   {
      LocalTableSorter( Table * tbl, const std::vector<std::string> & fldList )
      {
         const database::TableDefinition & tblDef = tbl->getTableDefinition();

         // cache fields index and data type
         if ( fldList.empty() )
         {
            for ( int i = 0; i < static_cast<int>(tblDef.size()); ++i )
            {
               m_fldIDs.push_back( i );
               m_fldTypes.push_back( tblDef.getFieldDefinition( i )->dataType() );
            }
         }
         else
         {
            for ( size_t i = 0; i < fldList.size(); ++i )
            {
               int ind = tbl->getIndex( fldList[ i ] );
               if ( ind < 0 ) continue; // just ignore unknown fields
               m_fldIDs.push_back( ind );
               m_fldTypes.push_back( tblDef.getFieldDefinition( ind )->dataType() );
            }
         }
      }

      //  this function is used as less operator for the strict weak ordering
      bool operator() ( const Record * r1, const Record * r2 )
      {
         assert( r1 != NULL && r2 != NULL );

         for ( size_t i = 0; i < m_fldIDs.size(); ++i )
         {
            int id = m_fldIDs[ i ];
            switch ( m_fldTypes[ i ] )
            {
            case datatype::Bool:   { bool   v = r1->getValue<bool  >( id ); bool   w = r2->getValue<bool  >( id ); if ( v != w ) return v < w; } break;
            case datatype::Int:    { int    v = r1->getValue<int   >( id ); int    w = r2->getValue<int   >( id ); if ( v != w ) return v < w; } break;
            case datatype::Long:   { long   v = r1->getValue<long  >( id ); long   w = r2->getValue<long  >( id ); if ( v != w ) return v < w; } break;
            case datatype::Float:  { float  v = r1->getValue<float >( id ); float  w = r2->getValue<float >( id ); if ( v != w ) return v < w; } break;
            case datatype::Double: { double v = r1->getValue<double>( id ); double w = r2->getValue<double>( id ); if ( v != w ) return v < w; } break;
            case datatype::String: { string v = r1->getValue<string>( id ); string w = r2->getValue<string>( id ); if ( v != w ) return v < w; } break;
            case datatype::NoDataType: break;
            }
         }
         return false;
      }

      std::vector<int>                 m_fldIDs;
      std::vector<datatype::DataType>  m_fldTypes;
   };

   void Table::stable_sort( const std::vector<std::string> & fldList )
   {
      std::stable_sort( m_records.begin(), m_records.end(), LocalTableSorter( this, fldList ) );
   }

   void Table::unique( EqualityFunc equalityFunc, MergeFunc mergeFunc )
   {
      for ( Table::iterator iter = begin(); iter != end() && ( iter + 1 ) != end();)
      {
         Record *record = *iter;
         Record *recordNext = *( iter + 1 );

         if ( equalityFunc( record, recordNext ) )
         {
            if ( mergeFunc )
            {
               mergeFunc( record, recordNext );
            }

            m_records.erase( iter + 1 );
            delete recordNext;
         }
         else
            ++iter;
      }
   }

   bool Table::saveToFile( const string & filename )
   {
      bool result;

      if ( filename.length() == 0 )
         return false;

      ofstream outfile;

      outfile.open( filename.c_str(), ios::out );

      if ( outfile.fail() )
      {
         cerr << "Error occurred during opening file " << filename << endl;
         return false;
      }

      result = saveToStream( outfile );

      outfile.close();

      return result;
   }

   bool Table::saveToStream( ostream & ofile )
   {
      ofile << ";" << endl;

      bool rowBased;

      if ( static_cast<int>(m_tableDefinition.size()) > Database::GetMaxFieldsPerLine() )
         rowBased = true;
      else
         rowBased = false;

      if ( !m_tableDefinition.saveToStream( ofile, rowBased ) )
         return false;

      for ( Table::iterator iter = begin(); iter != end(); ++iter )
      {
         Record *record = *iter;

         bool result = record->saveToStream( ofile, rowBased );

         if ( !result )
            return false;
      }

      ofile << "[End]" << endl;
      ofile << ";" << endl;

      return !ofile.fail();
   }

   bool Table::loadFromStream( istream & infile )
   {
      vector < int >dataToFieldMap;

      string line;

      if ( !loadLine( infile, line, true ) ) { return false; }

      int version = m_tableDefinition.version(); 
      if ( line.find( ";v" ) == 0 ) // if table has version - read it
      {
         version = stoi( line.substr( 2 ) );
         if ( !loadLine( infile, line ) ) { return false; }
      }
      setVersion( version );

      if ( findAndRemoveDelimiters( line, "[]" ) && line == "End" )
      {
         return true;              // empty row-based table without Field-Specs
      }
      else if ( findAndRemoveDelimiters( line, "<>" ) && line == "Row" )
      {
         return loadRowBasedRecordsFromStream( infile );
      }
      else
      {
         if ( !loadDataToFieldMapFromLine( line, dataToFieldMap ) ) { return false; }
         if ( !loadUnitsFromStream(        infile               ) ) { return false; }

         return loadRecordsFromStream( infile, dataToFieldMap );
      }
   }

   bool Table::loadDataToFieldMapFromLine( string & line, vector < int >&dataToFieldMap )
   {
      size_t fieldNameStartPos = 0;
      size_t fieldNameEndPos = 0;
      size_t fieldNameLength = 0;

      int lineIndex = -1;

      const string separators = " \t";

      while ( ( fieldNameStartPos =
                line.find_first_not_of( separators, fieldNameEndPos ) ) != string::npos )
      {
         lineIndex++;

         fieldNameEndPos = line.find_first_of( separators, fieldNameStartPos );

         if ( fieldNameEndPos == string::npos )
            fieldNameLength = string::npos;
         else
            fieldNameLength = fieldNameEndPos - fieldNameStartPos;

         string fieldName = line.substr( fieldNameStartPos, fieldNameLength );

         dataToFieldMap.push_back( m_tableDefinition.getIndex( fieldName ) );
      }

      return true;
   }

   bool Table::loadUnitsFromStream( istream & infile )
   {
      string line;

      // remember the position in the stream
      streampos pos = infile.tellg();

      if ( !loadLine( infile, line ) )
         return false;

      const string separators = " \t";
      size_t firstNonSpace = line.find_first_not_of( separators, 0 );

      if ( firstNonSpace == string::npos || line[ firstNonSpace ] != '(' )
      {
         // this is not a unit line, we need to revert to the remembered position
         infile.seekg( pos );
      }
      else
      {
         // Units line, we should really compare with the specified units ....
      }
      return true;
   }

   bool Table::loadRecordsFromStream( istream & infile, vector < int >&dataToFieldMap )
   {
      string line;

      while ( true )
      {
         if ( !loadLine( infile, line ) )
            return false;
         if ( findAndRemoveDelimiters( line, "[]" ) && line == "End" )
            return true;

         Record *record = createRecord();

         if ( !record->loadFromLine( line, dataToFieldMap ) )
            return false;
      }
   }

   bool Table::loadRowBasedRecordsFromStream( istream & infile )
   {
      while ( true )
      {
         Record *record = createRecord();

         while ( true )
         {
            string line;

            string fieldName;
            string fieldValue;
            int fieldIndex;

            size_t linePos = 0;

            if ( !loadLine( infile, line ) )
               return false;

            if ( findAndRemoveDelimiters( line, "[]" ) && line == "End" )
               return true;

            if ( findAndRemoveDelimiters( line, "<>" ) && line == "Row" )
               break;

            linePos = loadWordFromLine( line, linePos, fieldName );

            if ( linePos == string::npos )
               return false;

            fieldIndex = m_tableDefinition.getIndex( fieldName );
            if ( fieldIndex == -1 )
               continue;           // unknown field

            linePos = loadWordFromLine( line, linePos, fieldValue );

            if ( linePos == string::npos )
               return false;

            if ( !record->assignFromStringToIndex( fieldValue, fieldIndex ) )
               return false;
         }
      }
   }


   Record::Record( const TableDefinition & tableDefinition, Table * table )
      : m_table( table ), m_tableDefinition( tableDefinition )
   {
      createFields();
   }

   Record::Record( const Record & other )
      : m_table( other.m_table ),
      m_tableDefinition( other.m_tableDefinition ),
      m_fields()
   {
      for ( size_t i = 0; i < other.m_fields.size(); ++i )
         m_fields.push_back( other.m_fields[ i ]->clone() );
   }

   Record::Record( const Record & record, Table * table )
      : m_table( table ),
      m_tableDefinition( table->getTableDefinition() ),
      m_fields()
   {
      for ( size_t i = 0; i < record.m_fields.size(); ++i ) {
         m_fields.push_back( record.m_fields[ i ]->clone() );
      }
   }


   Record* Record::deepCopy( Table * table ) const {

      Record* newRecord = nullptr;

      if ( table != nullptr and table->name() == m_table->name() ) {
         newRecord = new Record( *this, table );
      }

      return newRecord;
   }

   void Record::createFields( void )
   {
      for ( size_t i = 0; i < m_tableDefinition.size(); i++ )
      {
         FieldDefinition *fieldDef = m_tableDefinition.getFieldDefinition( i );

         std::shared_ptr<AbstractField> field;

         switch ( fieldDef->dataType() )
         {
         case Bool:
            field.reset( new Field < bool >( *fieldDef ) );

            break;
         case Int:
            field.reset( new Field < int >( *fieldDef ) );

            break;
         case Long:
            field.reset( new Field < long >( *fieldDef ) );

            break;
         case Float:
            field.reset( new Field < float >( *fieldDef ) );

            break;
         case Double:
            field.reset( new Field < double >( *fieldDef ) );

            break;
         case String:
            field.reset( new Field < string >( *fieldDef ) );

            break;
         default:
            assert( false );
         }

         m_fields.push_back( field );
      }
   }

   const std::string & Record::tableName() const
   {
      return getTable()->name();
   }

   void Record::destroyYourself( void )
   {
      getTable()->deleteRecord( this );
   }

   void Record::addToTable( void )
   {
      getTable()->addRecord( this );
   }

   std::shared_ptr<AbstractField> Record::getField( const string & name, int *cachedIndex ) const
   {
      int hint = ( cachedIndex ? *cachedIndex : -1 );

      int index = m_tableDefinition.getIndex( name, hint );

      if ( cachedIndex )
         *cachedIndex = index;

      return index == -1 ? std::shared_ptr<AbstractField>() : m_fields[ index ];
   }

   template <> bool Field < bool >::saveToStream( ostream & ofile, int &borrowed )
   {

      string valueString = m_value == false ? "false" : "true";

      return saveStringToStream( ofile, valueString, borrowed, Database::GetFieldWidth() );
   }

   template <> bool Field < string >::saveToStream( ostream & ofile, int &borrowed )
   {
      const string quote = "\"";

      string valueString = m_value;

      size_t wordQuotePos;
      size_t wordStartPos = 0;

      do
      {
         wordQuotePos = valueString.find_first_of( quote, wordStartPos );
         if ( wordQuotePos != string::npos )
         {
            valueString.insert( wordQuotePos, "\\" );
            wordStartPos = wordQuotePos + 2;
         }
         else
         {
            break;
         }
      } while ( true );

      valueString.insert( 0, "\"" );
      valueString += "\"";

      return saveStringToStream( ofile, valueString, borrowed, Database::GetFieldWidth() );
   }


   template < class Type > bool Field < Type >::saveToStream( ostream & ofile, int &borrowed )
   {

      ostringstream buf;
      buf.precision( Database::GetPrecision() );
      buf << m_value;
      std::string return_string = buf.str();

      return saveStringToStream( ofile, return_string, borrowed, Database::GetFieldWidth() );
   }


   template < class Type > bool Field < Type >::assignFromString( const string & word )
   {
      assert( false );
      return false;
   }

   template <> bool Field < bool >::assignFromString( const string & word )
   {
      if ( word.length() == 0 )
         return false;

      if ( word == "false" )
         m_value = false;
      else
         m_value = true;

      return true;
   }

   template <> bool Field < string >::assignFromString( const string & word )
   {
      if ( word.length() == 0 )
         return false;

      m_value = word;
      return true;
   }

   template <> bool Field < int >::assignFromString( const string & word )
   {
      if ( word.length() == 0 )
         return false;

      m_value = atoi( word.c_str() );

      return true;
   }

   template <> bool Field < long >::assignFromString( const string & word )
   {
      if ( word.length() == 0 )
         return false;

      m_value = atol( word.c_str() );

      return true;
   }

   template <> bool Field < float >::assignFromString( const string & word )
   {
      if ( word.length() == 0 )
         return false;

      m_value = (float)atof( word.c_str() );

      return true;
   }

   template <> bool Field < double >::assignFromString( const string & word )
   {
      if ( word.length() == 0 )
         return false;

      m_value = atof( word.c_str() );

      return true;
   }

   void Record::printOn( ostream & str )
   {
      saveToStream( str, false );
   }

   bool Record::saveToStream( ostream & ofile, bool rowBased )
   {
      bool result = true;

      if ( rowBased )
         ofile << "<Row>" << endl;

      int borrowed = 0;

      for ( size_t order = 0; order < m_tableDefinition.persistentSize(); order++ )
      {
         const size_t position = m_tableDefinition.getPosition( order );

         if ( rowBased )
         {
            borrowed = 0;

            const FieldDefinition & fieldDef = m_fields[ position ]->getFieldDefinition();

            fieldDef.saveNameToStream( ofile, borrowed );
            ofile << "      ";

            m_fields[ position ]->saveToStream( ofile, borrowed );
            ofile << "      ";

            fieldDef.saveUnitToStream( ofile, borrowed );
            ofile << endl;
         }
         else
         {
            if ( order != 0 )
            {
               ofile << " ";
            }

            m_fields[ position ]->saveToStream( ofile, borrowed );
         }
      }

      if ( !rowBased )
         ofile << endl;
      return result;
   }


   bool Record::loadFromLine( const std::string & line, vector < int > & dataToFieldMap )
   {
      size_t linePos = 0;

      for ( vector < int >::iterator mapIter = dataToFieldMap.begin();
            mapIter != dataToFieldMap.end(); ++mapIter )
      {
         string fieldValue;

         if ( linePos == string::npos )
            return false;

         linePos = loadWordFromLine( line, linePos, fieldValue );

         int toIndex = *mapIter;

         if ( toIndex == -1 )
         {
            // data belongs to an unknown field, skip over it
            continue;
         }

         if ( !assignFromStringToIndex( fieldValue, toIndex ) )
            return false;
      }

      return true;
   }

   bool Record::assignFromStringToIndex( const string & word, int toIndex )
   {
      FieldDefinition *fieldDef = m_tableDefinition.getFieldDefinition( toIndex );
      assert( fieldDef->isValid() );
      getField( toIndex )->assignFromString( word );

      return true;
   }

   template < class Type > void checkType( const Type &, const datatype::DataType type );

   template <> void checkType < bool >( const bool &, const datatype::DataType type )
   {
      assert( type == datatype::Bool );
   }

   template <> void checkType < int >( const int &, const datatype::DataType type )
   {
      assert( type == datatype::Int );
   }

   template <> void checkType < long >( const long &, const datatype::DataType type )
   {
      assert( type == datatype::Long );
   }

   template <> void checkType < float >( const float &, const datatype::DataType type )
   {
      assert( type == datatype::Float );
   }

   template <> void checkType < double >( const double &, const datatype::DataType type )
   {
      assert( type == datatype::Double );
   }

   template <> void checkType < string >( const string &, const datatype::DataType type )
   {
      assert( type == datatype::String );
   }

}
