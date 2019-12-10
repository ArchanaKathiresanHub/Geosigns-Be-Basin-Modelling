//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

// Genex6
#include "GenexBaseSourceRock.h"
#include "ChemicalModel.h"

// DataAccess library
#include "Surface.h"
#include "Snapshot.h"

// utilities library
#include "ConstantsNumerical.h"
#include "LogHandler.h"

#include "SnapshotInterval.h"

#include "Simulator.h"
#include "SourceRockNode.h"
#include "Input.h"
#include "ConstantsGenex.h"

// std library
#include <vector>

using namespace std;

using namespace DataAccess;
using Interface::Snapshot;
using Interface::Formation;
using Interface::Surface;

namespace Genex6
{

const double GenexBaseSourceRock::conversionCoeffs [8] =
   { -2.60832073307101E-05, 0.236463623513642,
     -0.0319467563289369, 0.00185738251210839,
     2.36948559032296E-05, -6.62225531134738E-06,
     2.38411451425613E-07, -2.692340754443E-09 };

GenexBaseSourceRock::GenexBaseSourceRock ()
{
   m_theSimulator = nullptr;
   m_formation = nullptr;
   m_theChemicalModel  = nullptr;
   m_theChemicalModel1 = nullptr;
   m_theChemicalModel2 = nullptr;

   m_applySRMixing = false;
   m_isSulphur = false;
}

GenexBaseSourceRock::~GenexBaseSourceRock(void)
{
   clear();
}

void GenexBaseSourceRock::getHIBounds( double &HILower, double &HIUpper ) {
   HILower = 28.47;
   HIUpper = 773.6;
}

double GenexBaseSourceRock::convertHCtoHI( double aHC ) {

   double HILower;
   double HIUpper;
   double HIValue;
   double HCValue;

   getHIBounds( HILower, HIUpper );

   // Simple bisection method to compute HI from H/C
   while ( fabs( HILower - HIUpper ) > 0.0001 ) {
      HIValue = 0.5 * (HILower + HIUpper);
      HCValue = convertHItoHC( HIValue );

      if ( HCValue > aHC ) {
         HIUpper = HIValue;
      }
      else {
         HILower = HIValue;
      }
   }

   return HIValue;
}

double GenexBaseSourceRock::convertHItoHC( double aHI ) {

   if ( aHI != Interface::DefaultUndefinedMapValue ) {
      int i;
      double hc = conversionCoeffs[7];
      const double sqrtHI = sqrt( aHI );

      for ( i = 6; i >= 0; --i ) {
         hc = hc * sqrtHI + conversionCoeffs[i];
      }

      return floor( hc * 1000 + 0.5 ) / 1000;
   }
   else {
      return  Interface::DefaultUndefinedMapValue;
   }
}

void GenexBaseSourceRock::clear()
{
   clearSnapshotIntervals();
   clearSimulator();
   clearSourceRockNodeAdsorptionHistory ();

   if ( m_theChemicalModel2 != nullptr ) {
      delete m_theChemicalModel2;
      m_theChemicalModel2 = nullptr;
   }
   m_theChemicalModel = nullptr;
}

void GenexBaseSourceRock::clearSimulator()
{
   if (m_theSimulator) {
      // set ChemicalModel1 to be deleted inside Simulator destructor
      m_theSimulator->setChemicalModel( m_theChemicalModel1 );

      delete m_theSimulator;
      m_theSimulator = nullptr;

      m_theChemicalModel1 = nullptr;
      m_theChemicalModel  = 0;
   }
}

void GenexBaseSourceRock::clearSnapshotIntervals()
{
   std::vector<SnapshotInterval*>::iterator itEnd = m_theIntervals.end();
   for(std::vector<SnapshotInterval*>::iterator it = m_theIntervals.begin(); it != itEnd; ++ it) {
     delete (*it);
   }
   m_theIntervals.clear();
}

double GenexBaseSourceRock::getDepositionTime() const
{
   const Surface * topSurface = m_formation->getTopSurface ();
   const Snapshot * depoSnapshot = topSurface->getSnapshot ();
   return depoSnapshot->getTime ();
}

bool GenexBaseSourceRock::compute()
{
   bool status = true;

   LogHandler( LogHandler::INFO_SEVERITY ) << "Ready to compute SourceRock at : "<< m_formation->getName();

   if(status) status = initialize();

   if(status) status = preprocess();

   if(status) status = addHistoryToNodes ();

   if(status) status = process();

   return status;
}


void GenexBaseSourceRock::saveSourceRockNodeAdsorptionHistory () {

   SourceRockAdsorptionHistoryList::iterator histIter;

   for ( histIter = m_sourceRockNodeAdsorptionHistory.begin (); histIter != m_sourceRockNodeAdsorptionHistory.end (); ++histIter ) {
      (*histIter)->save ();
   }

}

void GenexBaseSourceRock::clearSourceRockNodeAdsorptionHistory () {

   SourceRockAdsorptionHistoryList::iterator histIter;

   for ( histIter = m_sourceRockNodeAdsorptionHistory.begin (); histIter != m_sourceRockNodeAdsorptionHistory.end (); ++histIter ) {
      delete *histIter;
   }

   m_sourceRockNodeAdsorptionHistory.clear ();
}


double GenexBaseSourceRock::getMaximumTimeStepSize ( const double depositionTime ) const {
   return m_theSimulator->GetMaximumTimeStepSize ( depositionTime );
}


void GenexBaseSourceRock::computeSnapshotIntervals (const DataAccess::Interface::SnapshotList & snapshots)
{
   m_depositionTime = getDepositionTime ();

   DataAccess::Interface::SnapshotList::const_reverse_iterator snapshotIter;

   const DataAccess::Interface::Snapshot * start;
   const DataAccess::Interface::Snapshot * end;

   if (snapshots.size () >= 1) {
      end = 0;
      for(snapshotIter = snapshots.rbegin (); snapshotIter != snapshots.rend () - 1; ++ snapshotIter) {

         start = (*snapshotIter);
         end = 0;

         if ((m_depositionTime > start->getTime ()) ||
             (fabs (m_depositionTime - start->getTime ()) < Genex6::Constants::Zero)) {
            start = *snapshotIter;
            end = *(snapshotIter + 1);
            SnapshotInterval *theInterval = new SnapshotInterval (start, end);

            m_theIntervals.push_back (theInterval);
         }
      }
   } else {
      //throw
   }
}

// The following functions are not the the trunk version.
// Can they be removed?
const Simulator& GenexBaseSourceRock::getSimulator () const {
   return *m_theSimulator;
}

}//namespace Genex6
