#include <assert.h>
#include <iostream>
#include <sstream>
using namespace std;

#include "Interface/FaultCollection.h"
#include "Interface/Faulting.h"
#include "Interface/Snapshot.h"
#include "Interface/Formation.h"

using namespace DataAccess;
using namespace Interface;

//------------------------------------------------------------//

FaultCollection::FaultCollection (ProjectHandle * projectHandle, const string & mapName):DAObject (projectHandle, 0),
m_name (mapName)
{
}

//------------------------------------------------------------//


FaultCollection::~FaultCollection ()
{
   NameFaultMapping::iterator faults;

   for (faults = m_faults.begin (); faults != m_faults.end (); ++faults)
   {
      delete ((*faults).second);
   }

   m_faults.clear ();
}

FaultList * FaultCollection::getFaults (void) const
{
   FaultList * faultList = new FaultList;
   NameFaultMapping::const_iterator faults;
   for (faults = m_faults.begin (); faults != m_faults.end (); ++faults)
   {
      faultList->push_back ((*faults).second);
   }
   return faultList;
}

//------------------------------------------------------------//

const string & FaultCollection::getName (void) const
{
   return m_name;
}

//------------------------------------------------------------//

void FaultCollection::addFormation (const Formation * formation)
{
   m_formations.push_back (formation);
}

bool FaultCollection::appliesToFormation (const Formation * formation) const
{
   FormationList::const_iterator formationIter;

   for (formationIter = m_formations.begin (); formationIter != m_formations.end (); ++formationIter)
   {
      if (formation == * formationIter)
         return true;
   }
   return false;
}

void FaultCollection::addFault (const std::string & faultName, const PointSequence & faultLine)
{
   if (m_faults.find (faultName) == m_faults.end ())
   {
      Fault *newFault = new Fault (faultName, faultLine);

      m_faults[faultName] = newFault;
   }
   else
   {
      cerr << "****************    ERROR FaultCollection::addFault:  Fault name "
            << faultName << "  exists already.   ****************" << endl;
   }
}

//------------------------------------------------------------//

void FaultCollection::addEvent (const std::string & faultName, const Snapshot * snapshot, const string & status)
{
   NameFaultMapping::iterator selectedFault = m_faults.find (faultName);

   if (selectedFault != m_faults.end ())
   {
      Fault * fault = (*selectedFault).second;
      fault->addEvent (snapshot, status);
   }
   else
   {
      ///
      /// ERROR: A fault with the name faultName MUST exist.
      ///
      cerr << "****************    ERROR FaultCollection::addEvent:  Fault name "
            << faultName << "  does NOT exist already.   ****************" << endl;
   }
}

         void addOverpressureEvent (const std::string & faultName,
                                    const Snapshot * snapshot,
                                    const string & faultLithology,
                                    const bool usedInOverpressure );

void FaultCollection::addOverpressureEvent (const std::string & faultName,
                                            const Snapshot * snapshot,
                                            const string & faultLithology,
                                            const bool usedInOverpressure )
{
   NameFaultMapping::iterator selectedFault = m_faults.find (faultName);

   if (selectedFault != m_faults.end ())
   {
      Fault * fault = (*selectedFault).second;
      fault->addOverpressureEvent (snapshot, faultLithology, usedInOverpressure );
   }
   else
   {
      ///
      /// ERROR: A fault with the name faultName MUST exist.
      ///
      cerr << "****************    ERROR FaultCollection::addEvent:  Fault name "
            << faultName << "  does NOT exist already.   ****************" << endl;
   }
}

Fault * FaultCollection::findFault (const std::string & faultName) const
{
   Fault * fault = 0;
   NameFaultMapping::const_iterator selectedFault = m_faults.find (faultName);

   if (selectedFault != m_faults.end ())
   {
      fault = (*selectedFault).second;
   }
   return fault;
}

unsigned int FaultCollection::size (void) const
{
   return m_faults.size ();
}

//------------------------------------------------------------//

#if 0
void FaultCollection::setBasinFaults (AppCtx * basinModel) const const
{

   NameFaultMapping::const_iterator faults;
   Basin_Modelling::Layer_Iterator layerIter;

   GridDescription grid;


   grid.deltaX = basinModel->cauldronGridDescription.deltaI;
   grid.deltaY = basinModel->cauldronGridDescription.deltaJ;

   grid.originX = basinModel->cauldronGridDescription.originI;
   grid.originY = basinModel->cauldronGridDescription.originJ;

   layerIter.Initialise_Iterator (basinModel->layers, topFormationName, bottomFormationName);

   for (faults = collection.begin (); faults != collection.end (); ++faults)
   {
      (*faults).second->setBasinFault (grid, &basinModel->lithomanager, layerIter);
   }

}
#endif

//------------------------------------------------------------//

ostream & DataAccess::Interface::operator<< (ostream & o, const FaultCollection & collection)
{

   FaultCollection::NameFaultMapping::const_iterator faults;

   o << " map name " << collection.m_name << "  " << collection.m_faults.size () << endl;

   for (faults = collection.m_faults.begin (); faults != collection.m_faults.end (); ++faults)
   {
      o << *(*faults).second;
   }

   return o;
}

//------------------------------------------------------------//
