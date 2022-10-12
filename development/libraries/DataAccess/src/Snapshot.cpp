//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

//std
#include <sstream>
#include <iomanip>
using namespace std;

//TableIO
#include "database.h"
#include "cauldronschemafuncs.h"
using namespace database;

//DataAccess
#include "Snapshot.h"
using namespace DataAccess::Interface;

Snapshot::Snapshot (ProjectHandle& projectHandle, Record * record) : DAObject (projectHandle, record)
{
   m_time = database::getTime (m_record);
   if (database::getIsMinorSnapshot (m_record) == 1)
      m_type = MINOR;
   else
      m_type = MAJOR;

   m_appendFile = false;
}

Snapshot::Snapshot (ProjectHandle& projectHandle, const double time) : DAObject (projectHandle, nullptr)
{
   m_time = time;
   m_type = MINOR;
   m_appendFile = false;
}

double Snapshot::getTime () const
{
   return m_time;
}

const string & Snapshot::getFileName (const bool setIfNecessary) const
{
   const string & fileName = database::getSnapshotFileName (m_record);
   if (!fileName.empty()) return fileName;

   // generate file name
   std::ostringstream buf;

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

int Snapshot::getType () const
{
   return m_type;
}

bool Snapshot::getUseInResQ () const {
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
   std::ostringstream buf;

   buf << "Snapshot: ";
   buf << setprecision (10) << getTime ();
   buf << endl;

   str = buf.str ();
}

const string & Snapshot::asString () const
{
   static string str;
   std::ostringstream buf;

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
