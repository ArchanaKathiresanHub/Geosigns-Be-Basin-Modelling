#include <assert.h>
#ifdef sgi
   #ifdef _STANDARD_C_PLUS_PLUS
      #include<iostream>
      #include <sstream>
      #include <iomanip>
      using namespace std;
      #define USESTANDARD
   #else // !_STANDARD_C_PLUS_PLUS
      #include<iostream.h>
      #include<strstream.h>
      #include <iomanip.h>
   #endif // _STANDARD_C_PLUS_PLUS
#else // !sgi
   #include <iostream>
   #include <sstream>
   #include <iomanip>
   #define USESTANDARD
   using namespace std;
#endif // sgi

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
#ifdef USESTANDARD
   ostringstream buf;
#else
   strstream buf;
#endif
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

void Snapshot::setAppendFile ( const bool append ) {
   m_appendFile = append;
}

bool Snapshot::getAppendFile () const {
   return m_appendFile;
}


void Snapshot::asString (string & str) const
{
#ifdef USESTANDARD
   ostringstream buf;
#else
   strstream buf;
#endif

   buf << "Snapshot: ";
   buf << setprecision (10) << getTime ();
   buf << endl;

   str = buf.str ();
#ifndef USESTANDARD
   buf.rdbuf ()->freeze (0);
#endif
}

const string & Snapshot::asString (void) const
{
   static string str;
#ifdef USESTANDARD
   ostringstream buf;
#else
   strstream buf;
#endif

   buf << setprecision (10) << getTime ();

   str = buf.str ();

#ifndef USESTANDARD
   buf.rdbuf ()->freeze (0);
#endif

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
