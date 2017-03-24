//                                                                      
// Copyright (C) 2012-2017 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 
#include "datautils.h"

#include <cstdlib>
#include <cassert>
#include <ctype.h>
#include <cstring>

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm> // contains std::sort
#include <stdexcept>

//#define DEBUG 1

using namespace std;

typedef vector<string> StringList;
typedef vector<int>    IntList;

typedef map <string, string,       less<string>> StringToStringMap;
typedef map <string, int,          less<string>> StringToIntMap;
typedef map <string, bool,         less<string>> StringToBoolMap;
typedef map <string, StringList,   less<string>> StringToStringListMap;
typedef map <string, IntList,      less<string>> StringToIntListMap;

StringToStringListMap FieldTables;       ///< Contains the list of table names for each field
StringList            FieldList;         ///< Contains the list of field names
StringToStringMap     FieldDescriptions; ///< Contains a description for each field
StringToStringMap     FieldUnits;        ///< Contains the unit of each field
StringToStringMap     FieldTypes;        ///< Contains the type of each field
StringToStringMap     FieldDefaults;     ///< Contains the default value of each field
StringToIntMap        FieldIndices;      ///< Contains the index a field was assigned to.
StringToBoolMap       FieldRelation;     ///< Specifies whether this field is a relation to another table

StringList            TableList;         ///< Contains the list of tables
StringToStringMap     TableDescriptions; ///< description for each table
StringToIntMap        TableVersions;     ///< Version number of each table
StringToStringListMap TableFields;       ///< Fields of each table

map<pair<string,string>, pair<string,string>> TableRelations; /// Keep relations info between tables Field.Table->Field.Table

int lineno          = 0;
int alignWidth      = 15;
int versionNumStart = 100;

static const string separators = " \t";
static const string badSymbols = "+-*/=: ";

/// Reads the next non-comment line from a file
bool loadSpecLine( ifstream & infile, string & line )
{
   while ( true )
   {
      getline( infile, line, '\n' );
      lineno++;
      if ( infile.eof() ) { return false; } // no more lines in file

      size_t firstNonSpace = line.find_first_not_of( separators, 0 );

      if ( firstNonSpace         != string::npos && // if it non empty line and
           line[ firstNonSpace ] != ';'             // not a comment possibly with leading spaces
         ) { return true; }
   }
}

/// if a line contains the given keyword, return the associated value
bool getKeywordValue( const string & line, const string & keyword, string & value )
{
   size_t keywordStartPos = line.find_first_not_of( separators, 0 );

   if ( keywordStartPos == string::npos || line.find( keyword, keywordStartPos ) != keywordStartPos )
   {
      return false;
   }

   size_t valueStartPos = line.find( ':', keywordStartPos + keyword.length() );

   if ( valueStartPos == string::npos ) { return false; }

   loadWordFromLine( line, valueStartPos + 1, value );

   return true;
}

/// Get the index of a field in a table
int GetIndex( string & tableName, string & fieldName )
{
   const StringList & fieldList = TableFields[ tableName ];

   auto pos = find( fieldList.begin(), fieldList.end(), fieldName );
   return pos == fieldList.end() ? -1 : static_cast<int>( pos - fieldList.begin() );
}

/// Check if a field has the same index in each table it features in
bool CheckIndex( string & fieldName, string & tableNameUsed, int indexUsed )
{
   for ( auto & tableName : FieldTables[ fieldName ] )
   {
      if ( GetIndex( tableName, fieldName ) != indexUsed ) { return false; }
   }
   return true;
}

/// Check if the specified fieldIndex is the correct index for the field specified by fieldName
/// in the table specified by tableName.
bool IsValidIndex( string & tableName, string & fieldName, int fieldIndex )
{
   StringList fieldList = TableFields[ tableName ];

   for ( auto & tableFieldName : fieldList )
   {
      if ( tableFieldName != fieldName )
      {
         if ( FieldIndices[ tableFieldName ] == fieldIndex )
         {
            return false;
         }
      }
   }
   return true;
}


