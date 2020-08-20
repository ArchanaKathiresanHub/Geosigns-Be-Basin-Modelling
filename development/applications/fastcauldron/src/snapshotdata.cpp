//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#include "snapshotdata.h"
#include "petscts.h"

#include "ConstantsFastcauldron.h"
#include "fileio.h"
#include "cauldronschema.h"
#include "cauldronschemafuncs.h"
#include "utils.h"
#include "FilePath.h"

//------------------------------------------------------------//


SnapshotEntry::SnapshotEntry  ( const double       snapshotAge,
                                const bool         initialIsMinor,
                                const std::string& initialType,
                                const std::string& newFileName ) {

  m_time = snapshotAge;

  if ( initialType == "" ) {
     m_type = "System Generated";
  } else {
     m_type = initialType;
  }

  m_isMinor = initialIsMinor;

  /// Only generate a file name if it is a major snapshot and the newFileName is a null string ("").
  if ( ! m_isMinor && newFileName == "" ) {
    m_fileName = generatePropertyFileName ( "", m_isMinor, snapshotAge );
  } else {
    // Should we check here to see if the file name has an extension or not?
    m_fileName = newFileName;
  }

}


bool SnapshotEntry::isMinor () const {
  return m_isMinor;
}

bool SnapshotEntry::isMajor () const  {
  return ! isMinor ();
}

double SnapshotEntry::time () const  {
  return m_time;
}

const std::string& SnapshotEntry::type () const  {
  return m_type;
}

const std::string& SnapshotEntry::fileName () const  {
  return m_fileName;
}


bool SnapshotEntryLess::operator ()( const SnapshotEntry* left,
                                     const SnapshotEntry* right ) const {

  return left->time () < right->time ();

}

//------------------------------------------------------------//


void SnapshotInterval::clear () {
  snapshots.clear ();
}

void SnapshotInterval::addSnapshot ( SnapshotEntry* SS ) {
  snapshots.push_back ( SS );
}

int SnapshotInterval::numberOfSnapshots () const {
  return snapshots.size ();
}

const SnapshotEntry& SnapshotInterval::operator ()( const int position ) const {
  return *snapshots [ position ];
}

//------------------------------------------------------------//

SnapshotData::SnapshotData () {
  m_minorSnapshotsPrescribed = false;
}

//------------------------------------------------------------//

SnapshotData::~SnapshotData () {
  clearSnapshotEntrySet ( majorSnapshotTimes );
  clearSnapshotEntrySet ( minorSnapshotTimes );
}

//------------------------------------------------------------//

void SnapshotData::setMinorSnapshotsPrescribed ( const bool newValue ) {
  m_minorSnapshotsPrescribed = newValue;
}

//------------------------------------------------------------//

bool SnapshotData::projectPrescribesMinorSnapshots () const {
  return m_minorSnapshotsPrescribed;
}

//------------------------------------------------------------//

void SnapshotData::clearSnapshotEntrySet ( SnapshotEntrySet& dataSet ) {

  SnapshotEntrySet::reverse_iterator it;

  for ( it = dataSet.rbegin (); it != dataSet.rend (); ++it ) {
    delete (*it);
  }

  dataSet.clear ();
}

//------------------------------------------------------------//

void SnapshotData::setMaximumNumberOfMinorSnapshots () {

  SnapshotEntrySetIterator majorSnapshots;
  SnapshotEntrySetIterator minorSnapshots;

  double snapshotIntervalEndTime;
  int    minorSnapshotCount;

  maximumNumberOfMinorSnapshotsValue = 0;

  majorSnapshots = majorSnapshotsBegin ();
  ++majorSnapshots;

  minorSnapshots = minorSnapshotsBegin ();

  ///
  /// Find the maximum number of minor snapshots that lie between two major snapshots.
  ///
  while ( majorSnapshots != majorSnapshotsEnd ()) {
    snapshotIntervalEndTime = (*majorSnapshots)->time ();
    minorSnapshotCount = 0;

    ///
    /// Count the number of minor snapshots that have a time that is greater than the current major snapshot.
    ///
    while ( minorSnapshots != minorSnapshotsEnd () && (*minorSnapshots)->time () > snapshotIntervalEndTime ) {
      ++minorSnapshotCount;
      ++minorSnapshots;
    }

    maximumNumberOfMinorSnapshotsValue = Integer_Max ( maximumNumberOfMinorSnapshotsValue, minorSnapshotCount );
    ++majorSnapshots;
  }

}

