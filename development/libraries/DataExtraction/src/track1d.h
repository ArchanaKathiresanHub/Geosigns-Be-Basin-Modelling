//
// Copyright ( C ) 2013-2019 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#pragma once


#include "dataExtractor.h"
#include "parseUtilities.h"

#include "DerivedPropertyManager.h"
#include "OutputPropertyValue.h"

#include <map>
#include <string>
#include <vector>

using namespace DataAccess;
using namespace Interface;

namespace GeoPhysics
{
class ObjectFactory;
class ProjectHandle;
}

namespace DataAccess
{
namespace Interface
{
class Grid;
}
}

namespace DataExtraction
{

typedef std::pair < const Formation*, const Surface* > FormationSurface;
typedef std::vector < FormationSurface > FormationSurfaceVector;

typedef std::map < int, double > OutputPropertyDepthMap;
typedef std::map < const Property*, OutputPropertyDepthMap > OutputPropertyValueMap;
typedef std::map < int, OutputPropertyValueMap> CoordsFormationOutputPropertyValueMap;
typedef std::map < const Formation*, CoordsFormationOutputPropertyValueMap > FormationOutputPropertyValueMap;
typedef std::map < const Snapshot*, FormationOutputPropertyValueMap> SnapshotFormationOutputPropertyValueMap;
typedef std::map < const Formation*, unsigned int > FormationMaxKMap;

class Track1d : public DataExtractor
{
public:

  explicit Track1d( const std::string& inputProjectFileName );
  virtual ~Track1d();

  bool isCorrect() const;
  std::vector<unsigned int> retrieveKs( const FormationSurface& formationSurfacePair ) const;

  void acquireCoordinatePairs( DoublePairVector& realWorldCoordinatePairs, DoublePairVector& logicalCoordinatePairs );
  void acquireSnapshots( const DoubleVector& ages );
  void acquireProperties( const StringVector& propertyNamesUser, const bool all2Dproperties, const bool all3Dproperties );
  void acquireFormationSurfacePairs( const StringVector& topSurfaceFormationNames,
                                     const StringVector& bottomSurfaceFormationNames,
                                     const StringVector& formationNames,
                                     const StringVector& formationSurfaceNames,
                                     const bool basement );

  StringVector getPropertyNames( bool all2Dproperties, bool all3Dproperties ) const;
  SnapshotList* getSnapshots() const;
  FormationList* getFormationsSnapshot0() const;

  SnapshotFormationOutputPropertyValueMap allOutputPropertyValues() const;

  bool run( const bool lean );

  void doListProperties();
  void doListSnapshots();
  void doListStratigraphy();

  void writeOutputStream( std::ostream& outputStream, const bool history );

protected:
  Interface::ProjectHandle* getProjectHandle() const;

private:
  void readDataFromHDFfiles();

  bool calculateDerivedProperties();

  DerivedProperties::OutputPropertyValuePtr allocateOutputProperty( DerivedProperties::DerivedPropertyManager& propertyManager,
                                                                    const DataModel::AbstractProperty* property,
                                                                    const DataModel::AbstractSnapshot* snapshot,
                                                                    const FormationSurface& formationItem );
  void createFormationMaxKMap();
  void acquireFormationSurfaces(FormationSurfaceVector& formationSurfacePairs, const StringVector& formationNames, bool useTop );
  void acquireFormations( FormationSurfaceVector& formationSurfacePairs, const StringVector& formationNames );
  void acquireSurfaces( FormationSurfaceVector& formationSurfacePairs, const StringVector& formationSurfaceNames );

  static bool snapshotSorter( const Snapshot* snapshot1, const Snapshot* snapshot2 );
  static bool snapshotIsEqual( const Snapshot* snapshot1, const Snapshot* snapshot2 );

  GeoPhysics::ObjectFactory* m_objectFactory;
  GeoPhysics::ProjectHandle* m_projectHandle;
  FormationMaxKMap m_formationMaxKMap;
  const Grid* m_grid;

  SnapshotList m_snapshots;
  FormationSurfaceVector m_formationSurfacePairs;
  PropertyList m_properties;
  DoublePairVector m_logicalCoordinatePairs;
  DoublePairVector m_realWorldCoordinatePairs;

  SnapshotFormationOutputPropertyValueMap m_allOutputPropertyValues;
  std::string getFormationSurfaceName(const std::vector<unsigned int>& ks, unsigned int k, const Formation* formation, const Surface* surface);
  void outputSnapshotFormationData(std::ostream& outputStream, double x, double y, double i, double j, const Snapshot* snapshot, const std::string& formationName, const std::string& formationSurfaceName, OutputPropertyValueMap& allOutputPropertyValues, unsigned int k);
};

} // namespace DataExtraction