/// Try to assign an index to each field such that a field has the same index in each table.
/// Return the number of fields for which the attempt was unsuccessful.
int OrderFields()
{
   int numErrors = 0;

   for ( auto & fieldName : FieldList ) { FieldIndices[ fieldName ] = -1; } // reset all field indexes to -1
   for ( auto & fieldName : FieldList )
   {
      if ( FieldTables[ fieldName ].size() == 0 ) { continue; }

#if DEBUG
      cerr << "Assigning field " << fieldName << " (" << FieldIndices[ fieldName ] << ")" << endl;
#endif

      for ( int fieldIndicesIndex = 0; true; fieldIndicesIndex++ )
      {
         bool validIndex = true;
         bool outOfRange = false;

         for ( auto & tableName : FieldTables[ fieldName ] )
         {
            if ( fieldIndicesIndex >= TableFields[ tableName ].size() )
            {
               outOfRange = true;
               break;
            }

            if ( !IsValidIndex( tableName, fieldName, fieldIndicesIndex ) )
            {
               validIndex = false;
               break;
            }
         }

         // no valid index could be found in combination with the previously positioned fields.
         // put this field temporarily at the back of each table
         if ( outOfRange )
         {
            FieldIndices[ fieldName ] = 999;
            numErrors++;
#if DEBUG
            cerr << "Assigning " << 999 << " to " << fieldName << endl;
#endif
            break;
         }

         if ( validIndex )
         {
#if DEBUG
            cerr << "Assigning " << fieldIndicesIndex << " to " << fieldName << endl;
#endif
            FieldIndices[ fieldName ] = fieldIndicesIndex;
            break;
         }
      }
   }
   return numErrors;
}

string cleanNameForSpecialSymbols( string inName )
{
   string outName = inName;
   
   for_each( outName.begin(), outName.end(), []( char & ch ) { if ( badSymbols.find( ch ) != string::npos ) { ch = '_'; } } );
   
   return outName;
}

void ParseSpecFile( const string & specFile )
{
   ifstream infile( specFile.c_str(), ios::in );
   if ( infile.fail() ) { throw runtime_error( string( "Error occurred during opening input file " ) + specFile ); }

   string line;
   string value;
   string fieldName;
   string description;
   string unit;
   string type;
   string defaultValue;

   // find the place where property definitions are located
   while ( loadSpecLine( infile, line ) && line.find( "START-properties" ) == string::npos ) { ; }

   // Read in all the fields possible.
   while ( loadSpecLine( infile, line ) )
   {
      bool endProps = line.find( "END-properties" ) != string::npos ? true : false;
      bool startNewProp = getKeywordValue( line, "Identification", value );

      if ( endProps || startNewProp )
      {
         if ( fieldName != "" )
         {
            FieldList.push_back( fieldName );
            FieldDescriptions[ fieldName ] = description;
            FieldUnits[        fieldName ] = unit;
            FieldTypes[        fieldName ] = type;
            FieldDefaults[     fieldName ] = defaultValue;
            FieldRelation[     fieldName ] = false;
         }

         fieldName    = "";
         description  = "";
         unit         = "";
         type         = "";
         defaultValue = "";
      }

      if ( endProps ) { break; }
      if ( startNewProp ) { fieldName = value; }

      if ( getKeywordValue( line, "Description", value ) ) { description = value; }
      else if ( getKeywordValue( line, "Unit of measurement", value ) )
      {
         size_t fieldUnitStartPos = value.find_first_of( separators, 0 );

         if ( fieldUnitStartPos != string::npos ) { value.erase( fieldUnitStartPos ); }
         unit = value;
      }
      else if ( getKeywordValue( line, "Type", value ) )
      {
         if (      value.find( "boolean"  ) != string::npos ) { value = "Int";    }
         else if ( value.find( "smallInt" ) != string::npos ) { value = "Int";    }
         else if ( value.find( "smallint" ) != string::npos ) { value = "Int";    }
         else if ( value.find( "integer"  ) != string::npos ) { value = "Int";    }
         else if ( value.find( "float"    ) != string::npos ) { value = "Double"; }
         else if ( value.find( "double"   ) != string::npos ) { value = "Double"; }
         else if ( value.find( "string"   ) != string::npos ) { value = "String"; }
         else { throw runtime_error( string( "Unknown column Type " ) + value + " at line " + to_string( lineno ) ); }

         type = value;
      }
      else if ( getKeywordValue( line, "Default", value ) ) { defaultValue = value; }
   }

   // roll file till table definitions are
   while ( loadSpecLine( infile, line ) && line.find( "START-property-tables" ) == string::npos ) { ; }

   // read in all the tables
   bool tableIO = false;
   string tableName;

   while ( loadSpecLine( infile, line ) && line.find( "END-property-tables" ) == string::npos )
   {
      if ( getKeywordValue( line, "Identification", value ) )
      {
         tableIO = true;
         tableName = value;
         TableList.push_back( value );
      }

      if ( tableIO && getKeywordValue( line, "Description", value ) ) { TableDescriptions[ tableName ] = value; }
      if ( tableIO && getKeywordValue( line, "Version",     value ) ) { TableVersions[ tableName ] = stoi( value ); }
      if ( tableIO && getKeywordValue( line, "Column",      value ) )
      {
         auto pos = value.find( "->" ); // column is a reference to another table
         if ( pos != string::npos )
         {
            auto refColPos      = value.find( "." );
            string colName      = value.substr( 0, pos );
            string refTableName = value.substr( pos+2, refColPos - pos - 2 );
            string refColName   = value.substr( refColPos+1 );

            FieldRelation[ colName ] = true;
            TableRelations[pair<string,string>( colName, tableName )] = pair<string,string>( refColName, refTableName );

            value = colName;
            if ( find( FieldList.begin(), FieldList.end(), value ) == FieldList.end() )
            {
               FieldList.push_back( value );
               FieldDescriptions[ value ] = FieldDescriptions[ refColName ];
               FieldUnits[        value ] = FieldUnits[        refColName ];
               FieldTypes[        value ] = FieldTypes[        refColName ];
               FieldDefaults[     value ] = FieldDefaults[     refColName ];
            }
         }

         TableFields[ tableName ].push_back( value ); 
         FieldTables[ value     ].push_back( tableName );
      }
   }
}