//------------------------------------------------------------//

void SnapshotData::initialiseMinorSnapshotVector ( const bool usingDarcy ) {


  PetscBool foundMinorSnapshotCount;
  SnapshotEntrySetIterator it;
  double snapshotIntervalStartTime;
  double snapshotIntervalEndTime;
  double minorSnapshotInterval;
  int    I;

  if ( not usingDarcy and projectPrescribesMinorSnapshots ()) {
    /// Only need to find the maximum number of minor snapshots that lie bwtween two consecutive major snapshots.
    setMaximumNumberOfMinorSnapshots ();
  } else {

    ///
    /// Need to create the minor snapshot times. These times may not be used since
    /// the time steps may not land exactly on one of the time computed here.
    /// The minor snapshot time that will be used will be the next time step after
    /// the time computed here.
    ///

    clearSnapshotEntrySet ( minorSnapshotTimes );

    PetscOptionsGetInt( PETSC_NULL, "-numberminorss", &maximumNumberOfMinorSnapshotsValue, &foundMinorSnapshotCount );

#if 0
    if ( ! foundMinorSnapshotCount ) {
      maximumNumberOfMinorSnapshotsValue = DefaultNumberOfMinorSnapshots;
    } else if ( maximumNumberOfMinorSnapshotsValue < 0 ) {
      maximumNumberOfMinorSnapshotsValue = 0;
    }
#endif

    if ( not foundMinorSnapshotCount and not usingDarcy ) {
      maximumNumberOfMinorSnapshotsValue = DefaultNumberOfMinorSnapshots;
    } else {

       if ( foundMinorSnapshotCount ) {

          if ( maximumNumberOfMinorSnapshotsValue < 0 ) {
             maximumNumberOfMinorSnapshotsValue = 0;
          }

       } else if ( usingDarcy ) {
          maximumNumberOfMinorSnapshotsValue = 0;
       }

    }

    it = majorSnapshotsBegin ();
    snapshotIntervalStartTime = (*it)->time ();
    ++it;

    while ( it != majorSnapshotsEnd ()) {
      snapshotIntervalEndTime = (*it)->time ();

      /// Initialise the minor snapshot times to be regularly spaced in the major snapshot time interval.
      minorSnapshotInterval = ( snapshotIntervalStartTime - snapshotIntervalEndTime ) / double ( maximumNumberOfMinorSnapshotsValue + 1 );

      for ( I = 1; I <= maximumNumberOfMinorSnapshotsValue; I++ ) {
        minorSnapshotTimes.insert ( new SnapshotEntry ( snapshotIntervalStartTime - double ( I ) * minorSnapshotInterval,
                                                        true,
                                                        SystemGeneratedSnapshotStr,
                                                        generatePropertyFileName ( "", true, snapshotIntervalStartTime - double ( I ) * minorSnapshotInterval )));
      }

      snapshotIntervalStartTime = snapshotIntervalEndTime;
      ++it;
    }

  }

}

//------------------------------------------------------------//

void SnapshotData::clearMinorSnapshots () {
  clearSnapshotEntrySet ( minorSnapshotTimes );
  maximumNumberOfMinorSnapshotsValue = 0;
}

//------------------------------------------------------------//

bool SnapshotData::isMinorSnapshot ( const double                  age,
                                     const SnapshotEntrySetIterator& snapshots ) const {

  if ( snapshots == minorSnapshotTimes.rend ()) {
    return false;
  } else if ( projectPrescribesMinorSnapshots ()) {
    return age == (*snapshots)->time ();
  } else {
    return age <= (*snapshots)->time ();
  }

}

//------------------------------------------------------------//

bool SnapshotData::isAlmostSnapshot ( const double                    age,
                                      const SnapshotEntrySetIterator& snapshots,
                                      const double                    absoluteError ) const {

  return snapshots != minorSnapshotTimes.rend () && fabs ((*snapshots)->time () - age ) < absoluteError;
}

//------------------------------------------------------------//

bool SnapshotData::isAlmostSnapshot ( const double                    currentTime,
                                      const double                    currentTimeStep,
                                      const SnapshotEntrySetIterator& snapshots,
                                      const double                    timeStepFraction ) const {

  return snapshots != minorSnapshotTimes.rend () && (( currentTime - timeStepFraction * currentTimeStep ) - (*snapshots)->time ()) < 0.0;
}

