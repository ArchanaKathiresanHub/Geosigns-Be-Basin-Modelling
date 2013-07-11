#include <algorithm> // contains std::sort
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <iomanip>

#include <assert.h>
#include <vector>
#include <map>
#include <string>
#include <ctype.h>
using namespace std;

#include <string.h>

#include "datautils.h"

using namespace std;

typedef vector < string > StringList;
typedef vector < int >IntList;

typedef map < string, string, less < string > >StringToStringMap;
typedef map < string, int, less < string > >StringToIntMap;
typedef map < string, bool, less < string > >StringToBoolMap;
typedef map < string, StringList, less < string > >StringToStringListMap;
typedef map < string, IntList, less < string > >StringToIntListMap;

StringToStringListMap FieldTables; ///< Contains the list of table names for each field
StringList FieldList;           ///< Contains the list of field names
StringToStringMap FieldDescriptions; ///< Contains a description for each field
StringToStringMap FieldUnits;   ///< Contains the unit of each field
StringToStringMap FieldTypes;   ///< Contains the type of each field
StringToStringMap FieldDefaults; ///< Contains the default value of each field
StringToIntMap FieldIndices;    ///< Contains the index a field was assigned to.
StringToBoolMap FieldErrors;    ///< Specifies whether an error field is to be created for a field

StringList TableList;           ///< Contains the list of tables
StringToStringMap TableDescriptions; ///< description for each table
StringToStringListMap TableFields; ///< Fields of each table

int lineno = 0;

/// Reads the next non-comment line from a file
bool loadSpecLine (ifstream & infile, string & line)
{
   while (true)
   {
#ifdef sun
      char buf[1024];

      infile.getline (buf, 1024);
      line = buf;
#else
      getline (infile, line, '\n');
#endif
      lineno++;
      if (infile.eof ())
      {
         return false;
      }
      else
      {
         const string separators = " \t";
         size_t firstNonSpace = line.find_first_not_of (separators, 0);

         if (firstNonSpace == string::npos) // empty line
            continue;
         else if (line[firstNonSpace] == ';') // comment possibly with leading spaces
            continue;
         else
            return true;
      }
   }
}

/// Extracts a word from a line beyond linePos
size_t loadWordFromLine (const string & line, size_t linePos, string & word)
{
   string separators = " \t";

   size_t wordStartPos = line.find_first_not_of (separators, linePos);

   if (wordStartPos == string::npos)
   {
      word = "";
      return string::npos;
   }

   size_t wordEndPos = line.find_last_not_of (separators);

   size_t wordLength;

   if (wordEndPos == string::npos)
      wordLength = string::npos;
   else
      wordLength = wordEndPos + 1 - wordStartPos;

   word = line.substr (wordStartPos, wordLength);
   return wordEndPos + 1;
}

/// if a line contains the given keyword, return the associated value
bool getKeywordValue (const string & line, const string & keyword, string & value)
{
   string separators = " \t";
   size_t keywordStartPos = line.find_first_not_of (separators, 0);

   if (keywordStartPos == string::npos)
      return false;

   if (line.find (keyword, keywordStartPos) != keywordStartPos)
      return false;

   size_t valueStartPos = line.find (':', keywordStartPos + keyword.length ());

   if (valueStartPos == string::npos)
      return false;

   loadWordFromLine (line, valueStartPos + 1, value);

   return true;
}

/// Used in the sorting of fields based on the number of tables they feature in
bool FieldLess (const string & lhs, const string & rhs)
{
   return FieldTables[lhs].size () > FieldTables[rhs].size ();
}

/// Used in the sorting of fields based on the index of the field in the tables
bool FieldIndexLess (const string & lhs, const string & rhs)
{
   return FieldIndices[lhs] < FieldIndices[rhs];
}

/// Get the index of a field in a table
int GetIndex (string & tableName, string & fieldName)
{
   StringList & fieldList = TableFields[tableName];

   int index = -1;

   for (StringList::iterator fieldIter = fieldList.begin (); fieldIter != fieldList.end (); ++fieldIter)
   {
      index++;
      string loopFieldName = *fieldIter;

      if (loopFieldName == fieldName)
         return index;
   }
   return -1;
}