//#define API_UNIT_TEST

void GenerateCMBAPI( const string & schemaName, const string & schemaDir, bool verbose )
{
   // construct output file names
   // convert file names to low case
   string lowCaseSchemaName = schemaName;
   transform( lowCaseSchemaName.begin(), lowCaseSchemaName.end(), lowCaseSchemaName.begin(), ::tolower );

   string sourceFile  = schemaDir + lowCaseSchemaName + "API.cpp";
   string headerFile  = schemaDir + lowCaseSchemaName + "API.h";

   // open all the output files required
   ofstream sourceOut( sourceFile.c_str(), ios::out );
   if ( sourceOut.fail() ) { throw runtime_error( "Error occurred during opening output file " + sourceFile ); }

   ofstream headerOut( headerFile.c_str(), ios::out );
   if ( headerOut.fail() ) { throw runtime_error( "Error occurred during opening output file " + headerFile ); }

   // Header file generation
   headerOut << 
R"(#include "database.h"


namespace database
{
   class ProjectFileHandler;
}

namespace bmapi
{
   typedef size_t RecNumber;
)";
   sourceOut << "#include \"" << lowCaseSchemaName + "API.h\"\n";
   sourceOut << "#include \"cauldronschemafuncs.h\"\n\n";
   sourceOut << "#include \"ProjectFileHandler.h\"\n\n";

#ifdef API_UNIT_TEST
   sourceOut << "#include <iostream>\n";
   sourceOut << "#include <iomanip>\n";
#endif

   sourceOut << "namespace bmapi\n{\n";
   
   for ( auto tableName : TableList )
   {
      // Declare class and auxillary Record structure for the current table
      headerOut << "   // " << TableDescriptions[ tableName ] << "\n";
      headerOut << "   class Table" << tableName << "\n";
      headerOut << "   {\n";
      headerOut << "   public:\n";
      headerOut << "      struct Record\n";
      headerOut << "      {\n";
      for ( auto fieldName : TableFields[ tableName ] )
      {
         const string & fieldType = FieldTypes[ fieldName ];
         if(       fieldType == "Int"    ) { headerOut << "         int          "; }
         else if ( fieldType == "Double" ) { headerOut << "         double       "; }
         else if ( fieldType == "String" ) { headerOut << "         std::string  "; }
         else { throw runtime_error( string( "Unknown column Type " ) + fieldType ); }
         string fieldDescr = FieldDescriptions[ fieldName ];
         headerOut << "m_" << fieldName << "; // " << fieldDescr << "\n";
      }
      headerOut << "\n         Record(); // default constructor. Intitilized structure with default field values\n\n";
      headerOut << "      };\n\n";

      headerOut << "      Table" << tableName << "( database::Table * tbl );\n";
      headerOut << "      ~Table" << tableName << "();\n\n";
      headerOut << "      RecNumber  addRecord(  const Record & rec );\n";
      headerOut << "      Record getRecord( RecNumber rec );\n\n";
      headerOut << "      size_t size() const { return m_recordList.size(); }\n";
      headerOut << "      std::vector<Record>::const_iterator begin() const { return m_recordList.begin(); }\n";
      headerOut << "      std::vector<Record>::const_iterator end()   const { return m_recordList.end(); }\n\n";

      for ( auto fieldName : TableFields[ tableName ] )
      {
         const string & fieldType = FieldTypes[ fieldName ];
         string fieldGetType;
         string fieldSetType;
         if(       fieldType == "Int"    ) { fieldSetType = fieldGetType = "int"; }
         else if ( fieldType == "Double" ) { fieldSetType = fieldGetType = "double"; }
         else if ( fieldType == "String" ) { fieldGetType = "std::string"; fieldSetType = "const " + fieldGetType + " & "; }
         else { throw runtime_error( string( "Unknown column Type " ) + fieldType ); }

         headerOut << "      " << fieldGetType << " get" << fieldName << "( RecNumber id ) const;\n";
         headerOut << "      void set" << fieldName << "( RecNumber id, " << fieldSetType << " val );\n\n";
      }
      headerOut << "   protected:\n";
      headerOut << "      std::vector<Record> m_recordList;\n";
      headerOut << "   };\n\n";

      // Create default constructor for Record
      sourceOut << "   Table" << tableName << "::Record::Record()\n";
      sourceOut << "   {\n";
      for ( auto fieldName : TableFields[ tableName ] )
      {     
         const string & defVal  = FieldDefaults[ fieldName ];
         const string & valType = FieldTypes[    fieldName ];
         const string & valUnit = FieldUnits[    fieldName ];

         if ( valType == "Int" )
         {
            sourceOut << "      m_" << fieldName << " = "; 
            if ( defVal.empty() || defVal == "FALSE" || defVal == "false" ) { sourceOut << 0; }
            else if (              defVal == "TRUE"  || defVal == "true"  ) { sourceOut << 1; }
            else                                                            { sourceOut << defVal; }
            sourceOut << "; " << (valUnit.empty() ? string( "" ) : " // [" + valUnit + "]" ) << "\n";
         }
         else if ( valType == "Double" )
         {
            sourceOut << "      m_" << fieldName << " = " << (defVal.empty() ? string( "0.0" ) : defVal );
            sourceOut << "; " << (valUnit.empty() ? string( "" ) : " // [" + valUnit + "]" ) << "\n";
         }
         else if ( valType == "String" && !defVal.empty() )
         {
            sourceOut << "      m_" << fieldName << " = \"" << defVal << "\"";
            sourceOut << "; " << (valUnit.empty() ? string( "" ) : " // [" + valUnit + "]" ) << "\n";
         }
      }
      sourceOut << "   }\n\n";

      // Create constructor for table class
      sourceOut << "   Table" << tableName << "::Table" << tableName << "( database::Table * tbl )\n";
      sourceOut << "   {\n";
      sourceOut << "      for ( auto rec : *tbl )\n";
      sourceOut << "      {\n";
      sourceOut << "         RecNumber id = addRecord( Record() );\n";
      for ( auto fieldName : TableFields[ tableName ] )
      {
         sourceOut << "         set" << fieldName << "( id, database::get" << fieldName << "( rec ) );\n";
      }
      sourceOut << "      }\n";
      sourceOut << "   }\n\n";

      sourceOut << "   Table" << tableName << "::~Table" << tableName << "() {}\n\n";

      // Create methods for get/set record
      sourceOut << "   bmapi::RecNumber Table" << tableName << "::addRecord( const Table" << tableName << "::Record & rec )\n";
      sourceOut << "   {\n";
      sourceOut << "      m_recordList.push_back( rec );\n";
      sourceOut << "      return m_recordList.size() - 1;\n";
      sourceOut << "   }\n\n";
 
      sourceOut << "   Table" << tableName << "::Record Table" << tableName << "::getRecord( RecNumber id ) ";
      sourceOut << "{ return id < m_recordList.size() ? m_recordList[id] : Record(); }\n\n";

      for ( auto fieldName : TableFields[ tableName ] )
      {
         const string & fieldType = FieldTypes[ fieldName ];
         string fieldGetType;
         string fieldSetType;
         if(       fieldType == "Int"    ) { fieldSetType = fieldGetType = "int"; }
         else if ( fieldType == "Double" ) { fieldSetType = fieldGetType = "double"; }
         else if ( fieldType == "String" ) { fieldGetType = "std::string"; fieldSetType = "const " + fieldGetType + " & "; }
         else { throw runtime_error( string( "Unknown column Type " ) + fieldType ); }

         sourceOut << "   " << fieldGetType << " Table" << tableName << "::get" << fieldName << "( RecNumber id ) const\n";
         sourceOut << "   {\n";
         sourceOut << "      if ( id >= m_recordList.size() ) { throw std::runtime_error( \"No such record with id\" + std::to_string( id ) ); }\n"; 
#ifdef API_UNIT_TEST
         if ( fieldType == "String" )
         {
            sourceOut << "      std::cerr << \"   ph.m_" << tableName << "->set" << fieldName << "( id, \\\"\" << m_recordList[id].m_" << 
                         fieldName << " << \"\\\" );\" << std::endl;\n";
         }
         else if ( fieldType == "Double" )
         {
            sourceOut << "      std::cerr << \"   ph.m_" << tableName << "->set" << fieldName << 
                         "( id, \" << std::setprecision( 15 ) << m_recordList[id].m_" << fieldName << " << \" );\" << std::endl;\n";
         }
         else
         {
            sourceOut << "      std::cerr << \"   ph.m_" << tableName << "->set" << fieldName << "( id, \" << m_recordList[id].m_" << 
                         fieldName << " << \" );\" << std::endl;\n";
         }
#endif
         sourceOut << "      return m_recordList[id].m_" << fieldName << ";\n";
         sourceOut << "   }\n\n";
         sourceOut << "   void Table" << tableName << "::set" << fieldName << "( RecNumber id, " << fieldSetType << " val )\n";
         sourceOut << "   {\n";
         sourceOut << "      if ( id >= m_recordList.size() ) { throw std::runtime_error( \"No such record with id\" + std::to_string( id ) ); }\n"; 
         sourceOut << "      m_recordList[id].m_" << fieldName << " = val;\n";
         sourceOut << "   }\n\n";
      }
   }
   
   // Generate declaration of ProjectHandle as incorporated tables
   headerOut << "   class ProjectIoAPI\n";
   headerOut << "   {\n";
   headerOut << "   public:\n";
   for ( auto tableName : TableList )
   {
      headerOut << "      Table" << tableName << string( std::max( (3*alignWidth - (int)tableName.length()), 1 ), ' ' ) << " * m_" << tableName << ";\n";
   }
   headerOut << R"(

      ProjectIoAPI();                                    // default constructor
      ProjectIoAPI( const std::string & projFileName );  // constructor from database
      ~ProjectIoAPI();                                   // destructor

      bool saveToProjectFile( const std::string & projFileName ); // save to file

   protected:
      void initialize( database::ProjectFileHandler & ph ); // initialize all table pointers
   };)";

   sourceOut << R"(
   // Default constructor
   ProjectIoAPI::ProjectIoAPI()
   {
      database::ProjectFileHandler projFileHandler;
      initialize( projFileHandler ); 
   }

   // Constructor which loads project file
   ProjectIoAPI::ProjectIoAPI( const std::string & projFileName )
   {
      database::ProjectFileHandler ph( projFileName );
      initialize( ph );
   }

   // Save project to project file
   bool ProjectIoAPI::saveToProjectFile( const std::string & projFileName )
   {
      database::ProjectFileHandler   ph;
      database::Table              * tbl;
      )";

   for ( auto tableName : TableList )
   {
      sourceOut << "\n      tbl = ph.getTable( \"" << tableName << "\" );\n";
#ifdef API_UNIT_TEST
      sourceOut << "      for ( RecNumber id = 0; id < m_" << tableName << "->size(); id++ )\n";
      sourceOut << "      {\n";
      sourceOut << "         std::cerr << \"   id = ph.m_" << tableName << "->addRecord( bmapi::Table" << tableName << "::Record() );\" << std::endl;\n";
      sourceOut << "         database::Record * trec = tbl->createRecord( true );\n";
      for ( auto fieldName : TableFields[ tableName ] )
      {
         sourceOut << "         database::set" << fieldName << "( trec, m_" << tableName << "->get" << fieldName << "( id ) );\n";
      }
      sourceOut << "      }\n\n";
#else
      sourceOut << "      for ( auto & rec : *m_" << tableName << " )\n";
      sourceOut << "      {\n";
      sourceOut << "         database::Record * trec = tbl->createRecord( true );\n";
      for ( auto fieldName : TableFields[ tableName ] )
      {
         sourceOut << "         database::set" << fieldName << "( trec, rec.m_" << fieldName << " );\n";
      }
      sourceOut << "      }\n\n";
#endif
   }

   sourceOut << R"(
      return ph.saveToFile( projFileName );
   }

   void ProjectIoAPI::initialize( database::ProjectFileHandler & ph ))";
   sourceOut << "\n   {\n";
   for ( auto tableName : TableList )
   {
      sourceOut << "      m_" << tableName << " = new Table" << tableName << "( ph.getTable( \"" << tableName << "\" ) );\n";
   }
   sourceOut << "   }\n\n";      

   // Generate destructor 
   sourceOut << "   ProjectIoAPI::~ProjectIoAPI()\n";
   sourceOut << "   {\n";
   for ( auto tableName : TableList )
   {
      sourceOut << "      if ( m_" << tableName << " != nullptr ) { delete m_" << tableName << "; m_" << tableName << " = nullptr; }\n";
   }
   sourceOut << "   }\n";

   headerOut << "\n}; // namespace bmapi\n";
   sourceOut << "\n}; // namespace bmapi\n";
}



