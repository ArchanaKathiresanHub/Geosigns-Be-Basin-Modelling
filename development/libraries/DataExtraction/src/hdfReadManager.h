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

#include "typeDefinitions.h"

#include <hdf5.h>

#include <string>
#include <vector>


namespace DataAccess
{
namespace Interface
{
class ProjectHandle;
}
}

namespace DataExtraction
{

class HDFReadManager
{
public:
  explicit HDFReadManager( const DataAccess::Interface::ProjectHandle* projectHandle );
  virtual ~HDFReadManager();

  void openMapsFile( const std::string& mapFileName );
  void closeMapsFile();
  bool openSnapshotFile( const std::string& snapshotFileName );
  bool checkDataGroup( const std::string& dataGroup );
  void closeSnapshotFile();

  DoubleVector get2dCoordinatePropertyVector( const DoublePairVector&, const std::string& propertyFormationDataGroup );
  DoubleMatrix get3dCoordinatePropertyMatrix( const DoublePairVector&, const std::string& propertyFormationDataGroup );

private:
  double interpolate2d( const double ll, const double lr, const double tl,
                        const double tr, const double di, const double dj );

  const DataAccess::Interface::ProjectHandle* m_projectHandle;
  hid_t m_mapsFileId;
  hid_t m_snapshotFileId;
};


} // namespace DataExtraction
