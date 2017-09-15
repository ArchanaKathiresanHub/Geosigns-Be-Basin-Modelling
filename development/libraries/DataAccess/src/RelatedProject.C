#include <assert.h>
#include <iostream>
#include <sstream>
using namespace std;

#include "database.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "Interface/RelatedProject.h"
#include "Interface/ProjectHandle.h"
#include "FilePath.h"

using namespace DataAccess;
using namespace Interface;


RelatedProject::RelatedProject (ProjectHandle * projectHandle, Record * record) : DAObject (projectHandle, record)
{
   const std::string& fileName = database::getFilename ( m_record );

   m_projectName = fileName;
   size_t pointLocation = fileName.find ( "*Point " );

   if ( pointLocation == std::string::npos ) {
      // load 1d project.

      database::DataSchema * projSchema = database::createCauldronSchema ();
    
      database::Database * relatedProjectDatabase;

      ibs::FilePath filePathName( projectHandle->getProjectPath () );
      filePathName << fileName;
      relatedProjectDatabase = Database::CreateFromFile ( filePathName.path(), *projSchema );
    
      if ( relatedProjectDatabase != nullptr )
      {
         database::Table * tbl = relatedProjectDatabase->getTable ( "WellLocIoTbl" );

         assert ( tbl != nullptr );

         database::Record *relatedProjectRecord = tbl->getRecord ( 0 );
   
         assert (relatedProjectRecord != nullptr);
   
         m_eastPosition  = database::getXCoord ( relatedProjectRecord );
         m_northPosition = database::getYCoord ( relatedProjectRecord );
         

         tbl = relatedProjectDatabase->getTable ( "StratIoTbl" );
         assert ( tbl != nullptr );

         database::Record * stratRecord = tbl->getRecord((int) tbl->size() - 1);
         m_bottomOfSediment = database::getDepth (stratRecord);

         m_topOfSediment = -9999;

         double overburdenThickness = 0;
         for (int i = 0; i < (int)tbl->size () && m_topOfSediment == -9999; ++i)
         {
            double depth;

            stratRecord = tbl->getRecord(i);
            if ((depth = database::getDepth (stratRecord)) == -9999)
            {
               double thickness = database::getThickness (stratRecord);
               if (thickness != -9999)
               {
            overburdenThickness += thickness;
               }
            }
            else
            {
               m_topOfSediment = depth - overburdenThickness;
            }
         }

         delete relatedProjectDatabase;
      }

      if ( projSchema != nullptr ) {
         delete projSchema;
      }

   } else {
      // Convert string to location.

      const char* startLocation = fileName.c_str ();
      char* endLocation;

      // Find first space character after the '*point'.
      while ( *startLocation != ' ' ) {
         ++startLocation;
      }

      // This is the east location value.
      m_eastPosition = strtod ( startLocation, &endLocation );

      if ( endLocation == startLocation ) {
         // error.
         m_eastPosition = DefaultUndefinedScalarValue;
      } else {

         // Set the start location to the character past the end of the previous number.
         startLocation = endLocation;

         // get rid of all spaces.
         while ( *startLocation == ' ' ) {
            ++startLocation;
         }

         // find the next space character (reading over the 'east,' sub-string).
         while ( *startLocation != ' ' ) {
            ++startLocation;
         }

         // This is the north location value.
         m_northPosition = strtod ( startLocation, &endLocation );

         if ( endLocation == startLocation ) {
            // error.
            m_northPosition = DefaultUndefinedScalarValue;
         }

      }
   }
}


RelatedProject::~RelatedProject ()
{
}

double RelatedProject::getNorth () const {
   return m_northPosition;
}

double RelatedProject::getEast () const {
   return m_eastPosition;
}

// returns top of sediment of related project
double RelatedProject::getTopOfSediment () const
{
   return m_topOfSediment;
}

// returns bottom of sediment of related project
double RelatedProject::getBottomOfSediment () const
{
   return m_bottomOfSediment;;
}

const std::string& RelatedProject::getName () const {
   return m_projectName;
}


void RelatedProject::printOn (ostream & ostr) const {
   string str;
   asString (str);
   ostr << str;
}

void RelatedProject::asString (string & str) const {

   ostringstream buf;

   buf << "RelatedProject:";
   buf << " name = " << getName ();
   buf << ", position = (" << getNorth () << ", " << getEast () << ")";
   buf << endl;

   str = buf.str ();

}