/// Check if a field has the same index in each table it features in
bool CheckIndex (string & fieldName, string & tableNameUsed, int indexUsed)
{
   bool result = true;

   StringList & tableList = FieldTables[fieldName];

   for (StringList::iterator tableIter = tableList.begin (); tableIter != tableList.end (); ++tableIter)
   {
      string & tableName = *tableIter;
      if (GetIndex (tableName, fieldName) != indexUsed)
      {
         result = false;
      }
   }
   return result;
}

/// Check if the specified fieldIndex is the correct index for the field specified by fieldName
/// in the table specified by tableName.
bool IsValidIndex (string & tableName, string & fieldName, int fieldIndex)
{
   bool result = true;
   StringList fieldList = TableFields[tableName];

   for (StringList::iterator fieldIter = fieldList.begin (); fieldIter != fieldList.end (); ++fieldIter)
   {
      string tableFieldName = *fieldIter;

      if (tableFieldName == fieldName)
         continue;
      int tableFieldIndex = FieldIndices[tableFieldName];

      if (tableFieldIndex == fieldIndex)
      {
         result = false;
         break;
      }
   }

   return result;
}


/// Try to assign an index to each field such that a field has the same index in each table.
/// Return the number of fields for which the attempt was unsuccessful.
int OrderFields (void)
{
   int numErrors = 0;

   int fieldListIndex;
   for (fieldListIndex = 0; fieldListIndex >= 0 && fieldListIndex < FieldList.size (); fieldListIndex++)
   {
      string & fieldName = FieldList[fieldListIndex];
      FieldIndices[fieldName] = -1;
   }

   for (fieldListIndex = 0; fieldListIndex >= 0 && fieldListIndex < FieldList.size (); fieldListIndex++)
   {
      string & fieldName = FieldList[fieldListIndex];
      if (FieldTables[fieldName].size () == 0)
         continue;

#if DEBUG
      cerr << "Assigning field " << fieldName << " (" << fieldListIndex << ")" << endl;
#endif

      for (int fieldIndicesIndex = 0; true; fieldIndicesIndex++)
      {
         bool validIndex = true;
         bool outOfRange = false;

         for (StringList::iterator tableIter = FieldTables[fieldName].begin ();
              tableIter != FieldTables[fieldName].end (); ++tableIter)
         {
            string & tableName = *tableIter;

            if (fieldIndicesIndex >= TableFields[tableName].size ())
            {
               outOfRange = true;
               break;
            }
            if (!IsValidIndex (tableName, fieldName, fieldIndicesIndex))
            {
               validIndex = false;
               break;
            }
         }

         // no valid index could be found in combination with the previously positioned fields.
         // put this field temporarily at the back of each table
         if (outOfRange)
         {
            FieldIndices[fieldName] = 999;
            numErrors++;
#if DEBUG
            cerr << "Assigning " << 999 << " to " << fieldName << endl;
#endif
            break;
         }

         if (validIndex)
         {
#if DEBUG
            cerr << "Assigning " << fieldIndicesIndex << " to " << fieldName << endl;
#endif
            FieldIndices[fieldName] = fieldIndicesIndex;
            break;
         }
      }
   }
   return numErrors;
}