void GenerateDataSchema( const string & schemaName, const string & schemaDir, bool verbose )
{
   // construct output file names

   // convert file names to low case
   string lowCaseSchemaName = schemaName;
   transform( lowCaseSchemaName.begin(), lowCaseSchemaName.end(), lowCaseSchemaName.begin(), ::tolower );

   string schemaSourceFile = schemaDir + lowCaseSchemaName + ".cpp";
   string schemaHeaderFile = schemaDir + lowCaseSchemaName + ".h";
   string funcsSourceFile  = schemaDir + lowCaseSchemaName + "funcs.cpp";
   string funcsHeaderFile  = schemaDir + lowCaseSchemaName + "funcs.h";

   // open all the output files required
   ofstream schemaSourceOut( schemaSourceFile.c_str(), ios::out );
   if ( schemaSourceOut.fail() ) { throw runtime_error( "Error occurred during opening output file " + schemaSourceFile ); }

   ofstream schemaHeaderOut( schemaHeaderFile.c_str(), ios::out );
   if ( schemaHeaderOut.fail() ) { throw runtime_error( "Error occurred during opening output file " + schemaHeaderFile ); }

   ofstream funcsSourceOut( funcsSourceFile.c_str(), ios::out );
   if ( funcsSourceOut.fail() ) { throw runtime_error( "Error occurred during opening output file " + funcsSourceFile );   }

   ofstream funcsHeaderOut( funcsHeaderFile.c_str(), ios::out );
   if ( funcsHeaderOut.fail() ) { throw runtime_error( "Error occurred during opening output file " + funcsHeaderFile );   }

   // Construct the output files
   schemaHeaderOut << "#include \"database.h\"\n\n";
   schemaHeaderOut << "namespace database\n";
   schemaHeaderOut << "{\n";
   schemaHeaderOut << "   DataSchema * create" << schemaName << " ();\n";
   schemaHeaderOut << "   void upgradeAllTables" << "In" << schemaName << "( Database * db );\n\n";

   schemaSourceOut << "#include \"dataschema.h\"\n";
   schemaSourceOut << "#include \"" << schemaHeaderFile << "\"\n\n";
   schemaSourceOut << "using namespace datatype;\n";
   schemaSourceOut << "using namespace database;\n\n";

   funcsHeaderOut << "#include \"database.h\"\n\n";
   funcsHeaderOut << "namespace database\n";
   funcsHeaderOut << "{\n";

   funcsSourceOut << "#include \"database.h\"\n";
   funcsSourceOut << "#include \"" << funcsHeaderFile << "\"\n\n";
   funcsSourceOut << "using namespace datatype;\n";
   funcsSourceOut << "using namespace database;\n\n";

   sort( FieldList.begin(), FieldList.end(), [] ( const string & lhs, const string & rhs ) -> bool
   { return FieldTables[ lhs ].size() > FieldTables[ rhs ].size(); } );
#if DEBUG
   int numErrors = OrderFields();
   cerr << "Ordering succeeded with " << numErrors << " errors\n";

   for ( auto & fieldName : FieldList )
   {
      cerr << "FieldIndices[" << fieldName << "] = " << FieldIndices[ fieldName ] << endl;
   }
#endif
   
   // Create the table definitions
   for ( auto & tableName : TableList )
   {
      string tableDescription = TableDescriptions[ tableName ];
      schemaHeaderOut << "   void create" << tableName << "Definition( DataSchema * dataSchema );\n";

      // generate declarations for upgrade table methods. Developer must implement them when table is changed
      auto tblVer = TableVersions[ tableName ];
      if ( tblVer > versionNumStart )
      {
         for ( auto i = versionNumStart; i < tblVer; ++i )
         {
            schemaHeaderOut << "   void upgrade" << tableName << "TableVer" << i << "to" << i + 1 << "( Database * db, Table * tbl );\n";
         }
      }

      schemaSourceOut << "void database::create" << tableName << "Definition( DataSchema * dataSchema )\n";
      schemaSourceOut << "{" << endl;
      schemaSourceOut << "   TableDefinition * tableDefinition = dataSchema->addTableDefinition( \""
                      << tableName << "\", \"" << tableName << "\", " << tblVer << " );\n\n";

      StringList & fieldList = TableFields[ tableName ];

      // retain the outputOrder
      StringToIntMap outputOrder;
      int order = 0;
      for ( auto & fieldName : fieldList ) { outputOrder[ fieldName ] = order++; }

      sort( fieldList.begin(), fieldList.end(), [] ( const string & lhs, const string & rhs ) -> bool
      { return FieldIndices[ lhs ] < FieldIndices[ rhs ]; } );
#if DEBUG
      cerr << " --- " << tableName << endl;
#endif
      // fill up the holes in the list with the unpositioned fields
      int index = 0;

      for ( auto fieldIter = fieldList.begin(); fieldIter != fieldList.end(); ++fieldIter )
      {
         string fieldName = *fieldIter;

         while ( FieldIndices[ fieldName ] != 999 && index < FieldIndices[ fieldName ] && fieldList.end() - fieldIter > 1 )
         {
            string lastName = fieldList.back();
#if DEBUG
            cerr << " ++++++ " << lastName << " (" << FieldIndices[ lastName ] << ")" << endl;
#endif
            std::copy_backward( fieldIter, fieldList.end() - 1, fieldList.end() );
            *fieldIter = lastName;
            index++;
            ++fieldIter;
         }
#if DEBUG
         cerr << " ------ " << fieldName << " (" << FieldIndices[ fieldName ] << ")" << endl;
#endif
         index++;
      }

      // Add the field definitions
      for ( auto fieldName : fieldList )
      {
         string fieldDescription = FieldDescriptions[ fieldName ];
         string fieldUnit = "\"" + FieldUnits[ fieldName ] + "\"";
         string fieldType = FieldTypes[ fieldName ];
         string fieldDefault = FieldDefaults[ fieldName ];
         string fieldOrder = to_string( outputOrder[ fieldName ] );

         fieldName.insert( 0, "\"" );
         fieldName += "\"";

         if ( fieldType != "String" || fieldDefault.size() == 0 || fieldDefault[ 0 ] != '"' )
         {
            fieldDefault.insert( 0, "\"" );
            fieldDefault += "\"";
         }

         schemaSourceOut << "   tableDefinition->addFieldDefinition( " <<
            setw( alignWidth ) << setfill( ' ' ) << fieldName << ", " <<
            setw( alignWidth ) << setfill( ' ' ) << fieldType << ", " <<
            setw( alignWidth ) << setfill( ' ' ) << fieldUnit << ", " <<
            setw( alignWidth ) << setfill( ' ' ) << fieldDefault << ", " <<
            setw( alignWidth ) << setfill( ' ' ) << fieldOrder << " );\n";
      }
      schemaSourceOut << "}" << endl << endl;
   }

   schemaSourceOut << "DataSchema * database::create" << schemaName << "()\n";
   schemaSourceOut << "{" << endl;
   schemaSourceOut << "   DataSchema * dataSchema = new DataSchema;\n\n";

   for ( auto & tableName : TableList )
   {
      schemaSourceOut << "   create" << tableName << "Definition( dataSchema );\n";
   }

   schemaSourceOut << "\n   return dataSchema;\n";
   schemaSourceOut << "}\n\n";

   // Create the table upgrades function 
   schemaSourceOut << "void database::upgradeAllTables"<< "In" << schemaName << "( Database * db )\n";
   schemaSourceOut << "{\n";
   schemaSourceOut << "   Table * tbl = nullptr;\n" << endl;
   for ( auto & tableName : TableList )
   {
      auto tblVer = TableVersions[ tableName ];
      if ( tblVer > versionNumStart )
      {
         schemaSourceOut << "   tbl = db->getTable( \"" << tableName << "\" );\n";
 
         for ( auto i = versionNumStart; i < tblVer; ++i )
         {
            schemaSourceOut << "   database::upgrade" << tableName << "TableVer" << i << "to" << i+1 << "( db, tbl );\n";
         }
      }
   }
   schemaSourceOut << "}\n";

   for ( auto & fieldName : FieldList )
   {
      string cleanFieldName = cleanNameForSpecialSymbols( fieldName );

      if ( FieldTables[ fieldName ].size() == 0 )
      {
         if ( verbose ) { cerr << "Warning: " << fieldName << " is not being used\n"; }
         continue;
      }

      int    indexValue = GetIndex( FieldTables[ fieldName ][ 0 ], fieldName );
      string fieldType = FieldTypes[ fieldName ];

      fieldType[ 0 ] = tolower( fieldType[ 0 ] );

      if ( fieldType == "string" ) { fieldType.insert( 0, "std::" ); }

      funcsSourceOut << "// " << fieldName << " (" << FieldTypes[ fieldName ] << ") is used in ";
      for ( int tableIndex = 0; tableIndex < FieldTables[ fieldName ].size(); tableIndex++ )
      {
         if ( tableIndex != 0 ) { funcsSourceOut << ", "; }
         if ( tableIndex % 5 == 0 ) { funcsSourceOut << endl << "//     "; }

         string & tableName = FieldTables[ fieldName ][ tableIndex ];
         funcsSourceOut << tableName << "( " << GetIndex( tableName, fieldName ) << " )";
      }

      funcsSourceOut << endl;

      funcsHeaderOut << "   extern const " << fieldType << " & get" << cleanFieldName << "( Table * tbl, int i );\n";
      funcsHeaderOut << "   extern const " << fieldType << " & get" << cleanFieldName << "( Record * record );\n\n";

      funcsHeaderOut << "   extern void set" << cleanFieldName << "( Table * tbl, int i, const " << fieldType << " & my"
                     << cleanFieldName << " );\n";
      funcsHeaderOut << "   extern void set" << cleanFieldName << "( Record * record, const "    << fieldType << " & my"
                     << cleanFieldName << " );\n\n";

      funcsSourceOut << "const " << fieldType << " & database::get" << cleanFieldName << "( database::Table * tbl, int i )\n";
      funcsSourceOut << "{\n";
      funcsSourceOut << "   Record * record = tbl->getRecord( i );\n";
      funcsSourceOut << "   return get" << cleanFieldName << "( record );\n";
      funcsSourceOut << "}\n\n";

      funcsSourceOut << "const " << fieldType << " & database::get" << cleanFieldName << "( database::Record * record )\n";
      funcsSourceOut << "{\n";
      funcsSourceOut << "   static int cachedIndex = " << indexValue << ";\n";
      funcsSourceOut << "   return record->getValue<" << fieldType << ">( \"" << fieldName << "\", & cachedIndex );\n";
      funcsSourceOut << "}\n";

      funcsSourceOut << "void database::set" << cleanFieldName << "( database::Table * tbl, int i, const " << fieldType << " & my"
                     <<  cleanFieldName << ")\n";
      funcsSourceOut << "{\n";
      funcsSourceOut << "   Record * record = tbl->getRecord( i );\n";
      funcsSourceOut << "   set" << cleanFieldName << "( record, my" << cleanFieldName << " );\n";
      funcsSourceOut << "}\n\n";

      funcsSourceOut << "void database::set" << cleanFieldName << "( database::Record * record, const " << fieldType << " & my"
                     << cleanFieldName << " )\n";
      funcsSourceOut << "{\n";
      funcsSourceOut << "   static int cachedIndex = " << indexValue << ";\n";
      funcsSourceOut << "   record->setValue( \"" << fieldName << "\", my" << cleanFieldName << ", & cachedIndex );\n";
      funcsSourceOut << "}\n\n";
   }

   schemaHeaderOut << "}\n";
   funcsHeaderOut  << "}\n";
}

