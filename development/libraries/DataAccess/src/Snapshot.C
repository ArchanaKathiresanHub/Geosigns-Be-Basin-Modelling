#include <assert.h>
#include <iostream>
#include <sstream>
#include <iomanip>

using namespace std;


#include "database.h"
#include "cauldronschemafuncs.h"

using namespace database;

#include "Interface/Snapshot.h"


using namespace DataAccess;
using namespace Interface;

//using MINOR;
//using MAJOR;

Snapshot::Snapshot (ProjectHandle * projectHandle, Record * record) : DAObject (projectHandle, record)
{
   m_time = database::getTime (m_record);
   if (database::getIsMinorSnapshot (m_record) == 1)
      m_type = MINOR;
   else
      m_type = MAJOR;

   m_appendFile = false;
}

Snapshot::Snapshot (ProjectHandle * projectHandle, double time) : DAObject (projectHandle, 0)
{
   m_time = time;
   m_type = MINOR;
}

Snapshot::~Snapshot (void)
{
}

double Snapshot::getTime (void) const
{
   return m_time;
}

const string & Snapshot::getFileName (bool setIfNecessary) const
{
   const string & fileName = database::getSnapshotFileName (m_record);
   if (fileName != "") return fileName;

   // generate file name
   ostringstream buf;

   buf.setf (ios::fixed);
   buf.precision (6);

   if (getType () == MAJOR)
   {
      buf << "Time_" << getTime () << ".h5";
   }
   else
   {
      buf << "Time_" << getTime () << ".h5";
   }
   
   if (setIfNecessary)
   {
      database::setSnapshotFileName (m_record, buf.str ());
   }

   return database::getSnapshotFileName (m_record);
}

const std::string& Snapshot::getKind () const {
   return database::getTypeOfSnapshot ( m_record );
}

int Snapshot::getType (void) const
{
   return m_type;
}

bool Snapshot::getUseInResQ (void) const {
	
	return database::getUseInResQ ( m_record ) == 1;

}

void Snapshot::setAppendFile ( const bool append ) {
   m_appendFile = append;
}

bool Snapshot::getAppendFile () const {
   return m_appendFile;
}


void Snapshot::asString (string & str) const
{
   ostringstream buf;

   buf << "Snapshot: ";
   buf << setprecision (10) << getTime ();
   buf << endl;

   str = buf.str ();
}

const string & Snapshot::asString (void) const
{
   static string str;
   ostringstream buf;

   buf << setprecision (10) << getTime ();

   str = buf.str ();

   return str;
}

void Snapshot::printOn (ostream & ostr) const
{
   string str;
   asString (str);
   ostr << str;
}


bool DataAccess::Interface::operator== (const Snapshot & ss1, const Snapshot & ss2)
{
   return ss1.getTime () == ss2.getTime ();
}

bool DataAccess::Interface::operator< (const Snapshot & ss1, const Snapshot & ss2)
{
   return ss1.getTime () > ss2.getTime ();
}

bool DataAccess::Interface::operator> (const Snapshot & ss1, const Snapshot & ss2)
{
   return ss1.getTime () < ss2.getTime ();
}

bool DataAccess::Interface::operator<= (const Snapshot & ss1, const Snapshot & ss2)
{
   return !(ss1 > ss2);
}
   
bool DataAccess::Interface::operator>= (const Snapshot & ss1, const Snapshot & ss2)
{
   return !(ss1 < ss2);
}

bool DataAccess::Interface::SnapshotLessThan::operator ()( const Snapshot* ss1,
                                                           const Snapshot* ss2 ) const {
   return ss1->getTime () < ss2->getTime ();
}