//------------------------------------------------------------//

bool SnapshotData::validMinorSnapshots ( const SnapshotEntrySetIterator& snapshots ) const {
  return minorSnapshotTimes.size () > 0 && snapshots != minorSnapshotTimes.rend ();
}

//------------------------------------------------------------//

SnapshotEntrySetIterator SnapshotData::minorSnapshotsBegin () const {
  return minorSnapshotTimes.rbegin ();
}

//------------------------------------------------------------//

SnapshotEntrySetIterator SnapshotData::minorSnapshotsEnd   () const {
  return minorSnapshotTimes.rend ();
}

//------------------------------------------------------------//

SnapshotEntrySetIterator SnapshotData::majorSnapshotsBegin () const {
  return majorSnapshotTimes.rbegin ();
}

//------------------------------------------------------------//

SnapshotEntrySetIterator SnapshotData::majorSnapshotsEnd   () const {
  return majorSnapshotTimes.rend ();
}

//------------------------------------------------------------//

void SnapshotData::advanceMinorSnapshotIterator ( const double                    currentTime,
                                                        SnapshotEntrySetIterator& minorSnapshotIterator ) {

  if ( projectPrescribesMinorSnapshots () && validMinorSnapshots ( minorSnapshotIterator )) {
    ++minorSnapshotIterator;
  } else {

    while ( minorSnapshotIterator != minorSnapshotsEnd () && (*minorSnapshotIterator)->time () > currentTime ) {
      ++minorSnapshotIterator;
    }

  }

}

//------------------------------------------------------------//

void SnapshotData::addSnapshotEntry ( const double       time,
                                      const bool         snapshotIsMinor,
                                      const std::string& typeOfSnapshot,
                                      const std::string& dataFileName ) {

  SnapshotEntry* newItem;
  std::string generatedFileName;

  if ( duplicateFound ( time )) {
    return;
  }

  if ( snapshotIsMinor ) {
    newItem = new SnapshotEntry ( time, snapshotIsMinor, typeOfSnapshot, dataFileName );
    minorSnapshotTimes.insert ( newItem );
  } else {

    if ( dataFileName == "" ) {
      generatedFileName = generatePropertyFileName ( "", false, time );
      newItem = new SnapshotEntry ( time, snapshotIsMinor, typeOfSnapshot, generatedFileName );
    } else {
      newItem = new SnapshotEntry ( time, snapshotIsMinor, typeOfSnapshot, dataFileName );
    }

    majorSnapshotTimes.insert ( newItem );
  }

}

//------------------------------------------------------------//

bool SnapshotData::duplicateFound ( const double newAge ) const {

  const double epsilon = 1.0e-7;

  SnapshotEntrySetIterator it;

  for ( it = majorSnapshotsBegin (); it != majorSnapshotsEnd (); ++it ) {

    if ( fabs ( newAge - (*it)->time ()) < epsilon ) {
      return true;
    }

  }

  return false;
}

//------------------------------------------------------------//

int SnapshotData::numberOfMajorSnapshots () const {
  return majorSnapshotTimes.size ();
}

//------------------------------------------------------------//

int SnapshotData::numberOfMinorSnapshots () const {
  return minorSnapshotTimes.size ();

}

//------------------------------------------------------------//

int SnapshotData::maximumNumberOfMinorSnapshots () const {
  return maximumNumberOfMinorSnapshotsValue;
}

//------------------------------------------------------------//

void SnapshotData::getMinorSnapshotsInInterval ( const double            startTime,
                                                 const double            endTime,
                                                       SnapshotInterval& interval ) {

  SnapshotEntrySetIterator minorSnapshots;

  for ( minorSnapshots = minorSnapshotsBegin (); minorSnapshots != minorSnapshotsEnd (); ++minorSnapshots ) {

    if ( startTime > (*minorSnapshots)->time () && (*minorSnapshots)->time () > endTime ) {
      interval.addSnapshot ( *minorSnapshots );
    }

  }

}

//------------------------------------------------------------//

void SnapshotData::getSucceedingSnapshotInterval ( const SnapshotEntrySetIterator& fromMajorSnapshot,
                                                   const bool                      includeMinorSnapshots,
                                                         SnapshotInterval&         interval ) {

  SnapshotEntrySetIterator nextMajorSnapshot = fromMajorSnapshot;

  interval.clear ();

  nextMajorSnapshot++;


  if ( nextMajorSnapshot != majorSnapshotsEnd ()) {

    ///
    /// Add first major snapshot
    ///
    interval.addSnapshot ( *fromMajorSnapshot );

    if ( includeMinorSnapshots ) {
      getMinorSnapshotsInInterval ((*fromMajorSnapshot)->time (), (*nextMajorSnapshot)->time (), interval );
    }

    interval.addSnapshot ( *nextMajorSnapshot );
  }

}