/// The parspec main routine
int main (int argc, char **argv)
{

   bool verbose = false;
   string specFile;
   string schemaName;
   string schemaDir;

   int i;
   for (i = 1; i < argc; i++)
   {
      if (strncmp (argv[i], "-directory", std::max ((size_t) 2, (size_t) strlen (argv[i]))) == 0)
      {
	 schemaDir = argv[++i];
	 schemaDir += "/";
      }
      else if (strncmp (argv[i], "-input", std::max ((size_t) 2, (size_t) strlen (argv[i]))) == 0)
      {
	 specFile = argv[++i];
      }
      else if (strncmp (argv[i], "-schema", std::max ((size_t) 2, (size_t) strlen (argv[i]))) == 0)
      {
	 schemaName = argv[++i];
      }
      else if (strncmp (argv[i], "-verbose", std::max ((size_t) 2, (size_t) strlen (argv[i]))) == 0)
      {
	 verbose = true;
      }
      else
      {
	 cerr << "Usage: " << argv[0]
	      << " -input filename -schema schemaname"
	         "[-directory outputdirectory] [-verbose]" << endl;
	 return -1;
      }
   }

   if (specFile.length () == 0 || schemaName.length () == 0)
   {
      cerr << "Usage: " << argv[0]
	 << " -input filename -schema schemaname [-directory outputdirectory]" << endl;
      return -1;
   }

   ifstream infile;

   infile.open (specFile.c_str (), ios::in);

   if (infile.fail ())
   {
      cerr << "Error occurred during opening input file " << argv[1] << endl;
      return -1;
   }

   // open all the output files required
   ofstream schemaSourceOut;
   ofstream schemaHeaderOut;

   ofstream funcsSourceOut;
   ofstream funcsHeaderOut;

   string schemaSourceFile = schemaName;
   string schemaHeaderFile = schemaName;

   for (i = schemaName.length () - 1; i >= 0; i--)
   {
      schemaSourceFile[i] = tolower (schemaSourceFile[i]);
      schemaHeaderFile[i] = tolower (schemaHeaderFile[i]);
   }

   schemaSourceFile.insert (0, schemaDir);
   schemaHeaderFile.insert (0, schemaDir);

   string funcsSourceFile = schemaSourceFile + "funcs";
   string funcsHeaderFile = schemaHeaderFile + "funcs";

   schemaSourceFile += ".C";
   schemaHeaderFile += ".h";
   funcsSourceFile += ".C";
   funcsHeaderFile += ".h";

   schemaSourceOut.open (schemaSourceFile.c_str (), ios::out);

   if (schemaSourceOut.fail ())
   {
      cerr << "Error occurred during opening output file " << schemaSourceFile << endl;
      return -1;
   }

   schemaHeaderOut.open (schemaHeaderFile.c_str (), ios::out);

   if (schemaHeaderOut.fail ())
   {
      cerr << "Error occurred during opening output file " << schemaHeaderFile << endl;
      return -1;
   }

   funcsSourceOut.open (funcsSourceFile.c_str (), ios::out);

   if (funcsSourceOut.fail ())
   {
      cerr << "Error occurred during opening output file " << funcsSourceFile << endl;
      return -1;
   }

   funcsHeaderOut.open (funcsHeaderFile.c_str (), ios::out);

   if (funcsHeaderOut.fail ())
   {
      cerr << "Error occurred during opening output file " << funcsHeaderFile << endl;
      return -1;
   }

   string line;
   string value;
   string fieldName;
   string description;
   string unit;
   string type;
   string defaultValue;
   bool error;

   bool endProps;
   bool startNewProp;

   while (loadSpecLine (infile, line))
   {
      if (line.find ("START-properties") != string::npos)
      {
         fieldName = "";
         description = "";
         unit = "";
         type = "";
         defaultValue = "";
         error = false;

         break;
      }
   }

   // Read in all the fields possible.
   while (loadSpecLine (infile, line))
   {
      endProps = (line.find ("END-properties") != string::npos);
      startNewProp = getKeywordValue (line, "Identification", value);

      if (endProps || startNewProp)
      {
         if (fieldName != "")
         {
            FieldList.push_back (fieldName);
            FieldDescriptions[fieldName] = description;
            FieldUnits[fieldName] = unit;
            FieldTypes[fieldName] = type;
            FieldDefaults[fieldName] = defaultValue;
            if (error)
            {
               FieldErrors[fieldName] = error;

               fieldName.insert (0, "Err");

               FieldList.push_back (fieldName);
               FieldDescriptions[fieldName] = description;
               FieldUnits[fieldName] = unit;
               FieldTypes[fieldName] = type;
               FieldDefaults[fieldName] = "";
            }
         }

         fieldName = "";
         description = "";
         unit = "";
         type = "";
         defaultValue = "";
         error = false;
      }

      if (endProps)
         break;
      if (startNewProp)
         fieldName = value;

      if (getKeywordValue (line, "Description", value))
         description = value;

      if (getKeywordValue (line, "Unit of measurement", value))
      {
         string separators = " \t";
         size_t fieldUnitStartPos = value.find_first_of (separators, 0);

         if (fieldUnitStartPos != string::npos)
            value.erase (fieldUnitStartPos, 100000);

         unit = value;
      }

      if (getKeywordValue (line, "Type", value))
      {
         if (value.find ("boolean") != string::npos)
            value = "Int";
         else if (value.find ("smallInt") != string::npos)
            value = "Int";
         else if (value.find ("smallint") != string::npos)
            value = "Int";
         else if (value.find ("integer") != string::npos)
            value = "Int";
         else if (value.find ("float") != string::npos)
            value = "Double";
         else if (value.find ("double") != string::npos)
            value = "Double";
         else if (value.find ("string") != string::npos)
            value = "String";
         else
            cerr << "Unknown Type " << value << " at line " << lineno << endl;

         type = value;

      }

      if (getKeywordValue (line, "Default", value))
         defaultValue = value;

      if (getKeywordValue (line, "Error Field", value))
      {
         error = (value == "Yes" ? true : false);
      }
   }

   while (loadSpecLine (infile, line))
   {
      if (line.find ("START-property-tables") != string::npos)
         break;
   }

   // read in all the tables
   string tableName;
   bool tableIO = false;

   while (loadSpecLine (infile, line))
   {
      if (line.find ("END-property-tables") != string::npos)
         break;

      if (getKeywordValue (line, "Identification", value))
      {
         tableIO = true;
         tableName = value;
         TableList.push_back (value);
      }

      if (tableIO && getKeywordValue (line, "Description", value))
         TableDescriptions[tableName] = value;

      if (tableIO && getKeywordValue (line, "Column", value))
      {
         TableFields[tableName].push_back (value);
         FieldTables[value].push_back (tableName);
      }

      if (tableIO && getKeywordValue (line, "Property I/O", value))
      {
         if (value != "Yes" && value != "yes")
         {
            TableList.pop_back ();
            for (StringList::iterator fieldIter = TableFields[tableName].begin ();
                 fieldIter != TableFields[tableName].end (); ++fieldIter)
            {
               string fieldName = *fieldIter;

               FieldTables[fieldName].pop_back ();
            }
            TableFields[tableName].clear ();
            tableIO = false;
         }
      }
   }

   // Add the error fields
   StringList::iterator tblIter;
   for (tblIter = TableList.begin (); tblIter != TableList.end (); ++tblIter)
   {
      const string & tableName = *tblIter;

      int tableSize = TableFields[tableName].size ();

      for (i = 0; i < tableSize; i++)
      {
         string fieldName = TableFields[tableName][i];

         if (FieldErrors[fieldName])
         {
            fieldName.insert (0, "Err");
            TableFields[tableName].push_back (fieldName);
            FieldTables[fieldName].push_back (tableName);
         }
      }
   }

   // Construct the output files
   schemaSourceOut << "#include \"dataschema.h\"" << endl;
   schemaSourceOut << "#include \"" << schemaHeaderFile << "\"" << endl;
   schemaSourceOut << endl;

   funcsSourceOut << "#include \"database.h\"" << endl;
   funcsSourceOut << "#include \"" << funcsHeaderFile << "\"" << endl;
   funcsSourceOut << endl;

   schemaHeaderOut << "#include \"database.h\"" << endl;
   schemaHeaderOut << endl;

   funcsHeaderOut << "#include \"database.h\"" << endl;
   funcsHeaderOut << endl;

   schemaHeaderOut << "namespace database" << endl;
   schemaHeaderOut << "{" << endl;

   funcsHeaderOut << "namespace database" << endl;
   funcsHeaderOut << "{" << endl;

   schemaSourceOut << "using namespace datatype;" << endl;
   schemaSourceOut << "using namespace database;" << endl;
   schemaSourceOut << endl;

   funcsSourceOut << "using namespace datatype;" << endl;
   funcsSourceOut << "using namespace database;" << endl;
   funcsSourceOut << endl;

   schemaHeaderOut << "   TABLEIO_DLL_EXPORT DataSchema * create" << schemaName << " (void);" << endl << endl;

   sort (FieldList.begin (), FieldList.end (), FieldLess);

#define EQUALIZEFIELDPOSITIONS
#ifdef EQUALIZEFIELDPOSITIONS
   int numErrors = OrderFields ();

#if DEBUG
   cerr << "Ordering succeeded with " << numErrors << " errors" << endl;
#endif

   for (int fieldListIndex = 0; fieldListIndex >= 0 && fieldListIndex < FieldList.size (); fieldListIndex++)
   {
      string & fieldName = FieldList[fieldListIndex];
#if DEBUG
      cerr << "FieldIndices[" << fieldName << "] = " << FieldIndices[fieldName] << endl;
#endif
   }
#endif

   // Create the table definitions
   for (tblIter = TableList.begin (); tblIter != TableList.end (); ++tblIter)
   {
      string tableName = *tblIter;
      string tableDescription = TableDescriptions[tableName];

      schemaHeaderOut << "   TABLEIO_DLL_EXPORT void create" << tableName << "Definition (DataSchema * dataSchema);" << endl;

      schemaSourceOut << "void database::create" << tableName << "Definition (DataSchema * dataSchema)" << endl;
      schemaSourceOut << "{" << endl;
      schemaSourceOut << "   TableDefinition * tableDefinition = dataSchema->addTableDefinition (\"" << tableName
            << "\", \"" << tableName << "\");" << endl << endl;

      StringList & fieldList = TableFields[tableName];
      StringList::iterator fieldIter;

      // retain the outputOrder
      StringToIntMap outputOrder;
      int order = 0;

      for (fieldIter = fieldList.begin (); fieldIter != fieldList.end (); ++fieldIter)
      {
         string fieldName = *fieldIter;

         outputOrder[fieldName] = order++;
      }

#ifdef EQUALIZEFIELDPOSITIONS
      sort (fieldList.begin (), fieldList.end (), FieldIndexLess);
#if DEBUG
      cerr << " --- " << tableName << endl;
#endif
      // fill up the holes in the list with the unpositioned fields
      int index = 0;

      for (fieldIter = fieldList.begin (); fieldIter != fieldList.end (); ++fieldIter)
      {
         string fieldName = *fieldIter;

         while (FieldIndices[fieldName] != 999 && index < FieldIndices[fieldName])
         {
            string lastName = fieldList.back ();

#if DEBUG
            cerr << " ++++++ " << lastName << " (" << FieldIndices[lastName] << ")" << endl;
#endif
            fieldList.pop_back ();

            fieldList.insert (fieldIter, lastName);
            index++;
            ++fieldIter;
         }
#if DEBUG
         cerr << " ------ " << fieldName << " (" << FieldIndices[fieldName] << ")" << endl;
#endif
         index++;
      }
#endif

      // Add the field definitions
      for (fieldIter = fieldList.begin (); fieldIter != fieldList.end (); ++fieldIter)
      {
         string fieldName = *fieldIter;

         string fieldDescription = FieldDescriptions[fieldName];
         string fieldUnit = FieldUnits[fieldName];
         string fieldType = FieldTypes[fieldName];
         string fieldDefault = FieldDefaults[fieldName];

         string fieldOrder;
         char tmpStr[24];

         sprintf (tmpStr, "%d", outputOrder[fieldName]);
         fieldOrder = tmpStr;

         fieldName.insert (0, "\"");
         fieldName += "\"";

         fieldUnit.insert (0, "\"");
         fieldUnit += "\"";

         if (fieldType != "String" || fieldDefault.size() == 0 || fieldDefault[0] != '"')
         {
            fieldDefault.insert (0, "\"");
            fieldDefault += "\"";
         }

         int borrowed = 0;

         schemaSourceOut << "   tableDefinition->addFieldDefinition (";
         saveStringToStream (schemaSourceOut, fieldName, borrowed);
         schemaSourceOut << ", ";
         saveStringToStream (schemaSourceOut, fieldType, borrowed);
         schemaSourceOut << ", ";
         saveStringToStream (schemaSourceOut, fieldUnit, borrowed);
         schemaSourceOut << ", ";
         saveStringToStream (schemaSourceOut, fieldDefault, borrowed);
         schemaSourceOut << ", ";
         saveStringToStream (schemaSourceOut, fieldOrder, borrowed);
         schemaSourceOut << ");" << endl;
      }
      schemaSourceOut << "}" << endl << endl;
   }

   schemaSourceOut << "DataSchema * database::create" << schemaName << " (void)" << endl;
   schemaSourceOut << "{" << endl;
   schemaSourceOut << "   DataSchema * dataSchema = new DataSchema;" << endl;
   schemaSourceOut << endl;
   for (tblIter = TableList.begin (); tblIter != TableList.end (); ++tblIter)
   {
      string tableName = *tblIter;

      schemaSourceOut << "   create" << tableName << "Definition (dataSchema);" << endl;
   }

   schemaSourceOut << endl;
   schemaSourceOut << "   return dataSchema;" << endl;
   schemaSourceOut << "}" << endl;

   // Add field access convenience functions, starting with most frequently used fields.
   int numInconsistencies = 0;

   for (StringList::iterator fieldIter = FieldList.begin (); fieldIter != FieldList.end (); ++fieldIter)
   {
      string fieldName = *fieldIter;
      string cleanFieldName = fieldName;
      for (int i = 0; i < cleanFieldName.length (); ++i)
      {
	 if (cleanFieldName[i] == '+' ||
	       cleanFieldName[i] == '-' ||
	       cleanFieldName[i] == '*' ||
	       cleanFieldName[i] == '/' ||
	       cleanFieldName[i] == '=' ||
	       cleanFieldName[i] == ':' ||
	       cleanFieldName[i] == ' ')
	 {
	    cleanFieldName[i] = '_';
	 }
      }

      string tableName;

      if (FieldTables[fieldName].size () == 0)
      {
	 if (verbose)
	    cerr << "Warning: " << fieldName << " is not being used" << endl;
         continue;
      }

      tableName = FieldTables[fieldName][0];

      int indexValue = GetIndex (tableName, fieldName);
      bool indexConsistency = CheckIndex (fieldName, tableName, indexValue);

      if (!indexConsistency)
      {
	 if (verbose)
	    cerr << "Warning: " << fieldName << " has differing table positions" << endl;
         numInconsistencies++;
      }

      string fieldType = FieldTypes[fieldName];

      fieldType[0] = tolower (fieldType[0]);

      if (fieldType == "string")
      {
         fieldType.insert (0, "std::");
      }

      funcsSourceOut << "// " << fieldName << " (" << FieldTypes[fieldName] << ") is used in ";
      for (int tableIndex = 0; tableIndex < FieldTables[fieldName].size (); tableIndex++)
      {
         if (tableIndex != 0)
            funcsSourceOut << ", ";
         if (tableIndex % 5 == 0)
            funcsSourceOut << endl << "//     ";

         string & tableName = FieldTables[fieldName][tableIndex];
         funcsSourceOut << tableName << " (" << GetIndex (tableName, fieldName) << ")";
      }

      funcsSourceOut << endl;

      funcsHeaderOut << "   TABLEIO_DLL_EXPORT extern const " << fieldType << " & get" << cleanFieldName << " (Table * tbl, int i);" << endl;
      funcsHeaderOut << "   TABLEIO_DLL_EXPORT extern const " << fieldType << " & get" << cleanFieldName << " (Record * record);" << endl <<
            endl;
      funcsHeaderOut << "   TABLEIO_DLL_EXPORT extern void set" << cleanFieldName << " (Table * tbl, int i, const " << fieldType << " & my" <<
            cleanFieldName << ");" << endl;
      funcsHeaderOut << "   TABLEIO_DLL_EXPORT extern void set" << cleanFieldName << " (Record * record, const " << fieldType << " & my" <<
            cleanFieldName << ");" << endl << endl;

      funcsSourceOut << "const " << fieldType << " & database::get" << cleanFieldName << " (database::Table * tbl, int i)" << endl;
      funcsSourceOut << "{" << endl;
      funcsSourceOut << "   Record * record = tbl->getRecord (i);" << endl;
      funcsSourceOut << "   return get" << cleanFieldName << " (record);" << endl;
      funcsSourceOut << "}" << endl << endl;

      // The template handling of the current Sun compiler leaves some things to be desired.
      funcsSourceOut << "#ifdef sun" << endl;
      funcsSourceOut << "const " << fieldType << " & database::get" << cleanFieldName << " (database::Record * record)" << endl;
      funcsSourceOut << "{" << endl;

      if (indexConsistency)
      {
         funcsSourceOut << "   return record->getValue (" << indexValue << ", (" << fieldType << " *) 0);" << endl;
      }
      else
      {
         funcsSourceOut << "   static int cachedIndex = " << indexValue << ";" << endl;
         funcsSourceOut << "   return record->getValue (\"" << fieldName << "\", (" << fieldType <<
               " *) 0, & cachedIndex);" << endl;
      }
      funcsSourceOut << "}" << endl;

      funcsSourceOut << "#else" << endl;
      funcsSourceOut << "const " << fieldType << " & database::get" << cleanFieldName << " (database::Record * record)" << endl;
      funcsSourceOut << "{" << endl;
      if (indexConsistency)
      {
         funcsSourceOut << "   return record->getValue<" << fieldType << "> (" << indexValue << ");" << endl;
      }
      else
      {
         funcsSourceOut << "   static int cachedIndex = " << indexValue << ";" << endl;
         funcsSourceOut << "   return record->getValue<" << fieldType << "> (\"" << fieldName << "\", & cachedIndex);"
               << endl;
      }
      funcsSourceOut << "}" << endl;
      funcsSourceOut << "#endif" << endl << endl;

      funcsSourceOut << "void database::set" << cleanFieldName << " (database::Table * tbl, int i, const " << fieldType << " & my" <<
            cleanFieldName << ")" << endl;
      funcsSourceOut << "{" << endl;
      funcsSourceOut << "   Record * record = tbl->getRecord (i);" << endl;
      funcsSourceOut << "   set" << cleanFieldName << " (record, my" << cleanFieldName << ");" << endl;
      funcsSourceOut << "}" << endl << endl;

      funcsSourceOut << "void database::set" << cleanFieldName << " (database::Record * record, const " << fieldType << " & my" <<
            cleanFieldName << ")" << endl;
      funcsSourceOut << "{" << endl;
      if (indexConsistency)
      {
         funcsSourceOut << "   record->setValue (" << indexValue << ", my" << cleanFieldName << ");" << endl;
      }
      else
      {
         funcsSourceOut << "   static int cachedIndex = " << indexValue << ";" << endl;
         funcsSourceOut << "   record->setValue (\"" << fieldName << "\", my" << cleanFieldName << ", & cachedIndex);" <<
               endl;
      }
      funcsSourceOut << "}" << endl << endl;
   }

   schemaHeaderOut << "}" << endl;
   funcsHeaderOut << "}" << endl;

   schemaSourceOut.close ();
   schemaHeaderOut.close ();
   funcsSourceOut.close ();
   funcsHeaderOut.close ();

   if (verbose)
      cerr << "Number of inconsistent fields is " << numInconsistencies << endl;
   return 0;
}
