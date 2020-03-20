//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef PROGRADE_ALC_UPGRADE_MANAGER_H
#define PROGRADE_ALC_UPGRADE_MANAGER_H

//std
#include <map>
#include <memory>

//DataAccess
#include "Interface.h"
#include "Local2DArray.h"

//CBMGenerics
#include "Polyfunction.h"

//Prograde
#include "IUpgradeManager.h"


//DataModel
#include "AbstractSnapshot.h"

namespace mbapi {
   class MapsManager;
   class Model;
}

namespace DataAccess {
   namespace Interface {
      class ProjectHandle;
      class CrustFormation;
      class GridMap;
      class Snapshot;
   }
}

namespace Prograde
{
   /// @class AlcUpgradeManager Manager to upgrade a legacy ALC project (v2016.11 release) to the latest ALC version
   /// @brief Legacy ALC project are using the BasaltIoTbl and compute the oceanic crust history from the present day basalt thickness
   ///    and the crust thickness at melt onset. Latest ALC reads directly the oceanic crust history from the project file.
   ///    The upgrade manager will create new oceanic crust history maps from the present day basalt thickness and the crust thickness at melt onset and
   ///    add these maps to the new OceaCrustThicknessIoTbl and refer them in GridMapIoTbl.
   ///    The Pressure-Temperature simulation results for a legacy and the corresponding upgraded project will be identical (exluding numerical differences for e.g. HeatFlow).
   class AlcUpgradeManager: public IUpgradeManager {

      typedef DataAccess::Interface::Local2DArray <CBMGenerics::Polyfunction> PolyFunction2DArray;

      typedef std::map<const std::shared_ptr<const DataModel::AbstractSnapshot>,
                       const std::shared_ptr<const DataAccess::Interface::GridMap>,
                       DataModel::AbstractSnapshot::ComparePointers<const std::shared_ptr<const DataModel::AbstractSnapshot>>> SmartAbstractSnapshotVsSmartGridMap;

      public:
         AlcUpgradeManager()                                     = delete;
         AlcUpgradeManager(const AlcUpgradeManager &)            = delete;
         AlcUpgradeManager& operator=(const AlcUpgradeManager &) = delete;

         /// @param[in] model The model on which to perform the ALC upgrade
         /// throw std::invalid_argument if the model's project handle or curst formation is null
         explicit AlcUpgradeManager( mbapi::Model& model );

         ~AlcUpgradeManager() final = default;

         /// @brief Upgrades the model to use the latest ALC
         ///    -create the crust thickness polyfunction in order to access 0Ma thickness
         ///    -compute the oceanic crust (basalt) thickness for each corresponding continental crust entry
         ///    -write the oceanic crust (basalt) thickness maps to Input.HDF under dataset
         ///    -registers the oceanic crust (basalt) thickness maps in the GridMapIoTbl under OceanicCrustThicknessFromLegacyALC_{age}
         ///    -write the oceanic crust (basalt) thickness history table (OceaCrustThicknessIoTbl) from these maps
         ///    -clean the legacy ALC table (BasalatIoTbl)
         /// @details If the model is already using latest ALC, then does nothing
         void upgrade() final;

      private:
         /// @brief Detects if the project is using the legacy ALC by looking for a present day basalt thickness and the crust thickness at melt onset
         /// @return True if the project is using the legacy ALC (ALC mode and has a present day basalt thickness or a crust thickness at melt onset), false otherwise
         bool isLegacyAlc() const;

         /// @brief Read the continental crust thickness history, detects its initial thickness and creates its polyfunction
         /// @details Updates member valriables m_initialCrustalThickness and m_crustThicknessHistory
         void createCrustThickness();

         /// @brief Compute the new basalt thickness history maps from the present day basalt thickness and the crust thickness at melt onset
         /// @details Updates member variable m_basaltThicknessHistory
         void computeBasaltThickness();

         /// @brief Write the OceaCrustalThicknessIoTbl from the basalt thickness history maps stored in m_basaltThicknessHistory
         /// @details Adds basalt thickness maps to Input.HDF and reference them in GridMapIoTbl
         ///    Map naming convention: OceanicCrustThicknessFromLegacyALC_{age}, written to Input.HDF
         void writeOceaCrustalThicknessIoTbl();

         /// @brief Clean the BasaltThicknessIoTbl which is the legacy ALC table
       //  void cleanBasaltThicknessIoTbl() const;

         mbapi::Model& m_model;                                      ///< The model to upgrade
         std::shared_ptr<DataAccess::Interface::ProjectHandle> m_ph; ///< The project handle of the model to upgrade
         const DataAccess::Interface::CrustFormation* m_crust;       ///< The crust formation of the model to upgrade
 
         SmartAbstractSnapshotVsSmartGridMap m_basaltThicknessHistory; ///< The new oceanic crustal thickness grid maps stored by age [Ma, m]
         PolyFunction2DArray m_crustThicknessHistory;                  ///< The continental crustal thickness polyfunction [m]
         double m_initialCrustalThickness;                             ///< The crustal thickness at the age of the basin  [m]
   };
}

#endif