//------------------------------------------------------------//

void SnapshotData::getPrecedingSnapshotInterval ( const SnapshotEntrySetIterator& uptoMajorSnapshot,
                                                  const bool                      includeMinorSnapshots,
                                                        SnapshotInterval&         interval ) {

  SnapshotEntrySetIterator previousMajorSnapshot = uptoMajorSnapshot;

  interval.clear ();



  if ( uptoMajorSnapshot != majorSnapshotsBegin ()) {
//    if ( uptoMajorSnapshot != majorSnapshotTimes.rbegin ()) {
    --previousMajorSnapshot;

    ///
    /// Add first major snapshot
    ///
    interval.addSnapshot ( *previousMajorSnapshot );

    if ( includeMinorSnapshots ) {
      getMinorSnapshotsInInterval ((*previousMajorSnapshot)->time (), (*uptoMajorSnapshot)->time (), interval );
    }

    interval.addSnapshot ( *uptoMajorSnapshot );
  }

}

//------------------------------------------------------------//

void SnapshotData::getCurrentSnapshotInterval ( const SnapshotEntrySetIterator& majorSnapshot,
                                                      SnapshotInterval&         interval ) {

  interval.clear ();

  if ( majorSnapshot != majorSnapshotsBegin ()) {
    interval.addSnapshot ( *majorSnapshot );
  }

}

//------------------------------------------------------------//


void SnapshotData::setActualMinorSnapshots ( const snapshottimeContainer& savedMinorSnapshotTimes ) {

  SnapshotConstIterator ssIter;

  clearSnapshotEntrySet ( minorSnapshotTimes );

  for ( ssIter = savedMinorSnapshotTimes.rbegin (); ssIter != savedMinorSnapshotTimes.rend (); ++ssIter ) {
    minorSnapshotTimes.insert ( new SnapshotEntry ( *ssIter,
                                                    true,
                                                    SystemGeneratedSnapshotStr,
                                                    generatePropertyFileName ( "", true, *ssIter )));
  }


}

//------------------------------------------------------------//


void SnapshotData::deleteIntermediateMinorSnapshotFiles ( const snapshottimeContainer& minorSnapshotTimesToDelete,
                                                          const std::string&           outputDirectoryName ) const {

  if ( PetscGlobalRank != 0 ) {
    return;
  }

  SnapshotConstIterator ssIter;
  string                fileName;

  for ( ssIter = minorSnapshotTimesToDelete.rbegin (); ssIter != minorSnapshotTimesToDelete.rend (); ++ssIter ) {
    fileName = outputDirectoryName + generatePropertyFileName ( "", true, *ssIter );
    unlink ( fileName.c_str ());
  }

}

//------------------------------------------------------------//


void SnapshotData::deleteMinorSnapshotFiles ( const std::string& outputDirectoryName ) const {

  if ( PetscGlobalRank != 0 ) {
    return;
  }

  SnapshotEntrySetIterator ssIter;
  string                   fileName;

  for ( ssIter = minorSnapshotTimes.rbegin (); ssIter != minorSnapshotTimes.rend (); ++ssIter ) {

    if ((*ssIter)->fileName () != "" ) {
       ibs::FilePath fileName ( outputDirectoryName );
       fileName << (*ssIter)->fileName ();
       unlink ( fileName.cpath ());
    }

  }

}

//------------------------------------------------------------//


void SnapshotData::deleteMajorSnapshotFiles ( const std::string& outputDirectoryName ) const {

  if ( PetscGlobalRank != 0 ) {
    return;
  }

  SnapshotEntrySetIterator ssIter;
  string                   fileName;

  for ( ssIter = majorSnapshotTimes.rbegin (); ssIter != majorSnapshotTimes.rend (); ++ssIter ) {

    if ((*ssIter)->fileName () != "" ) {
       ibs::FilePath fileName ( outputDirectoryName );
       fileName << (*ssIter)->fileName ();
       unlink ( fileName.cpath ());
    }

  }

}

//------------------------------------------------------------//
