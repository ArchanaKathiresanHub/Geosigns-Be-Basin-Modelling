#ifndef _SNAPSHOT_DATA_H
#define _SNAPSHOT_DATA_H

#include <string>
#include <set>
#include <vector>
#include <algorithm>

#include "timefilter.h"

#include "database.h"



///
/// An entry in the snapshot table.
///
class SnapshotEntry {

public :

  SnapshotEntry ( const double       snapshotAge,
                  const bool         initialIsMinor,
                  const std::string& initialType,
                  const std::string& newFileName );

  bool isMinor () const;

  bool isMajor () const;

  double time () const;

  const std::string& type () const;

  const std::string& fileName () const;


private :

   double      m_time;
   bool        m_isMinor;
   std::string m_type;
   std::string m_fileName;

};


class SnapshotEntryLess {

public :

  bool operator ()( const SnapshotEntry* left,
                    const SnapshotEntry* right ) const;

};


typedef std::set < SnapshotEntry*, SnapshotEntryLess > SnapshotEntrySet;

typedef std::vector < SnapshotEntry* > SnapshotEntryVector;

typedef SnapshotEntrySet::const_reverse_iterator SnapshotEntrySetIterator;

/// Rename this type
typedef std::set < double, std::less < double > > snapshottimeContainer;

typedef snapshottimeContainer::reverse_iterator SnapshotIterator;

typedef snapshottimeContainer::const_reverse_iterator SnapshotConstIterator;

///
/// A snapshot interval is bounded at each side by a major snapshot, and may have 0
/// or more (upto the maximum number of minor snapshots allowed) minor snapshots.
///
class SnapshotInterval {

public :

  void clear ();

  void addSnapshot ( SnapshotEntry* SS );

  int numberOfSnapshots () const;

  const SnapshotEntry& operator ()( const int position ) const;

private :

  std::vector < SnapshotEntry* > snapshots;

};


///
/// Contains all the snapshots information of a project.
///
class SnapshotData {

public :

  SnapshotData ();

  ~SnapshotData ();


  void initialiseMinorSnapshotVector ();

  void writeSnapshotIoTbl ( database::Table * snapshotTable );

  void addSnapshotEntry ( const double       time,
                          const bool         snapshotIsMinor,
                          const std::string& typeOfSnapshot,
                          const std::string& dataFileName );

  int numberOfMajorSnapshots () const;

  int numberOfMinorSnapshots () const;

  int maximumNumberOfMinorSnapshots () const;



  SnapshotEntrySetIterator minorSnapshotsBegin () const;

  SnapshotEntrySetIterator minorSnapshotsEnd   () const;

  SnapshotEntrySetIterator majorSnapshotsBegin () const;

  SnapshotEntrySetIterator majorSnapshotsEnd   () const;


  void clearMinorSnapshots ();

  void setMinorSnapshotsPrescribed ( const bool newValue );

  /// Are the minor snapshot times prescribed by the project file.
  /// ie. Are we performing a coupled temperature calculation.
  bool projectPrescribesMinorSnapshots () const;

  /// Get snapshots from current major snapshot up to and including next 
  /// major snapshot, optionally include intervening minor snapshots.
  void getSucceedingSnapshotInterval ( const SnapshotEntrySetIterator& fromMajorSnapshot,
                                       const bool                      includeMinorSnapshots,
                                             SnapshotInterval&         interval );

  /// Get snapshots up to and including current major snapshot from previous 
  /// major snapshot, optionally include intervening minor snapshots.
  void getPrecedingSnapshotInterval ( const SnapshotEntrySetIterator& uptoMajorSnapshot,
                                      const bool                      includeMinorSnapshots,
                                            SnapshotInterval&         interval );

  /// Get interval consisting solely of the current major snapshot.
  void getCurrentSnapshotInterval ( const SnapshotEntrySetIterator& majorSnapshot,
                                          SnapshotInterval&         interval );

  /// If the minor snapshots are not prescribed my the previous calculation (from the
  /// project file), then the minor snapshot times that the properties were saved are
  /// not those that are defined in the minorSnapshotTimes array, and so must be updated.
  void setActualMinorSnapshots ( const snapshottimeContainer& savedMinorSnapshotTimes );

  bool isMinorSnapshot ( const double                    age,
                         const SnapshotEntrySetIterator& snapshots ) const;

  /// 
  bool validMinorSnapshots ( const SnapshotEntrySetIterator& snapshots ) const;

  /// Deletes the minor snapshot files specified in the snapshotTime Container.
  void deleteMinorSnapshotFiles ( const std::string& outputDirectoryName ) const;

  /// Deletes the major snapshot files specified in the snapshotTime Container.
  void deleteMajorSnapshotFiles ( const std::string& outputDirectoryName ) const;

  /// Deletes the minor snapshot files with names generated from the minorSnapshotTimes
  /// This is only used if the calculation is geometric-loop overpressure or coupled.
  void deleteIntermediateMinorSnapshotFiles ( const snapshottimeContainer& minorSnapshotTimesToDelete,
                                              const std::string&           outputDirectoryName ) const;


  /// Increment the minor snapshot iterator up to the next minor snapshot past the current time.
  void advanceMinorSnapshotIterator ( const double              currentTime,
                                      SnapshotEntrySetIterator& minorSnapshotIterator );

  /// The error is the error in the absolute value of the difference between the time and the snapshot time.
  bool isAlmostSnapshot ( const double                    time,
                          const SnapshotEntrySetIterator& snapshots,
                          const double                    absoluteError = 1.0e-4 ) const;

  /// The error here is the fraction of the time step.
  bool isAlmostSnapshot ( const double                    currentTime,
                          const double                    currentTimeStep,
                          const SnapshotEntrySetIterator& snapshots,
                          const double                    timeStepFraction = 0.25 ) const;



private :

  /// Determines if there the newAge exists in the major snapshot time set.
  bool duplicateFound ( const double newAge ) const;

  void setMaximumNumberOfMinorSnapshots ();

  /// Find all minor snapshots that have a time between the startTime 
  /// and endTime and add them to the snapshot interval
  void getMinorSnapshotsInInterval ( const double            startTime,
                                     const double            endTime,
                                           SnapshotInterval& interval );

  void clearSnapshotEntrySet ( SnapshotEntrySet& dataSet );

  /// The minor snapshot times. This set will NOT contain the major snapshot times.
  /// The time at which GenEx and the loosely coupled data will be output.
  SnapshotEntrySet       minorSnapshotTimes;
  SnapshotEntrySet       majorSnapshotTimes;

  bool m_minorSnapshotsPrescribed;
  int  maximumNumberOfMinorSnapshotsValue;

}; 



#endif // _SNAPSHOT_DATA_H