/// The parspec main routine
int main( int argc, char **argv )
{
   bool verbose = false;
   bool bmapi  = false;
   string specFile;
   string schemaName;
   string schemaDir;

   try
   {
      for ( int i = 1; i < argc; i++ )
      {  
         size_t argvSz = std::max( size_t{ 2 }, strlen( argv[ i ] ) );
         if ( strncmp( argv[ i ], "-directory", argvSz ) == 0 )
         {
            schemaDir = argv[ ++i ];
#ifdef _WIN32
            schemaDir += "\\";
#else
            schemaDir += "/";
#endif
      }
         else if ( strncmp( argv[ i ], "-input",   argvSz ) == 0 ) { specFile   = argv[ ++i ]; }
         else if ( strncmp( argv[ i ], "-schema",  argvSz ) == 0 ) { schemaName = argv[ ++i ]; }
         else if ( strncmp( argv[ i ], "-verbose", argvSz ) == 0 ) { verbose    = true; }
         else if ( strncmp( argv[ i ], "-bmapi",   argvSz ) == 0 ) { bmapi     = true; }
         else
         {
            throw runtime_error( string( "Usage: " ) + argv[ 0 ] + " -input filename -schema schemaname [-directory outputdirectory] [-verbose]\n" );
         }
      }

      if ( specFile.length() == 0 || schemaName.length() == 0 )
      {
         throw runtime_error( string( "Usage: " ) + argv[ 0 ] + " -input filename -schema schemaname [-directory outputdirectory]" );
      }

      ParseSpecFile( specFile );

      if ( bmapi ) { GenerateCMBAPI(     schemaName, schemaDir, verbose ); }
      else         { GenerateDataSchema( schemaName, schemaDir, verbose ); }
   }
   catch( runtime_error & ex )
   {
      cerr << ex.what() << endl;
      return -1;
   }

   return 0;
}
