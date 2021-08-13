//
// Copyright (C) 2010-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include "RequestHandling.h"

#include "Formation.h"
#include "MigrationSurface.h"
#include "rankings.h"
#include "migration.h"
#include "ObjectFactory.h"
#include "MigrationReservoir.h"
#include "Column.h"
#include "BrooksCorey.h"

#include "FiniteElement.h"
#include "FiniteElementTypes.h"

#include "ProjectHandle.h"
#include "RunParameters.h"
#include "Property.h"
#include "Snapshot.h"
#include "PropertyValue.h"
#include "GridMap.h"
#include "Grid.h"
#include "PropertyRetriever.h"
#include "DerivedFormationProperty.h"

#include "Migrator.h"
#include "MigrationPropertyManager.h"
#include "PrimaryFormationProperty.h"
#include "PrimarySurfaceProperty.h"
#include "SmartGridMapRetrieval.h"

#include "LinearGridInterpolator.h"
#include "GeoPhysicsSourceRock.h"
#include "BrooksCorey.h"

// std library
#include <assert.h>
#include <math.h>
#include <iostream>
#include <vector>
using namespace std;

// utilities library
#include "ConstantsMathematics.h"
using Utilities::Maths::CelciusToKelvin;
using Utilities::Maths::MegaPaToPa;
#include "ConstantsNumerical.h"
using Utilities::Numerical::DefaultNumericalTolerance;
#include "NumericFunctions.h"

using namespace CBMGenerics;
using namespace DataAccess;
using namespace FiniteElementMethod;
using namespace AbstractDerivedProperties;

using Interface::Property;
using Interface::Snapshot;
using Interface::GridMap;
using Interface::Grid;
using Interface::ProjectHandle;
using Interface::PropertyValueList;
using Interface::FormationList;

namespace migration
{
   MigrationFormation::MigrationFormation (ProjectHandle& projectHandle, Migrator * const migrator, database::Record * record) :
      Interface::Formation (projectHandle, record),
      GeoPhysics::GeoPhysicsFormation (projectHandle, record),
      m_migrator (migrator), m_index (-1)
   {
      m_isInitialised = false;
      m_detectedReservoir = false;

      m_genexData = 0;

      m_startGenexTime = -1;
      m_endGenexTime = -1;
      m_formationNodeArray = 0;

      // Initializing other data members for Windows build

      for (size_t i = 0; i != ComponentId::NUMBER_OF_SPECIES; ++i)
         m_expulsionGridMaps[i] = NULL;
   }

   MigrationFormation::~MigrationFormation (void)
   {
      if (m_genexData)
      {
         delete m_genexData;
         m_genexData = 0;
      }

      if (m_formationNodeArray)
      {
         delete m_formationNodeArray;
         m_formationNodeArray = 0;
      }
   }

   /// Can't be done in the constructor as we need a propertyvalue to calculate the vertical extent of the formation grid
   void MigrationFormation::createFormationNodes (void)
   {
      if (m_formationNodeArray) return;

      // Using the GeoPhysicsFormation function to get number of elements.
      int depth = getMaximumNumberOfElements ();
      assert (depth > 0);

      // High-resolution grid
      const Grid * grid = m_migrator->getProjectHandle().getActivityOutputGrid ();

      m_formationNodeArray = new FormationNodeArray (this,
         grid->numIGlobal () - 1, grid->numJGlobal () - 1,
         grid->firstI (), Min (grid->lastI (), grid->numIGlobal () - 2),
         grid->firstJ (), Min (grid->lastJ (), grid->numJGlobal () - 2), depth);

   }

   bool MigrationFormation::computePropertyMaps (Interface::GridMap * topDepthGridMap, const Interface::Snapshot * snapshot,
      bool lowResEqualsHighRes, const bool isOverPressureRun, bool nonGeometricLoop, bool chemicalCompaction)
   {
      assert (topDepthGridMap);
      const Grid *grid = getProjectHandle().getActivityOutputGrid ();

      assert (topDepthGridMap->getGrid () == grid);

      // Calculate all properties at once
      if ((m_formationPropertyPtr[DEPTHPROPERTY] = getFormationPropertyPtr ("DepthHighRes", snapshot)) == 0)
         return false;
      if ((m_formationPropertyPtr[PRESSUREPROPERTY] = getFormationPropertyPtr ("Pressure", snapshot)) == 0)
         return false;
      if ((m_formationPropertyPtr[TEMPERATUREPROPERTY] = getFormationPropertyPtr ("Temperature", snapshot)) == 0)
         return false;
      if ((m_formationPropertyPtr[VERTICALPERMEABILITYPROPERTY] = getFormationPropertyPtr ("Permeability", snapshot)) == 0)
         return false;
      if ((m_formationPropertyPtr[HORIZONTALPERMEABILITYPROPERTY] = getFormationPropertyPtr ("HorizontalPermeability", snapshot)) == 0)
         return false;
      if ((m_formationPropertyPtr[POROSITYPROPERTY] = getFormationPropertyPtr ("Porosity", snapshot)) == 0)
         return false;
      if (isOverPressureRun)
      {
         if ((m_formationPropertyPtr[OVERPRESSUREPROPERTY] = getFormationPropertyPtr ("OverPressure", snapshot)) == 0)
            return false;
      }

      int depth = getMaximumNumberOfElements () - 1;
      assert (depth >= 0);

      // Using the PropertyRetriever class which ensures the retrieval and later on the restoration of property pointers
      DerivedProperties::PropertyRetriever depthPropertyRetriever (m_formationPropertyPtr[DEPTHPROPERTY]);
      DerivedProperties::PropertyRetriever pressurePropertyRetriever (m_formationPropertyPtr[PRESSUREPROPERTY]);
      DerivedProperties::PropertyRetriever temperaturePropertyRetriever (m_formationPropertyPtr[TEMPERATUREPROPERTY]);
      DerivedProperties::PropertyRetriever porosityPropertyRetriever (m_formationPropertyPtr[POROSITYPROPERTY]);
      DerivedProperties::PropertyRetriever vPermeabilityPropertyRetriever (m_formationPropertyPtr[VERTICALPERMEABILITYPROPERTY]);
      DerivedProperties::PropertyRetriever hPermeabilityPropertyRetriever (m_formationPropertyPtr[HORIZONTALPERMEABILITYPROPERTY]);
      if (isOverPressureRun)
         DerivedProperties::PropertyRetriever depthPropertyRetriever (m_formationPropertyPtr[OVERPRESSUREPROPERTY]);


      for (unsigned int i = m_formationNodeArray->firstILocal (); i <= m_formationNodeArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_formationNodeArray->firstJLocal (); j <= m_formationNodeArray->lastJLocal (); ++j)
         {
            for (unsigned int k = 0; k <= (unsigned int) depth; ++k)
            {
               LocalFormationNode * formationNode = getLocalFormationNode (i, j, k);

               formationNode->setDepth (m_formationPropertyPtr[DEPTHPROPERTY]->get (i, j, k));
               formationNode->setPressure (m_formationPropertyPtr[PRESSUREPROPERTY]->get (i, j, k));
               formationNode->setTemperature (m_formationPropertyPtr[TEMPERATUREPROPERTY]->get (i, j, k));
               formationNode->setPorosity (m_formationPropertyPtr[POROSITYPROPERTY]->get (i, j, k));
               formationNode->setVerticalPermeability (m_formationPropertyPtr[VERTICALPERMEABILITYPROPERTY]->get (i, j, k));
               if (k == depth)
                  formationNode->setVerticalPermeability (m_formationPropertyPtr[VERTICALPERMEABILITYPROPERTY]->get (i, j, k + 1), true);
               formationNode->setHorizontalPermeability (m_formationPropertyPtr[HORIZONTALPERMEABILITYPROPERTY]->get (i, j, k));
               if (isOverPressureRun)
               {
                  formationNode->setOverPressure (m_formationPropertyPtr[OVERPRESSUREPROPERTY]->get (i, j, k));
               }
            }
         }
      }

      return true;
   }

   bool MigrationFormation::computeCapillaryPressureMaps (Interface::GridMap * topDepthGridMap, const Interface::Snapshot * snapshot)
   {
      assert (topDepthGridMap);
      const Grid *grid = getProjectHandle().getActivityOutputGrid ();

      assert (topDepthGridMap->getGrid () == grid);

      unsigned int depth = getMaximumNumberOfElements ();
      assert (depth > 0);

      const DataModel::AbstractProperty* vapourPcE = m_migrator->getPropertyManager ().getProperty ("CapillaryEntryPressureVapour");
      const DataModel::AbstractProperty* liquidPcE = m_migrator->getPropertyManager ().getProperty ("CapillaryEntryPressureLiquid");

      DerivedProperties::DerivedFormationPropertyPtr ptrVapourPcE = DerivedProperties::DerivedFormationPropertyPtr
         (new DerivedProperties::DerivedFormationProperty (vapourPcE, snapshot, this, grid, depth + 1));
      DerivedProperties::DerivedFormationPropertyPtr ptrLiquidPcE = DerivedProperties::DerivedFormationPropertyPtr
         (new DerivedProperties::DerivedFormationProperty (liquidPcE, snapshot, this, grid, depth + 1));

      assert (ptrVapourPcE);
      assert (ptrLiquidPcE);

      m_formationPropertyPtr[CAPILLARYENTRYPRESSUREVAPOURPROPERTY] = ptrVapourPcE;
      m_formationPropertyPtr[CAPILLARYENTRYPRESSURELIQUIDPROPERTY] = ptrLiquidPcE;

      // Using the PropertyRetriever class which ensures the retrieval and later on the restoration of property pointers
      DerivedProperties::PropertyRetriever pressurePropertyRetriever (m_formationPropertyPtr[PRESSUREPROPERTY]);
      DerivedProperties::PropertyRetriever temperaturePropertyRetriever (m_formationPropertyPtr[TEMPERATUREPROPERTY]);
      DerivedProperties::PropertyRetriever liquidDensityPropertyRetriever (m_formationPropertyPtr[LIQUIDDENSITYPROPERTY]);
      DerivedProperties::PropertyRetriever vapourDensityPropertyRetriever (m_formationPropertyPtr[VAPOURDENSITYPROPERTY]);
      DerivedProperties::PropertyRetriever vPermeabilityPropertyRetriever (m_formationPropertyPtr[VERTICALPERMEABILITYPROPERTY]);

      // Fluid type independent of the position of the node inside the formation.
      // Same for ctitical temperature for vapour and liquid
      const GeoPhysics::FluidType * fluid = (GeoPhysics::FluidType *) getFluidType ();
      double hcTempValueVapour = pvtFlash::getCriticalTemperature (ComponentId::C1, 0);
      double hcTempValueLiquid = pvtFlash::getCriticalTemperature (ComponentId::C6_MINUS_14SAT, 0);

      for (int k = depth; k >= 0; --k)
      {
         for (unsigned int i = ptrVapourPcE->firstI (true); i <= ptrVapourPcE->lastI (true); ++i)
         {
            for (unsigned int j = ptrVapourPcE->firstJ (true); j <= ptrVapourPcE->lastJ (true); ++j)
            {
               bool isNotGhostOrOnBoundary = ( i >= m_formationNodeArray->firstILocal() and i <= m_formationNodeArray->lastILocal() and
                  j >= m_formationNodeArray->firstJLocal() and j <= m_formationNodeArray->lastJLocal() and
                  i < grid->numIGlobal() - 1 and j < grid->numJGlobal() - 1 );

               double pressure = m_formationPropertyPtr[PRESSUREPROPERTY]->get (i, j, k);
               double temperature = m_formationPropertyPtr[TEMPERATUREPROPERTY]->get (i, j, k);
               double liquidDensity = m_formationPropertyPtr[LIQUIDDENSITYPROPERTY]->get (i, j, k);
               double vapourDensity = m_formationPropertyPtr[VAPOURDENSITYPROPERTY]->get (i, j, k);
               double vPermeability = m_formationPropertyPtr[VERTICALPERMEABILITYPROPERTY]->get (i, j, k);

               // If any of those quantities is undefined, the node is undefined,
               // so do not assign any value and continue with the next node.
               if ( NumericFunctions::isEqual(temperature, Interface::DefaultUndefinedMapValue, DefaultNumericalTolerance) or
                    NumericFunctions::isEqual(pressure, Interface::DefaultUndefinedMapValue, DefaultNumericalTolerance) )
                  continue;

               // Brine density may depend on pressure and temperature so needs to be calculated separately for every node.
               double waterDensity = fluid->density (temperature, pressure);

               double capillaryEntryPressureLiquid = Interface::DefaultUndefinedMapValue;
               double capillaryEntryPressureVapour = Interface::DefaultUndefinedMapValue;

               const GeoPhysics::CompoundLithology* compoundLithology = getCompoundLithology( i, j );
               /// Compute capillary pressure if at a valid node
               if ( compoundLithology )
               {
                  const double capC1 = compoundLithology->capC1();
                  const double capC2 = compoundLithology->capC2();

                  double capSealStrength_Air_Hg = CBMGenerics::capillarySealStrength::capSealStrength_Air_Hg( capC1, capC2, vPermeability );

                  // If liquid phase is present (density is a proxy, if 1000, there's only vapour)
                  if (liquidDensity != 1000.0 )
                  {
                     // Calculation of interfacial tension requires a positive density contrast
                     if (waterDensity - liquidDensity < minimumDensityDifference)
                        waterDensity = liquidDensity + minimumDensityDifference;

                     double liquidIFT = CBMGenerics::capillarySealStrength::capTension_H2O_HC( waterDensity, liquidDensity, temperature + CelciusToKelvin, hcTempValueLiquid );
                     capillaryEntryPressureLiquid = CBMGenerics::capillarySealStrength::capSealStrength_H2O_HC( capSealStrength_Air_Hg, liquidIFT );

                     // Vapour phase is also present
                     if (vapourDensity != 1000.0)
                        capillaryEntryPressureVapour = capillaryEntryPressureLiquid + capillaryEntryPressureLiquidVapour( vPermeability, pressure, capC1, capC2 );
                     else
                     {
                        capillaryEntryPressureVapour = capillaryEntryPressureLiquid;
                        vapourDensity = liquidDensity;
                     }
                  }
                  // There's only vapour. Calculate capillary pressures accordingly.
                  else
                  {
                     // Calculation of interfacial tension requires a positive density contrast
                     if (waterDensity - vapourDensity < minimumDensityDifference)
                        waterDensity = vapourDensity + minimumDensityDifference;

                     double vapourIFT = CBMGenerics::capillarySealStrength::capTension_H2O_HC( waterDensity, vapourDensity, temperature + CelciusToKelvin, hcTempValueVapour );
                     capillaryEntryPressureVapour = CBMGenerics::capillarySealStrength::capSealStrength_H2O_HC( capSealStrength_Air_Hg, vapourIFT );

                     capillaryEntryPressureLiquid = capillaryEntryPressureVapour;
                     liquidDensity = vapourDensity;
                  }
               }

               if ( NumericFunctions::isEqual(capillaryEntryPressureLiquid, Interface::DefaultUndefinedMapValue, DefaultNumericalTolerance) or 
                    NumericFunctions::isEqual(capillaryEntryPressureVapour, Interface::DefaultUndefinedMapValue, DefaultNumericalTolerance) ) 
               {
                  ptrVapourPcE->set (i, j, (unsigned int)k, 0.0);
                  ptrLiquidPcE->set (i, j, (unsigned int)k, 0.0);

                  // If not a ghost node and not on last I or J row of the basin then assign the values to the local formation node
                  if ( isNotGhostOrOnBoundary )
                  {
                     // If at the top choose the formation node right below it. We will still calculate and save values for the top node,
                     // but these values will be stored in the arrays of the node below it.
                     LocalFormationNode * formationNode = (k == depth) ? getLocalFormationNode (i, j, k - 1) : getLocalFormationNode (i, j, k);
                     if (!formationNode)
                        continue;

                     formationNode->setCapillaryEntryPressureVapour (0.0, k == depth);
                     formationNode->setCapillaryEntryPressureLiquid (0.0, k == depth);

                     if (k != depth)
                     {
                        formationNode->setVapourDensity (vapourDensity);
                        formationNode->setLiquidDensity (liquidDensity);
                     }
                  }
               }
               else
               {
                  ptrVapourPcE->set (i, j, (unsigned int)k, capillaryEntryPressureVapour);
                  ptrLiquidPcE->set (i, j, (unsigned int)k, capillaryEntryPressureLiquid);

                  // If not a ghost node and not on last I or J row of the basin then assign the values to the local formation node
                  if ( isNotGhostOrOnBoundary )
                  {
                     // If at the top choose the formation node right below it. We will still calculate and save values for the top node,
                     // but these values will be stored in the arrays of the node below it.
                     LocalFormationNode * formationNode = (k == depth) ? getLocalFormationNode (i, j, k - 1) : getLocalFormationNode (i, j, k);
                     if (!formationNode)
                        continue;

                     formationNode->setCapillaryEntryPressureVapour (capillaryEntryPressureVapour, k == depth);
                     formationNode->setCapillaryEntryPressureLiquid (capillaryEntryPressureLiquid, k == depth);

                     if (k != depth)
                     {
                        formationNode->setVapourDensity (vapourDensity);
                        formationNode->setLiquidDensity (liquidDensity);
                     }
                  }
               }
            }
         }
      }

      return true;
   }

   /// To be moved into GeoPhysics library
   double MigrationFormation::capillaryEntryPressureLiquidVapour (const double permeability, const double brinePressure, const double capC1, const double capC2) const
   {
      double pceHgAir;
      double pceog;
      double hgAigToLiquidVapourConversionFactor;

      GeoPhysics::BrooksCorey brooksCorey;

      // Ratio of interfacial-tensions and cos(contact-angle).
      hgAigToLiquidVapourConversionFactor = brooksCorey.liquidVapourInterfacialTension (brinePressure) * GeoPhysics::BrooksCorey::CosLiquidVapourContactAngle /
         //---------------------------------------------------------------------------------------------------------------------------------------------//
         (GeoPhysics::BrooksCorey::MercuryAirInterfacialTension * GeoPhysics::BrooksCorey::CosMercuryAirContactAngle);

      pceHgAir = CBMGenerics::capillarySealStrength::capSealStrength_Air_Hg (capC1, capC2, permeability);
      pceog = pceHgAir * hgAigToLiquidVapourConversionFactor;

      return pceog;
   }

   // DELETE WHEN CERTAIN IT'S NOT NEEDED ANYMORE
   bool MigrationFormation::removeComputedPropertyMaps (void)
   {
#if 0
      for (unsigned int i = 0; i < NUMBEROFPROPERTYINDICES; ++i)
      {
      delete m_gridMaps[i];
      m_gridMaps[i] = 0;
      }
#endif
      return true;
   }

   bool MigrationFormation::computeHCDensityMaps (const Interface::Snapshot * snapshot)
   {
      const Grid *grid = getProjectHandle().getActivityOutputGrid ();

      int depth = getMaximumNumberOfElements ();
      assert (depth > 0);

      const DataModel::AbstractProperty* vapourDensity = m_migrator->getPropertyManager ().getProperty ("HcVapourDensity");
      const DataModel::AbstractProperty* liquidDensity = m_migrator->getPropertyManager ().getProperty ("HcLiquidDensity");

      // We'll be storing the top nodes of each formation
      DerivedProperties::DerivedFormationPropertyPtr ptrVapourDensity = DerivedProperties::DerivedFormationPropertyPtr
         (new DerivedProperties::DerivedFormationProperty (vapourDensity, snapshot, this, grid, depth + 1));

      DerivedProperties::DerivedFormationPropertyPtr ptrLiquidDensity = DerivedProperties::DerivedFormationPropertyPtr
         (new DerivedProperties::DerivedFormationProperty (liquidDensity, snapshot, this, grid, depth + 1));

      assert (ptrVapourDensity);
      assert (ptrLiquidDensity);

      m_formationPropertyPtr[VAPOURDENSITYPROPERTY] = ptrVapourDensity;
      m_formationPropertyPtr[LIQUIDDENSITYPROPERTY] = ptrLiquidDensity;

      double compMasses[ComponentId::NUMBER_OF_SPECIES];
      double phaseCompMasses[Phase::NUMBER_OF_PHASES][ComponentId::NUMBER_OF_SPECIES];
      double phaseDensity   [Phase::NUMBER_OF_PHASES] = { 0 };
      double phaseViscosity [Phase::NUMBER_OF_PHASES] = { 0 };

      // Using the PropertyRetriever class which ensures the retrieval and later on the restoration of property pointers
      DerivedProperties::PropertyRetriever pressurePropertyRetriever (m_formationPropertyPtr[PRESSUREPROPERTY]);
      DerivedProperties::PropertyRetriever temperaturePropertyRetriever (m_formationPropertyPtr[TEMPERATUREPROPERTY]);

      for (int nc = 0; nc != ComponentId::NUMBER_OF_SPECIES; ++nc)
      {

         if (nc == CBMGenerics::ComponentManager::C1 or nc == CBMGenerics::ComponentManager::C6_MINUS_14SAT)
            compMasses[nc] = 1;
         else
            compMasses[nc] = 0;

         phaseCompMasses[0][nc] = 0;
         phaseCompMasses[1][nc] = 0;
      }

      for (int k = depth; k >= 0; --k)
      {
         for (unsigned int i = ptrVapourDensity->firstI (true); i <= ptrVapourDensity->lastI (true); ++i)
         {
            for (unsigned int j = ptrVapourDensity->firstJ (true); j <= ptrVapourDensity->lastJ (true); ++j)
            {
               double temperature = m_formationPropertyPtr[TEMPERATUREPROPERTY]->get (i, j, k);
               double pressure = m_formationPropertyPtr[PRESSUREPROPERTY]->get (i, j, k);
               
               if (NumericFunctions::isEqual(temperature, Interface::DefaultUndefinedMapValue, Utilities::Numerical::DefaultNumericalTolerance) and
                   NumericFunctions::isEqual(pressure, Interface::DefaultUndefinedMapValue, Utilities::Numerical::DefaultNumericalTolerance)
                   )
               {
                  bool flashSuccess = pvtFlash::EosPack::getInstance ().computeWithLumping (temperature + CelciusToKelvin, pressure * MegaPaToPa,
                                                                                            compMasses, phaseCompMasses, phaseDensity, phaseViscosity);

                  ptrVapourDensity->set (i, j, (unsigned int)k, phaseDensity[Phase::VAPOUR]);
                  ptrLiquidDensity->set (i, j, (unsigned int)k, phaseDensity[Phase::LIQUID]);
               }
            }
         }
      }
      return true;
   }

   bool MigrationFormation::computeFiniteElement (int i, int j, int k)
   {
      bool returnValue = true;
      double depthValues[NumberOfNodeCorners];

      int oi;
      for (oi = 0; oi < NumberOfNodeCorners; ++oi)
      {
         double depth = getDepth (i + NodeCornerOffsets[oi][0], j + NodeCornerOffsets[oi][1], k + NodeCornerOffsets[oi][2]);
         depthValues[oi] = depth;
         if ( NumericFunctions::isEqual(depth, Interface::DefaultUndefinedMapValue, DefaultNumericalTolerance) ) returnValue = false;
      }

      setFiniteElementDepths (i, j, k, depthValues);
      return returnValue;
   }

   double MigrationFormation::getFiniteElementValue (int i, int j, int k, double iOffset, double jOffset, double kOffset, PropertyIndex propertyIndex)
   {
      FormationNode * nd = getFormationNode (i, j, k);
      double value;

      if (!nd)
      {
         assert (0);
      }

      value = nd->getFiniteElementValue (iOffset, jOffset, kOffset, propertyIndex);

      return value;
   }

   void MigrationFormation::setFiniteElementDepths (int i, int j, int k, double * depths)
   {
      return getLocalFormationNode (i, j, k)->setFiniteElementDepths (depths);
   }

   void MigrationFormation::getThreeVectorValue (FormationNodeThreeVectorValueRequest & request, FormationNodeThreeVectorValueRequest & response)
   {
      getLocalFormationNode (request.i, request.j, request.k)->getThreeVectorValue (request, response);
   }

   void MigrationFormation::getThreeVector (FormationNodeThreeVectorRequest & request, FormationNodeThreeVectorRequest & response)
   {
      getLocalFormationNode (request.i, request.j, request.k)->getThreeVector (request, response);
   }

   FormationNode * MigrationFormation::getFormationNode (int i, int j, int k) const
   {
      if (k >= (int)getNodeDepth ())
      {
         return getTopFormation ()->getFormationNode (i, j, k - getNodeDepth ());
      }
      else if (k < 0)
      {
         return getBottomFormation ()->getFormationNode (i, j, k + getBottomFormation ()->getNodeDepth ());
      }
      else
      {
         assert (m_formationNodeArray);
         return m_formationNodeArray->getFormationNode (i, j, k);
      }
   }

   LocalFormationNode * MigrationFormation::getLocalFormationNode (int i, int j, int k) const
   {
      if (k >= (int)getNodeDepth ())
      {
         if (getTopFormation ())
            return getTopFormation ()->getLocalFormationNode (i, j, k - getNodeDepth ());
         else
            return 0;
      }
      else if (k < 0)
      {
         if (getBottomFormation ())
            return getBottomFormation ()->getLocalFormationNode (i, j, k + getBottomFormation ()->getNodeDepth ());
         else
            return 0;
      }
      else
      {
         assert (m_formationNodeArray);
         return m_formationNodeArray->getLocalFormationNode (i, j, k);
      }
   }

   int MigrationFormation::getNodeDepth (void) const
   {
      assert (m_formationNodeArray);
      return m_formationNodeArray->depth ();
   }

   int MigrationFormation::getGridMapDepth (void) const
   {
      return getMaximumNumberOfElements () + 1;
   }

   /// Get the value of a property at specified indices.
   /// Will look in adjacent formation if k is out of range
   double MigrationFormation::getPropertyValue (PropertyIndex propertyIndex, int i, int j, int k) const
   {
      double value;
      if (k >= getGridMapDepth ())
      {
         value = getTopFormation ()->getPropertyValue (propertyIndex, i, j, k - (getGridMapDepth () + 1));
      }
      else if (k < 0)
      {
         value = getBottomFormation ()->getPropertyValue (propertyIndex, i, j, k + getBottomFormation ()->getGridMapDepth () - 1);
      }
      else
      {
         if (!m_formationPropertyPtr[propertyIndex])
         {
            value = Interface::DefaultUndefinedMapValue;
         }
         else
         {
            value = m_formationPropertyPtr[propertyIndex]->get ((unsigned int)i, (unsigned int)j, (unsigned int)k);
         }
      }

      return value;
   }

   void MigrationFormation::getTopBottomOverpressures (const int i, const int j, boost::array<double,2> & overPressures) const
   {
      if (m_migrator->isHydrostaticCalculation())
      {
         overPressures[0] = 0.0;
         overPressures[1] = 0.0;
         return;
      }

      DerivedProperties::PropertyRetriever overPressurePropertyRetriever (m_formationPropertyPtr[OVERPRESSUREPROPERTY]);

      const int k = getMaximumNumberOfElements();

      if (m_formationPropertyPtr[OVERPRESSUREPROPERTY] != nullptr)
      {
         // Array has first value at the bottom and second at the top of the formation
         overPressures[0] = m_formationPropertyPtr[OVERPRESSUREPROPERTY]->get (i, j, 0);
         overPressures[1] = m_formationPropertyPtr[OVERPRESSUREPROPERTY]->get (i, j, k);
         
         if (NumericFunctions::isEqual(overPressures[0], m_formationPropertyPtr[OVERPRESSUREPROPERTY]->getUndefinedValue(), Utilities::Numerical::DefaultNumericalTolerance)
             or
             NumericFunctions::isEqual(overPressures[1], m_formationPropertyPtr[OVERPRESSUREPROPERTY]->getUndefinedValue(), Utilities::Numerical::DefaultNumericalTolerance)
             )
         {
            overPressures[0] = 0.0;
            overPressures[1] = 0.0;
         }
      }

      return;
   }

   FiniteElementMethod::ThreeVector & MigrationFormation::getAnalogFlowDirection (int i, int j, int k)
   {
      return getFormationNode (i, j, k)->getAnalogFlowDirection ();
   }

   int MigrationFormation::getIndex (void)
   {
      if (m_index == -1)
      {
         m_index = m_migrator->getIndex (this);
      }

      return m_index;
   }

   bool MigrationFormation::isOnBoundary (const FormationNode * formationNode) const
   {
      bool isOnBoundary = false;
      for (int di = 0; di < NumberOfLateralNeighbourOffsets; ++di)
      {
         FormationNode * neighbourNode = getFormationNode (formationNode->getI () + NeighbourOffsets2D[di][0],
                                                           formationNode->getJ () + NeighbourOffsets2D[di][1],
                                                           formationNode->getK ());
         if (neighbourNode == nullptr)
         {
            isOnBoundary = true;
            break;
         }
      }

      return isOnBoundary;
   }

   bool MigrationFormation::isShallowerThanNeighbour (const FormationNode * formationNode, const int neighbourI, const int neighbourJ) const
   {
      const double nodeDepth = getPropertyValue (DEPTHPROPERTY, formationNode->getI (), formationNode->getJ (), formationNode->getK () + 1);
      const double neighbourDepth = getPropertyValue (DEPTHPROPERTY, neighbourI, neighbourJ, formationNode->getK () + 1);
      
      if (NumericFunctions::isEqual(neighbourDepth, m_formationPropertyPtr[DEPTHPROPERTY]->getUndefinedValue(), Utilities::Numerical::DefaultNumericalTolerance)
          or
          neighbourDepth < nodeDepth
          )
         return false;
      
      if (NumericFunctions::isEqual(neighbourDepth, nodeDepth, Utilities::Numerical::DefaultNumericalTolerance))
      {
         if (formationNode->getI () + formationNode->getJ () > neighbourI + neighbourJ) return true;
         if (formationNode->getI () + formationNode->getJ () < neighbourI + neighbourJ) return false;

         if (formationNode->getI () > neighbourI) return true;
         if (formationNode->getI () < neighbourI) return false;
         if (formationNode->getJ () > neighbourJ) return true;
         if (formationNode->getJ () < neighbourJ) return false;
      }

      return true;
   }

   bool MigrationFormation::computeTargetFormationNodes (MigrationFormation * targetFormation)
   {
      targetFormation->setEndOfPath (); // The path has to stop somewhere if nowhere else

      bool allComputed = false;
      do
      {
         allComputed = true;
         for (MigrationFormation * formation = targetFormation; formation != getBottomFormation (); formation = (MigrationFormation *)formation->getBottomFormation ())
         {
            int minDepthIndex = (formation == this ? formation->getNodeDepth () - 1 : 0);
            int maxDepthIndex = (formation == targetFormation ? formation->getNodeDepth () - 2 : formation->getNodeDepth () - 1);

            for (int depthIndex = maxDepthIndex; depthIndex >= minDepthIndex; --depthIndex)
            {
               allComputed &= formation->computeTargetFormationNodes (depthIndex);
            }
         }
      }
      while (!RequestHandling::AllProcessorsFinished (allComputed));

      return allComputed;
   }

   bool MigrationFormation::computeTargetFormationNodes (int depthIndex)
   {
      RequestHandling::StartRequestHandling (getMigrator (), "computeTargetFormationNodes");

      bool allComputed = false;

      // depends on computations performed on other processors.
      // hence, keep on going (max. 10 times) until all target columns have been computed as it may not go right the first time
      do
      {
         allComputed = true;
         int targetNodesToCompute = 0;
         for (int i = (int)m_formationNodeArray->firstILocal (); i <= (int)m_formationNodeArray->lastILocal (); ++i)
         {
            for (int j = (int)m_formationNodeArray->firstJLocal (); j <= (int)m_formationNodeArray->lastJLocal (); ++j)
            {
               if (!computeTargetFormationNode (i, j, depthIndex))
               {
                  allComputed = false;
                  ++targetNodesToCompute;
               }
            }
         }
      }
      while (!allComputed);

      RequestHandling::FinishRequestHandling ();

      return allComputed;
   }

   bool MigrationFormation::computeTargetFormationNode (unsigned int i, unsigned int j, int depthIndex)
   {
      LocalFormationNode * formationNode = getLocalFormationNode (i, j, depthIndex);

		if (!IsValid (formationNode))
			return true;
			else
			return formationNode->computeTargetFormationNode ();
	 }

   void MigrationFormation::setEndOfPath (void)
   {
      int depthIndex = m_formationNodeArray->depth () - 1;

      for (unsigned int i = m_formationNodeArray->firstILocal (); i <= m_formationNodeArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_formationNodeArray->firstJLocal (); j <= m_formationNodeArray->lastJLocal (); ++j)
         {
            getLocalFormationNode (i, j, depthIndex)->setEndOfPath ();
         }
      }
   }

   bool MigrationFormation::hasVolumePropertyGridMap (const string & propertyName, double snapshotAge) const
   {
      const Property* prop = getProjectHandle().findProperty (propertyName);
      if (!prop)
      {
         cerr << "Basin_Error: Could not find the property " << propertyName << endl;
         return 0;
      }

      const Snapshot * snapshot = getProjectHandle().findSnapshot (snapshotAge);
      if (!snapshot)
      {
         cerr << "Basin_Error: Could not find snapshot with age " << snapshotAge << endl;
         return 0;
      }

      return hasVolumePropertyGridMap (prop, snapshot);
   }

   bool MigrationFormation::hasVolumePropertyGridMap (const Property* prop, double snapshotAge) const
   {
      const Snapshot * snapshot = getProjectHandle().findSnapshot (snapshotAge);
      if (!snapshot)
      {
         cerr << "Basin_Error: Could not find snapshot with age " << snapshotAge << endl;
         return 0;
      }

      return hasVolumePropertyGridMap (prop, snapshot);
   }

   bool MigrationFormation::hasVolumePropertyGridMap (const string & propertyName, const Snapshot * snapshot) const
   {
      const Property* prop = getProjectHandle().findProperty (propertyName);
      if (!prop)
      {
         cerr << "Basin_Error: Could not find the property " << propertyName << endl;
         return 0;
      }

      return hasVolumePropertyGridMap (prop, snapshot);
   }


   bool MigrationFormation::hasVolumePropertyGridMap (const Property* prop, const Snapshot * snapshot) const
   {
      PropertyValueList * propertyValues =
         getProjectHandle().getPropertyValues (Interface::FORMATION,
         prop, snapshot, 0, this, 0,
         Interface::VOLUME);

      bool state = (propertyValues->size () == 1);

      if (!state)
         cerr << "Basin_Error: Could not find the property " << prop->getName () <<
         " of formation " << getName () <<
         " at snapshot " << snapshot->getTime () << endl;

      delete propertyValues;
      return state;
   }

   const GridMap * MigrationFormation::getVolumePropertyGridMap (const string & propertyName, double snapshotAge) const
   {
      const Property* prop = getProjectHandle().findProperty (propertyName);
      if (!prop)
      {
         cerr << "Basin_Error: Could not find the property " << propertyName << endl;
         return 0;
      }

      const Snapshot * snapshot = getProjectHandle().findSnapshot (snapshotAge);
      if (!snapshot)
      {
         cerr << "Basin_Error: Could not find snapshot with age " << snapshotAge << endl;
         return 0;
      }

      return getVolumePropertyGridMap (prop, snapshot);
   }

   const GridMap * MigrationFormation::getVolumePropertyGridMap (const Property* prop, double snapshotAge) const
   {
      const Snapshot * snapshot = getProjectHandle().findSnapshot (snapshotAge);
      if (!snapshot)
      {
         cerr << "Basin_Error: Could not find snapshot with age " << snapshotAge << endl;
         return 0;
      }

      return getVolumePropertyGridMap (prop, snapshot);
   }

   const GridMap * MigrationFormation::getVolumePropertyGridMap (const string & propertyName, const Snapshot * snapshot) const
   {
      const Property* prop = getProjectHandle().findProperty (propertyName);
      if (!prop)
      {
         cerr << "Basin_Error: Could not find the property " << propertyName << endl;
         return 0;
      }

      return getVolumePropertyGridMap (prop, snapshot);
   }


   const GridMap * MigrationFormation::getVolumePropertyGridMap (const Property* prop, const Snapshot * snapshot) const
   {
      unique_ptr<PropertyValueList> propertyValues (getProjectHandle().getPropertyValues (Interface::FORMATION,
         prop, snapshot, 0, this, 0,
         Interface::VOLUME));

      if (propertyValues->size () != 1)
      {
         cerr << "Basin_Error: could not load volume property " << prop->getName () << " for formation " << getName () <<
            " at snapshot " << snapshot->getTime () << ": No or too much data available" << endl;
         return 0;
      }

      const GridMap *gridMap = (*propertyValues)[0]->getGridMap ();

      if (!gridMap)
      {
         cerr << "Basin_Error: could not load volume property " << prop->getName () << " for formation " << getName () <<
            " at snapshot " << snapshot->getTime () << ": Could not load grid map" << endl;
         return 0;
      }

      return gridMap;
   }

   FormationSurfaceGridMaps MigrationFormation::getFormationSurfaceGridMaps (const Property* prop, const Snapshot* snapshot) const
   {
      unsigned int index = 0;

      const GridMap* top = getFormationPrimaryPropertyGridMap (prop, snapshot);
      const GridMap* base = top;

      if (top)
      {
         index = top->getDepth () - 1;
      }
      else
      {
         top = getSurfacePropertyGridMap (prop, snapshot, getTopSurface ());
         base = getSurfacePropertyGridMap (prop, snapshot, getBottomSurface ());
         if (!top)
         {
            //     cout << "No top property " << prop->getName() << " " << this->getName() << " " << snapshot->getTime() << endl;
         }
         if (!base)
         {
            //      cout << "No base property " << prop->getName() << " " << this->getName() << " " << snapshot->getTime() << endl;
         }
      }

      return FormationSurfaceGridMaps (SurfaceGridMap (top, index), SurfaceGridMap (base, (unsigned int)0), this);
   }


   SurfaceGridMap MigrationFormation::getTopSurfaceGridMap (const Property* prop, const Snapshot* snapshot) const
   {
      unsigned int index = 0;
      const GridMap* top = getFormationPrimaryPropertyGridMap (prop, snapshot);

      if (top)
      {
         index = top->getDepth () - 1;
      }
      else
      {
         top = getSurfacePropertyGridMap (prop, snapshot, getTopSurface ());
      }

      return SurfaceGridMap (top, index);
   }

   SurfaceGridMap MigrationFormation::getBaseSurfaceGridMap (const Property* prop, const Snapshot* snapshot) const
   {
      const GridMap* base = getFormationPrimaryPropertyGridMap (prop, snapshot);

      if (!base)
         base = getSurfacePropertyGridMap (prop, snapshot, getBottomSurface ());

      return SurfaceGridMap (base, (unsigned int)0);
   }


   MigrationFormation * MigrationFormation::getTopFormation () const
   {
      const Interface::Surface * surface = getTopSurface ();

      return CastToFormation (surface ? surface->getTopFormation () : 0);
   }

   MigrationFormation * MigrationFormation::getBottomFormation () const
   {
      const Interface::Surface * surface = getBottomSurface ();

      return CastToFormation (surface ? surface->getBottomFormation () : 0);
   }

   bool MigrationFormation::isActive (const DataAccess::Interface::Snapshot * snapshot) const
   {
      return *(getBottomSurface ()->getSnapshot ()) < *snapshot;
   }

   void MigrationFormation::getValue (FormationNodeValueRequest & request, FormationNodeValueRequest & response)
   {
      LocalFormationNode * nd = getLocalFormationNode (request.i, request.j, request.k);

      if (!nd)
      {
         assert (0);
      }

      nd->getValue (request, response);
   }

   void MigrationFormation::setValue (FormationNodeValueRequest & request)
   {
      LocalFormationNode * nd = getLocalFormationNode (request.i, request.j, request.k);

      if (!nd)
      {
         assert (0);
      }

      nd->setValue (request);
   }

   FormationPropertyPtr MigrationFormation::getFormationPropertyPtr (const string & propertyName, const Interface::Snapshot * snapshot) const
   {

      const DataAccess::Interface::Property* property = m_migrator->getProjectHandle().findProperty (propertyName);

      FormationPropertyPtr theProperty =
         m_migrator->getPropertyManager ().getFormationProperty (property, snapshot, this);

      return theProperty;
   }

   // Use this method for getting a formation map of "primary" properties. If the formation map is not available calculate a surface map on-the-fly.
   const GridMap* MigrationFormation::getFormationPrimaryPropertyGridMap (const Property* prop, const Interface::Snapshot * snapshot) const
   {

      const GridMap* theMap = 0;

      FormationPropertyPtr theProperty = m_migrator->getPropertyManager ().getFormationProperty (prop, snapshot, this);
      if (theProperty != 0)
      {
         const DerivedProperties::PrimaryFormationProperty * thePrimaryProperty = dynamic_cast<const DerivedProperties::PrimaryFormationProperty *>(theProperty.get ());

         if (thePrimaryProperty != 0)
         {
            theMap = thePrimaryProperty->getGridMap ();
         }
      }

      return theMap;
   }

   const GridMap* MigrationFormation::getSurfacePropertyGridMap (const Property* prop, const Snapshot* snapshot, const Interface::Surface* surface) const
   {

      const GridMap* theMap = 0;

      if (prop->getName ().find ("Permeability") == std::string::npos )
      {

         SurfacePropertyPtr theProperty;

         // the way MigrationFormation::getSurfacePropertyGridMap is used, we can also return a 0 if the property cannot be computed (it is supposed to do it)
         // However we need to catch the exception, othrwise the program will terminate.
         try
         {
            theProperty = m_migrator->getPropertyManager().getSurfaceProperty(prop, snapshot, surface);
         }
         catch ( formattingexception::GeneralException& )
         {
            theProperty = 0;
         }

         if (theProperty != 0)
         {

            const DerivedProperties::PrimarySurfaceProperty * thePrimaryProperty = dynamic_cast<const DerivedProperties::PrimarySurfaceProperty *>(theProperty.get ());

            if (thePrimaryProperty != 0)
            {
               theMap = thePrimaryProperty->getGridMap ();
            }
            else
            {
               theMap = m_migrator->getPropertyManager ().produceDerivedGridMap (theProperty);
            }

         }
         else
         {
            FormationSurfacePropertyPtr theFormationProperty;

            try
            {
               theFormationProperty = m_migrator->getPropertyManager().getFormationSurfaceProperty(prop, snapshot, this, surface);
            }
            catch ( formattingexception::GeneralException& )
            {
               theFormationProperty = 0;
            }

            if (theFormationProperty != 0)
            {
               theMap = m_migrator->getPropertyManager ().produceDerivedGridMap (theFormationProperty);
            }

         }
      }
      else
      {
         FormationSurfacePropertyPtr theFormationProperty;

         try
         {
            theFormationProperty = m_migrator->getPropertyManager().getFormationSurfaceProperty(prop, snapshot, this, surface);
         }
         catch ( formattingexception::GeneralException& )
         {
            theFormationProperty = 0;
         }

         if (theFormationProperty != 0)
         {
            theMap = m_migrator->getPropertyManager ().produceDerivedGridMap (theFormationProperty);
         }
      }

      return theMap;
   }

   bool MigrationFormation::saveComputedSMFlowPaths (MigrationFormation * targetFormation, const Interface::Snapshot * end)
   {
      MigrationFormation * formation;

      MigrationFormation * formationAboveTarget = (MigrationFormation *)targetFormation->getTopFormation ();

      for (formation = this; formation != formationAboveTarget; formation = (MigrationFormation *)formation->getTopFormation ())
      {
         formation->saveComputedSMFlowPathsByGridOffsets (end);
      }

      return true;
   }

   bool MigrationFormation::saveComputedSMFlowPathsByGridOffsets (const Interface::Snapshot * end)
   {
      ((Interface::Snapshot *) end)->setAppendFile (false);
      if (!m_formationNodeArray)
         return true;

      unsigned int nodeDepth = (unsigned int)getNodeDepth ();

      // generate file name
      ostringstream buf;

      buf.setf (ios::fixed);
      buf.precision (6);
      buf << flowPathsFileNamePrefix << "_" << end->getTime () << ".h5";

      Interface::PropertyValue * propertyValue =
         m_migrator->getProjectHandle().createVolumePropertyValue ("FlowDirectionIJK", end, 0, this, nodeDepth + 1, buf.str ());
      assert (propertyValue);

      GridMap *gridMap = propertyValue->getGridMap ();

      assert (gridMap);

      gridMap->retrieveData ();

      // Get the reservoir object of this formation (if it hosts a reservoir)
      // in order to use FlowDirectionIJ values for lateral migration
      DataAccess::Interface::ReservoirList * reservoirList = m_migrator->getReservoirs (this);
      const MigrationReservoir * reservoir = 0;
      if (!reservoirList->empty ())
      {
         const DataAccess::Interface::Reservoir * dataAccessReservoir = (*reservoirList)[0];
         reservoir = dynamic_cast<const migration::MigrationReservoir *> (dataAccessReservoir);
         assert (reservoir);
      }

      for (unsigned int i = m_formationNodeArray->firstILocal (); i <= m_formationNodeArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_formationNodeArray->firstJLocal (); j <= m_formationNodeArray->lastJLocal (); ++j)
         {
            for (unsigned int k = 0; k < nodeDepth; ++k)
            {
               LocalFormationNode * node = getLocalFormationNode (i, j, k);
               double gridMapValue;

               // If there's an end-of-path node inside a reservoir it means we are at the reservoir top.
               // Use FlowDirectionIJ then to get non-zero values for lateral migration.
               if (reservoir and node->isEndOfPath ())
               {
                  LocalColumn * localColumn = reservoir->getLocalColumn (i, j);
                  if (IsValid(localColumn))
                     gridMapValue = 10 * localColumn->getFlowDirectionJ (migration::OIL) + 1 * localColumn->getFlowDirectionI (migration::OIL);
                  else
                     gridMapValue = Interface::DefaultUndefinedMapValue;
               }
               else
               {
                  if (m_migrator->performAdvancedMigration ())
                  {
                     gridMapValue =
                        100 * static_cast<double>(node->getAdjacentFormationNodeGridOffset (2)) +     // K
                        10 * static_cast<double>(node->getAdjacentFormationNodeGridOffset (1)) +     // J
                        1 * static_cast<double>(node->getAdjacentFormationNodeGridOffset (0));      // I
                  }
                  else
                  {
                     gridMapValue = 100;
                  }
               }

               gridMap->setValue (i, j, k, gridMapValue);
            }
         }
      }
      gridMap->restoreData ();

      //when calling getReservoirs (this) a new ReservoirList is created in project handle.
      delete reservoirList;

      return true;
   }

   bool MigrationFormation::retrievePropertyMaps (bool retrieveCapillary)
   {
      assert (m_formationPropertyPtr[DEPTHPROPERTY]);
      m_formationPropertyPtr[DEPTHPROPERTY]->retrieveData ();

      assert (m_formationPropertyPtr[VERTICALPERMEABILITYPROPERTY]);
      m_formationPropertyPtr[VERTICALPERMEABILITYPROPERTY]->retrieveData ();

      assert (m_formationPropertyPtr[HORIZONTALPERMEABILITYPROPERTY]);
      m_formationPropertyPtr[HORIZONTALPERMEABILITYPROPERTY]->retrieveData ();

      assert (m_formationPropertyPtr[POROSITYPROPERTY]);
      m_formationPropertyPtr[POROSITYPROPERTY]->retrieveData ();

      assert (m_formationPropertyPtr[TEMPERATUREPROPERTY]);
      m_formationPropertyPtr[TEMPERATUREPROPERTY]->retrieveData ();

      assert (m_formationPropertyPtr[PRESSUREPROPERTY]);
      m_formationPropertyPtr[PRESSUREPROPERTY]->retrieveData ();

      if (retrieveCapillary)
      {
         assert (m_formationPropertyPtr[CAPILLARYENTRYPRESSUREVAPOURPROPERTY]);
         m_formationPropertyPtr[CAPILLARYENTRYPRESSUREVAPOURPROPERTY]->retrieveData ();

         assert (m_formationPropertyPtr[CAPILLARYENTRYPRESSURELIQUIDPROPERTY]);
         m_formationPropertyPtr[CAPILLARYENTRYPRESSURELIQUIDPROPERTY]->retrieveData ();
      }
      return true;
   }

   bool MigrationFormation::restorePropertyMaps (bool restoreCapillary)
   {
      assert (m_formationPropertyPtr[DEPTHPROPERTY]);
      m_formationPropertyPtr[DEPTHPROPERTY]->restoreData ();

      assert (m_formationPropertyPtr[VERTICALPERMEABILITYPROPERTY]);
      m_formationPropertyPtr[VERTICALPERMEABILITYPROPERTY]->restoreData ();

      assert (m_formationPropertyPtr[HORIZONTALPERMEABILITYPROPERTY]);
      m_formationPropertyPtr[HORIZONTALPERMEABILITYPROPERTY]->restoreData ();

      assert (m_formationPropertyPtr[POROSITYPROPERTY]);
      m_formationPropertyPtr[POROSITYPROPERTY]->restoreData ();

      assert (m_formationPropertyPtr[TEMPERATUREPROPERTY]);
      m_formationPropertyPtr[TEMPERATUREPROPERTY]->restoreData ();

      assert (m_formationPropertyPtr[PRESSUREPROPERTY]);
      m_formationPropertyPtr[PRESSUREPROPERTY]->restoreData ();

      if (restoreCapillary)
      {
         assert (m_formationPropertyPtr[CAPILLARYENTRYPRESSUREVAPOURPROPERTY]);
         m_formationPropertyPtr[CAPILLARYENTRYPRESSUREVAPOURPROPERTY]->restoreData ();

         assert (m_formationPropertyPtr[CAPILLARYENTRYPRESSURELIQUIDPROPERTY]);
         m_formationPropertyPtr[CAPILLARYENTRYPRESSURELIQUIDPROPERTY]->restoreData ();
      }
      return true;
   }

   void MigrationFormation::deleteFormationProperties ()
   {
      for (size_t i = 0; i != NUMBEROFPROPERTYINDICES; ++i)
      {
         if (m_formationPropertyPtr[i] != nullptr)
         {

            //release only primary formation property grid maps since they are the only ones to have a grid map
            const auto primaryFormationPropertyPointer = dynamic_pointer_cast<const DerivedProperties::PrimaryFormationProperty>(m_formationPropertyPtr[i]);
            if(primaryFormationPropertyPointer != nullptr){
               const auto gridMap = primaryFormationPropertyPointer->getGridMap();
               if(gridMap != nullptr){
                  gridMap->release();
               }
            }

            //reset all formation property pointers
            m_formationPropertyPtr[i].reset ();
         }
      }
   }

   bool MigrationFormation::clearNodeProperties (void)
   {
      if (m_formationNodeArray) m_formationNodeArray->clearProperties ();
      return true;
   }

   bool MigrationFormation::computeNodeProperties ()
   {
      if (m_formationNodeArray) m_formationNodeArray->computeProperties ();
      return true;
   }

   bool MigrationFormation::clearNodeReservoirProperties (void)
   {
      if (m_formationNodeArray) m_formationNodeArray->clearReservoirProperties ();
      return true;
   }

   // Returns the first non-zero-thickness element in the current formation, or a null pointer if such an element does not exist
   LocalFormationNode * MigrationFormation::validReservoirNode (const int i, const int j) const
   {
      //reservoir formation, loop downwards until a formation node with thickness is found in the reservoir formation
      LocalFormationNode * reservoirFormationNode = 0;
      bool validReservoirFormationNode = false;
      int k = getNodeDepth () - 1;

      while (k >= 0 && validReservoirFormationNode == false)
      {
         reservoirFormationNode = getLocalFormationNode (i, j, k);
         if (reservoirFormationNode && !reservoirFormationNode->hasNoThickness ()) validReservoirFormationNode = true;
         else
            --k;
      }

      if (validReservoirFormationNode)
         return reservoirFormationNode;
      else
         return nullptr;
   }


   // Returns the bottommost non-zero-thickness element in topFormation, or any formation above, or a null pointer if such an element does not exist
   LocalFormationNode * MigrationFormation::validSealNode (const int i, const int j, const MigrationFormation * topFormation, const MigrationFormation * topActiveFormation) const
   {
      //seal formation, loop upwards until a formation node with thickness is found in one of the formations above the reservoir
      LocalFormationNode * sealFormationNode = 0;
      bool validSealFormationNode = false;
      int k = 0;
      int ktopseal = topActiveFormation->getNodeDepth () - 1;

      while (validSealFormationNode == false)
      {
         sealFormationNode = topFormation->getLocalFormationNode (i, j, k);
         if (sealFormationNode && !sealFormationNode->hasNoThickness ())
         {
            validSealFormationNode = true;
         }
         else
         {
            if (!sealFormationNode || (sealFormationNode->getFormation () == topActiveFormation && sealFormationNode->getK () == ktopseal)) break;
            ++k;
         }
      }

      if (validSealFormationNode)
         return sealFormationNode;
      else
         return nullptr;
   }

   //
   // Loop through the uppermost cells and check capillary pressure across the boundary
   //
   bool MigrationFormation::detectReservoir (MigrationFormation * topFormation,
      const double minLiquidColumnHeight, const double minVapourColumnHeight, const bool pressureRun, const MigrationFormation * topActiveFormation)
   {
      for (int i = (int)m_formationNodeArray->firstILocal (); i <= (int)m_formationNodeArray->lastILocal (); ++i)
      {
         for (int j = (int)m_formationNodeArray->firstJLocal (); j <= (int)m_formationNodeArray->lastJLocal (); ++j)
         {
            LocalFormationNode * reservoirFormationNode = validReservoirNode (i, j);
            LocalFormationNode * sealFormationNode = validSealNode (i, j, topFormation, topActiveFormation);

            if (reservoirFormationNode && sealFormationNode) reservoirFormationNode->detectReservoir (sealFormationNode, minLiquidColumnHeight, minVapourColumnHeight, pressureRun);
         }
      }

      return false;
   }

   // Loop through the uppermost cells and check if a trap crests exist with m_height_oil > minOilColumnHeight OR m_height_gas > minGasColumnHeight
   // Stop as soon as a trap crest is found.
   bool MigrationFormation::detectReservoirCrests ()
   {
      //cout << " Rank, MigrationFormation, m_detectedReservoir " << GetRank () << " " << getName () << " " << m_detectedReservoir << endl;
      bool reservoirCrestDetected = false;

      if (!m_detectedReservoir)
      {
         RequestHandling::StartRequestHandling (getMigrator (), "detectReservoirCrests");

         int upperIndex = getNodeDepth () - 1;
         // First  oil
         for (int i = (int)m_formationNodeArray->firstILocal (); i <= (int)m_formationNodeArray->lastILocal (); ++i)
         {
            for (int j = (int)m_formationNodeArray->firstJLocal (); j <= (int)m_formationNodeArray->lastJLocal (); ++j)
            {
               LocalFormationNode * reservoirFormationNode = validReservoirNode (i, j);
               if (reservoirFormationNode) reservoirCrestDetected = reservoirFormationNode->detectReservoirCrests (OIL);
               if (reservoirCrestDetected) break;
            }
            if (reservoirCrestDetected) break;
         }

         // Then gas
         if (!reservoirCrestDetected)
         {
            for (int i = (int)m_formationNodeArray->firstILocal (); i <= (int)m_formationNodeArray->lastILocal (); ++i)
            {
               for (int j = (int)m_formationNodeArray->firstJLocal (); j <= (int)m_formationNodeArray->lastJLocal (); ++j)
               {
                  LocalFormationNode * reservoirFormationNode = validReservoirNode (i, j);
                  if (reservoirFormationNode) reservoirCrestDetected = reservoirFormationNode->detectReservoirCrests (GAS);
                  if (reservoirCrestDetected) break;
               }
               if (reservoirCrestDetected) break;
            }
         }
         RequestHandling::FinishRequestHandling ();
      }
      else
      {
         reservoirCrestDetected = m_detectedReservoir;
      }

      return MaximumAll ((int)reservoirCrestDetected);

   }

   bool MigrationFormation::getDetectedReservoir () const
   {
      return m_detectedReservoir;
   }

   void MigrationFormation::setDetectedReservoir (bool detectedReservoir)
   {
      m_detectedReservoir = detectedReservoir;
   }

   // add the detected reservoir to the reservoir vector
   void MigrationFormation::addDetectedReservoir (const Interface::Snapshot * start)
   {
      if (!m_detectedReservoir)
      {
         //add a record to the reservoir list
         database::Record * record = m_migrator->addDetectedReservoirRecord (this, start);
         MigrationReservoir* reservoir = dynamic_cast<MigrationReservoir*>(getProjectHandle().addDetectedReservoirs (record, this));
         m_detectedReservoir = true;
      }
   }

   void MigrationFormation::saveReservoir (const Interface::Snapshot * curSnapshot)
   {
      if (!performHDynamicAndCapillary ())
         return;

      if (static_cast <int>(curSnapshot->getTime ()) != 0) return;
      int upperIndex = getNodeDepth () - 1;

      char fname[256];

      snprintf (fname, sizeof (fname), "%s_%d.txt", getName ().c_str (), static_cast <int>(curSnapshot->getTime ()));
      FILE *fres = fopen (fname, "w");
      LocalFormationNode *fnode;

      for (int i = (int)m_formationNodeArray->firstILocal (); i <= (int)m_formationNodeArray->lastILocal (); ++i)
      {
         for (int j = (int)m_formationNodeArray->firstJLocal (); j <= (int)m_formationNodeArray->lastJLocal (); ++j)
         {
            fprintf (fres, "%d %d\t", i, j);
            fnode = getLocalFormationNode (i, j, upperIndex);

            double depth = 0;
            int top = fnode->getFormation ()->getGridMapDepth () - 1;
            depth = fnode->getFormation ()->getPropertyValue (DEPTHPROPERTY, fnode->getI (), fnode->getJ (), top);

            fprintf (fres, "%d(%lf)\t\t%d(%lf) \t\t %d \t\t %d \t\t %lf\n", (fnode->getReservoirLiquid () ? 1 : 0), fnode->getHeightLiquid (),
               (fnode->getReservoirVapour () ? 1 : 0), fnode->getHeightVapour (), (fnode->getIsCrest (OIL) ? 1 : 0), (fnode->getIsCrest (GAS) ? 1 : 0), depth);
         }
      }

      fclose (fres);
   }

   void MigrationFormation::identifyAsReservoir (const bool advancedMigration) const
   {
      int depthIndex = getNodeDepth () - 1;
      assert (depthIndex >= 0);

      for (int i = (int)m_formationNodeArray->firstILocal (); i <= (int)m_formationNodeArray->lastILocal (); ++i)
      {
         for (int j = (int)m_formationNodeArray->firstJLocal (); j <= (int)m_formationNodeArray->lastJLocal (); ++j)
         {
            const Interface::Formation * formation = dynamic_cast<const Interface::Formation *> (this);
            assert (formation);

            Interface::ReservoirList * reservoirs = getProjectHandle().getReservoirs (formation);

            const MigrationReservoir * reservoir = dynamic_cast<const migration::MigrationReservoir *> (*reservoirs->begin ());
            assert (reservoir);

            double formationThickness = getDepth (i, j, 0) - getDepth (i, j, depthIndex + 1);

            if (getLocalFormationNode (i, j, depthIndex)->hasThickness () and
               getDepth (i, j, depthIndex + 1) < getDepth (i, j, depthIndex) or depthIndex == 0) // Top node is flagged
            {
               getLocalFormationNode (i, j, depthIndex)->identifyAsReservoir (advancedMigration);
            }
            else // There zero-thickness elements, so the correct node needs to be found
            {
               int depth = depthIndex;
               while (depth > 0 and
                  (getDepth (i, j, depth) < getDepth (i, j, depthIndex + 1)
                  or !getLocalFormationNode (i, j, depth)->hasThickness ()))
               {
                  --depth;
                  if (depth == 0)
                     break;
               }

               getLocalFormationNode (i, j, depth)->identifyAsReservoir (advancedMigration);
            }

            delete reservoirs;
         }
      }
   }

   void MigrationFormation::loadExpulsionMaps (const Interface::Snapshot * begin, const Interface::Snapshot * end)
   {
      PetscBool genexMinorSnapshots;
      PetscBool genexFraction;

      PetscOptionsHasName (PETSC_IGNORE, PETSC_IGNORE, "-genex",  &genexMinorSnapshots);
      PetscOptionsHasName (PETSC_IGNORE, PETSC_IGNORE, "-genexf", &genexFraction);

      const double depoTime = getTopSurface ()->getSnapshot ()->getTime ();
      bool sourceRockIsActive = (depoTime > begin->getTime ()) or fabs (depoTime - begin->getTime ()) < Genex6::Constants::Zero;

      // Nothing to load/calculate if SR is not active yet
      if (!sourceRockIsActive)
         return;

      // If "-genex" then compute expulsion on the fly
      if (genexMinorSnapshots)
      {
         computeExpulsionMapsOnTheFly (begin, end);
         return;
      }

      // Otherwise, load expulsion maps as usual, straight from Genex results
      for (int componentId = ComponentId::FIRST_COMPONENT; componentId < ComponentId::NUMBER_OF_SPECIES; ++componentId)
      {
         if (!ComponentsUsed[componentId]) continue;

         string propertyName = CBMGenerics::ComponentManager::getInstance().getSpeciesName( componentId );
         propertyName += "ExpelledCumulative";

         const GridMap * gridMapEnd = getPropertyGridMap (propertyName, end);
         const GridMap * gridMapStart = getPropertyGridMap (propertyName, begin);

         if (gridMapEnd and gridMapStart)
         {
            Interface::SubtractionFunctor subtract;
            m_expulsionGridMaps[componentId] = m_migrator->getProjectHandle().getFactory ()->produceGridMap (0, 0, gridMapEnd, gridMapStart, subtract);
         }
         else if (gridMapEnd)
         {
            Interface::IdentityFunctor idFunctor;
            m_expulsionGridMaps[componentId] = m_migrator->getProjectHandle().getFactory ()->produceGridMap (0, 0, gridMapEnd, idFunctor);
         }
         else if (gridMapStart)
         {
            Interface::IdentityMinusFunctor idMinusFunctor;
            m_expulsionGridMaps[componentId] = m_migrator->getProjectHandle().getFactory ()->produceGridMap (0, 0, gridMapStart, idMinusFunctor);
         }

         if (m_expulsionGridMaps[componentId])
         {
            m_expulsionGridMaps[componentId]->retrieveData (true); // divided over different nodes
         }

         if (gridMapEnd)  gridMapEnd->release ();
         if (gridMapStart) gridMapStart->release ();
      }
   }

   /// remove the expulsion gridmaps
   void MigrationFormation::unloadExpulsionMaps ()
   {
      for (int componentId = ComponentId::FIRST_COMPONENT; componentId < ComponentId::NUMBER_OF_SPECIES; ++componentId)
      {
         if (!ComponentsUsed[componentId])
            continue;
         if (!m_expulsionGridMaps || !m_expulsionGridMaps[componentId])
            continue;

         m_expulsionGridMaps[componentId]->restoreData (false); // no need to save
         delete m_expulsionGridMaps[componentId];
         m_expulsionGridMaps[componentId] = 0;
      }
   }

   void MigrationFormation::computeExpulsionMapsOnTheFly (const Interface::Snapshot * begin, const Interface::Snapshot * end)
   {
      PetscBool printDebug;
      PetscOptionsHasName (PETSC_IGNORE, PETSC_IGNORE, "-debug",  &printDebug);

      if (!isPreprocessed ())
      {
         bool status = preprocessSourceRock (begin->getTime (), printDebug);
         if (!status and GetRank () == 0)
         {
            const double depoTime = getTopSurface ()->getSnapshot ()->getTime ();
            LogHandler (LogHandler::ERROR_SEVERITY) << "Cannot preprocess " << getName () << ", depoage= " << depoTime << " at " << begin->getTime ();
         }
      }

      bool status = calculateGenexTimeInterval (begin, end, printDebug);
      if (!status and GetRank () == 0)
      {
         const double depoTime = getTopSurface ()->getSnapshot ()->getTime ();
         LogHandler (LogHandler::ERROR_SEVERITY) << "Cannot calculate genex " << getName () << ", depoage= " << depoTime << " at " << begin->getTime ();
      }

      if (m_genexData != nullptr)
      {
         m_genexData->retrieveData ();
      }
   }

   bool MigrationFormation::computeFaults (const Interface::Snapshot * end)
   {
      const GridMap * gridMap = computeFaultGridMap (getGrid (), end);
      if (!gridMap) return false;

      unsigned int gridMapDepth = gridMap->getDepth (); // should be 1

      gridMap->retrieveData ();

      for (unsigned int i = m_formationNodeArray->firstILocal (); i <= m_formationNodeArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_formationNodeArray->firstJLocal (); j <= m_formationNodeArray->lastJLocal (); ++j)
         {
            FaultStatus fs;

            double gmValue = gridMap->getValue (i, j, gridMapDepth - 1);
            if ( NumericFunctions::isEqual(gmValue, gridMap->getUndefinedValue(), DefaultNumericalTolerance) )
            {
               fs = NOFAULT;
            }
            else
            {
               fs = FaultStatus ((int)gmValue);
            }

            for (unsigned int k = 0; k < (unsigned int)getNodeDepth (); ++k)
            {
               getLocalFormationNode (i, j, k)->setFaultStatus (fs);
            }
         }
      }

      gridMap->restoreData ();
      delete gridMap; // required, not a PropertyValue GridMap

      return true;
   }

   bool MigrationFormation::computeAnalogFlowDirections (MigrationFormation * targetFormation, const Interface::Snapshot * begin,
      const Interface::Snapshot * end)
   {
      //DebugProgress (getName (), ": computeAnalogFlowDirections () upto ", targetFormation->getName (), end->getTime ());
      MigrationFormation * formation;

      MigrationFormation * formationAboveTarget = (MigrationFormation *)targetFormation->getTopFormation ();

      for (formation = this; formation != formationAboveTarget; formation = (MigrationFormation *)formation->getTopFormation ())
      {
         int firstDepthIndex = 0;
         if (formation == this) firstDepthIndex = getNodeDepth () - 1;

         int lastDepthIndex = formation->getNodeDepth () - 1;
         for (int depthIndex = firstDepthIndex; depthIndex <= lastDepthIndex; ++depthIndex)
         {
            formation->computeAnalogFlowDirections (depthIndex, begin, end); // also computes formation node depths
         }
      }

      return true;
   }

   bool MigrationFormation::computeAdjacentNodes (MigrationFormation * targetFormation, const Interface::Snapshot * begin,
      const Interface::Snapshot * end)
   {
      MigrationFormation * formation;

      MigrationFormation * formationAboveTarget = (MigrationFormation *)targetFormation->getTopFormation ();

      for (formation = this; formation != formationAboveTarget; formation = (MigrationFormation *)formation->getTopFormation ())
      {
         int firstDepthIndex = 0;
         if (formation == this) firstDepthIndex = getNodeDepth () - 1;

         int lastDepthIndex = formation->getNodeDepth () - 1;
         if (formation == targetFormation) lastDepthIndex = formation->getNodeDepth () - 2;

         for (int depthIndex = firstDepthIndex; depthIndex <= lastDepthIndex; ++depthIndex)
         {
            formation->computeAdjacentNodes (depthIndex, begin, end);
         }
      }

      return true;
   }

   bool migration::MigrationFormation::computeAnalogFlowDirections (int depthIndex, const Interface::Snapshot * begin,
      const Interface::Snapshot * end)
   {
#if 0
      cerr << getName () << "(" << depthIndex << ")->computeAnalogFlowDirections ()" << endl;
#endif
      for (int i = (int)m_formationNodeArray->firstILocal (); i <= (int)m_formationNodeArray->lastILocal (); ++i)
      {
         for (int j = (int)m_formationNodeArray->firstJLocal (); j <= (int)m_formationNodeArray->lastJLocal (); ++j)
         {
            getLocalFormationNode (i, j, depthIndex)->computeAnalogFlowDirection ();
         }
      }

      return true;
   }

   bool MigrationFormation::computeAdjacentNodes (int depthIndex, const Interface::Snapshot * begin, const Interface::Snapshot * end)
   {
      RequestHandling::StartRequestHandling (getMigrator (), "computeAdjacentNodes");

      for (int i = (int)m_formationNodeArray->firstILocal (); i <= (int)m_formationNodeArray->lastILocal (); ++i)
      {
         for (int j = (int)m_formationNodeArray->firstJLocal (); j <= (int)m_formationNodeArray->lastJLocal (); ++j)
         {
            getLocalFormationNode (i, j, depthIndex)->computeAdjacentNode ();
         }
      }

      RequestHandling::FinishRequestHandling ();

      return true;
   }

   // Refactor this. Too big, too complex
   /// migrate the contents of the charge grid maps to their appropriate columns in the target reservoir
   void MigrationFormation::migrateExpelledChargesToReservoir (unsigned int direction, MigrationReservoir * targetReservoir) const
   {
      if (direction == EXPELLEDNONE)
         return;

      PetscBool genexMinorSnapshots;
      PetscOptionsHasName (PETSC_IGNORE, PETSC_IGNORE, "-genex",  &genexMinorSnapshots);

      if (genexMinorSnapshots and m_genexData != nullptr)
         m_genexData->retrieveData();

      double expulsionFraction = (direction == EXPELLEDUPANDDOWNWARD ? 1.0 : 0.5);

      unsigned int offsets[4][2] = { { 0, 0 }, { 0, 1 }, { 1, 0 }, { 1, 1 } };
      int depthIndex = m_formationNodeArray->depth () - 1;

      RequestHandling::StartRequestHandling (getMigrator (), "migrateExpelledChargesToReservoir");

      double stuckHCMass = 0.0;

      for (unsigned int i = m_formationNodeArray->firstILocal (); i <= m_formationNodeArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_formationNodeArray->firstJLocal (); j <= m_formationNodeArray->lastJLocal (); ++j)
         {
            // Get and check the top SR element at (i,j) location of expulsion
            LocalFormationNode * formationNode = getLocalFormationNode (i, j, depthIndex);
            if (!IsValid (formationNode)) continue;

            // Force expulsion by getting node above source rock, unless it's impermeable
            LocalFormationNode * nodeAboveExpulsion = getLocalFormationNode (i, j, depthIndex + 1);
            if (IsValid(nodeAboveExpulsion) and !nodeAboveExpulsion->isImpermeable())
               formationNode = nodeAboveExpulsion;

            // getTargetFormationNode () uses recursively the node above as long as the current one has no thickness.
            // If at the top node of the basin, it always returns a targetFormationNode regardless of thickness.
            FormationNode *targetFormationNode = formationNode->getTargetFormationNode ();

            // check if targetReservoir is the reservoir to migrate to for given i, j
            if (targetFormationNode and targetFormationNode->isEndOfPath () and
               !targetFormationNode->goesOutOfBounds () and
               targetFormationNode->getFormation () == targetReservoir->getFormation ())
            {
               unsigned int iTarget = targetFormationNode->getI ();
               unsigned int jTarget = targetFormationNode->getJ ();

               // calculate the composition to migrate
               Composition composition;
               for (int componentId = ComponentId::FIRST_COMPONENT; componentId < ComponentId::NUMBER_OF_SPECIES; ++componentId)
               {
                  if (!ComponentsUsed[componentId])
                     continue;

                  double expelledMass = getExpelledMass (i, j, componentId);
                  if (expelledMass != 0.0)
                  {
                     double correctedMass = expelledMass * expulsionFraction;
                     composition.add ((ComponentId)componentId, correctedMass);
                  }
               }

               Column *targetColumn = targetReservoir->getColumn (iTarget, jTarget);

               // In the following lines, a suitable column where the HCs can be put is sought
               if (IsValid (targetColumn) and !targetColumn->isSealing ())
               {
                  targetColumn->addCompositionToBeMigrated (composition);
               }
               else
               {
                  // If the primary column (same i,j as the targetFormationNode) of the element
                  // is sealing look at the other three columns associated with that element.
                  if (targetColumn->isSealing ())
                  {
                     int offsetIndex;
                     Column * shallowestColumn = 0;
                     double shallowestDepth = Interface::DefaultUndefinedScalarValue;
                     for (offsetIndex = 1; offsetIndex < 4; ++offsetIndex)
                     {
                        Column *altTargetColumn = targetReservoir->getColumn (iTarget + offsets[offsetIndex][0],
                                                                           jTarget + offsets[offsetIndex][1]);

                        if (IsValid (altTargetColumn) and !altTargetColumn->isSealing () and (shallowestColumn == 0 || altTargetColumn->getTopDepth () < shallowestDepth))
                        {
                           shallowestColumn = altTargetColumn;
                           shallowestDepth = targetColumn->getTopDepth ();
                        }
                     }
                     if (shallowestColumn)
                        shallowestColumn->addCompositionToBeMigrated (composition);
                     else
                        targetReservoir->addBlocked (composition);
                  }
                  else
                     targetReservoir->addBlocked (composition);
               }
            }
            else if (isValidNodeBelowFormation(targetFormationNode, targetReservoir->getFormation ()))
            {
               calculateStuckHCs (expulsionFraction, i, j, stuckHCMass);
            }
         }
      }

      RequestHandling::FinishRequestHandling ();

      double stuckHCs = SumAll (stuckHCMass);
      if (GetRank () == 0 and stuckHCs > 0.0)
         std::cout << "Basin_Warning: Hydrocarbons expelled from SR " << getName() << " got trapped in undetected/undefined reservoirs. " << stuckHCs << " kg were eliminated.\n";

      if (genexMinorSnapshots and m_genexData != nullptr)
         m_genexData->restoreData();
   }

   // Refactor this. Too big, too complex
   void MigrationFormation::migrateLeakedChargesToReservoir (MigrationReservoir * targetReservoir) const
   {
      // LeakingReservoirList should contain only the reservoir corresponding
      // to the formation whose member function is being executed
      DataAccess::Interface::ReservoirList * leakingReservoirList = m_migrator->getReservoirs (this);
      assert (!leakingReservoirList->empty ());

      // leakingReservoir is the reservoir hosted in "this" formation
      const DataAccess::Interface::Reservoir * dataAccessReservoir = (*leakingReservoirList)[0];

      const MigrationReservoir * leakingReservoir = dynamic_cast<const migration::MigrationReservoir *> (dataAccessReservoir);

      unsigned int offsets[4][2] = { { 0, 0 }, { 0, 1 }, { 1, 0 }, { 1, 1 } };

      int depthIndex = m_formationNodeArray->depth () - 1;

      RequestHandling::StartRequestHandling (getMigrator (), "migrateLeakedChargesToReservoir");

      double stuckHCMass = 0.0;

      for (unsigned int i = m_formationNodeArray->firstILocal (); i <= m_formationNodeArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_formationNodeArray->firstJLocal (); j <= m_formationNodeArray->lastJLocal (); ++j)
         {
            LocalColumn * leakingColumn = leakingReservoir->getLocalColumn (i, j);
            if ( !IsValid(leakingColumn) or leakingReservoir->isOnBoundary((Column*) leakingColumn) ) continue;

            // Make sure the leaking node is a valid one.
            LocalFormationNode * formationNode = getLocalFormationNode (i, j, depthIndex);
            if (!IsValid (formationNode)) continue;

            // The "leaking" node probably has a reservoir flag (e.g. because it's a trap crest) or isEndOfPath.
            // Then the HC path will be forced to be lateral.
            // But we know it should leak so we force it to do so by probing the node right above it.
            formationNode = getLocalFormationNode (i, j, depthIndex + 1);
            if (!IsValid (formationNode)) continue;

            // getTargetFormationNode () uses recursively the node above as long as the current one has no thickness.
            // If at the top node of the basin, it always returns a targetFormationNode regardless of thickness.
            FormationNode *targetFormationNode = formationNode->getTargetFormationNode ();

            // check if targetReservoir is the reservoir to migrate to for given i, j
            if (targetFormationNode and targetFormationNode->isEndOfPath () and
               !targetFormationNode->goesOutOfBounds () and
               targetFormationNode->getFormation () == targetReservoir->getFormation ())
            {
               unsigned int iTarget = targetFormationNode->getI () ;
               unsigned int jTarget = targetFormationNode->getJ () ;

               Composition leakingComposition, composition;
               assert (leakingComposition.isEmpty ());
               assert (composition.isEmpty ());

               leakingComposition = leakingColumn->getComposition ();

               // calculate the composition to migrate
               for (int componentId = ComponentId::FIRST_COMPONENT; componentId < ComponentId::NUMBER_OF_SPECIES; ++componentId)
               {
                  if (!ComponentsUsed[componentId])
                     continue;

                  double weight = leakingComposition.getWeight ((ComponentId)componentId);

                  if (weight)
                  {
                     composition.add ((ComponentId)componentId, weight);
                  }
               }

               if (!composition.isEmpty ())
               {
                  Column *targetColumn = targetReservoir->getColumn (iTarget, jTarget);

                  // In the following lines, a suitable column where the HCs can be put is sought
                  if (IsValid (targetColumn) and !targetColumn->isSealing ())
                  {
                     targetColumn->addCompositionToBeMigrated (composition);
                  }
                  else
                  {
                     // If the primary column (same i,j as the targetFormationNode) of the element
                     // is sealing look at the other three columns associated with that element.
                     if (targetColumn->isSealing ())
                     {
                        int offsetIndex;
                        Column * shallowestColumn = 0;
                        double shallowestDepth = Interface::DefaultUndefinedScalarValue;
                        for (offsetIndex = 1; offsetIndex < 4; ++offsetIndex)
                        {
                           Column *altTargetColumn = targetReservoir->getColumn (iTarget + offsets[offsetIndex][0],
                                                                                 jTarget + offsets[offsetIndex][1]);

                           if (IsValid (altTargetColumn) and !altTargetColumn->isSealing () and (shallowestColumn == 0 || altTargetColumn->getTopDepth () < shallowestDepth))
                           {
                              shallowestColumn = altTargetColumn;
                              shallowestDepth = targetColumn->getTopDepth ();
                           }
                        }
                        if (shallowestColumn)
                           shallowestColumn->addCompositionToBeMigrated (composition);
                        else
                           targetReservoir->addBlocked (composition);
                     }
                     else
                        targetReservoir->addBlocked (composition);
                  }
               }
            }
            else if (isValidNodeBelowFormation(targetFormationNode, targetReservoir->getFormation ()))
            {
               stuckHCMass += leakingColumn->getComposition ().getWeight();
            }
         }
      }

      delete leakingReservoirList;
      RequestHandling::FinishRequestHandling ();

      double stuckHCs = SumAll (stuckHCMass);
      if (GetRank () == 0 and stuckHCs > 0.0)
         std::cout << "Basin_Warning: Hydrocarbons leaked from " << leakingReservoir->getFormation()->getName() << " got trapped in undetected/undefined reservoirs. " << stuckHCs << " kg were elinminated.\n";

   }

   bool MigrationFormation::calculateExpulsionSeeps (const Interface::Snapshot * end, const double expulsionFraction, const bool advancedMigration)
   {
      PetscBool genexMinorSnapshots;
      PetscOptionsHasName (PETSC_IGNORE, PETSC_IGNORE, "-genex",  &genexMinorSnapshots);

      if (genexMinorSnapshots and m_genexData != nullptr)
         m_genexData->retrieveData();

      int depthIndex = m_formationNodeArray->depth () - 1;

      MigrationFormation *topActiveFormation = m_migrator->getTopActiveFormation (end);
      MigrationFormation *formationAbove = getTopFormation ();

      RequestHandling::StartRequestHandling (getMigrator (), "calculateExpulsionSeeps");

      double stuckHCMass = 0.0;

      for (unsigned int i = m_formationNodeArray->firstILocal (); i <= m_formationNodeArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_formationNodeArray->firstJLocal (); j <= m_formationNodeArray->lastJLocal (); ++j)
         {
            // Get and check the top SR element at (i,j) location of expulsion
            LocalFormationNode * formationNode = getLocalFormationNode (i, j, depthIndex);
            if (!IsValid(formationNode)) continue;

            // Force expulsion by getting node above source rock, unless it's impermeable
            LocalFormationNode * nodeAboveExpulsion = getLocalFormationNode (i, j, depthIndex + 1);
            if (IsValid(nodeAboveExpulsion) and !nodeAboveExpulsion->isImpermeable())
               formationNode = nodeAboveExpulsion;

            FormationNode * targetFormationNode;

            if (!advancedMigration)
            {
               int k = topActiveFormation->getNodeDepth () - 1;
               targetFormationNode = topActiveFormation->getFormationNode (i, j, k);
            }
            else
               targetFormationNode = formationNode->getTargetFormationNode ();

            // check if the top formation is the formation to migrate to for given i, j
            if (targetFormationNode and targetFormationNode->isEndOfPath () and
               !targetFormationNode->goesOutOfBounds () and
               targetFormationNode->getFormation () == topActiveFormation)
            {
               // calculate the composition to migrate
               Composition composition;
               for (int componentId = ComponentId::FIRST_COMPONENT; componentId < ComponentId::NUMBER_OF_SPECIES; ++componentId)
               {
                  if (!ComponentsUsed[componentId])
                     continue;

                  double expelledMass = getExpelledMass (i, j, componentId);
                  if (expelledMass != 0.0)
                  {
                     double correctedMass = expelledMass * expulsionFraction;
                     composition.add ((ComponentId)componentId, correctedMass);
                  }
               }
               targetFormationNode->addComposition (composition);
            }
            else if (isValidNodeBelowFormation(targetFormationNode, topActiveFormation))
            {
               calculateStuckHCs (expulsionFraction, i, j, stuckHCMass);
            }
         }
      }

      RequestHandling::FinishRequestHandling ();

      double stuckHCs = SumAll (stuckHCMass);
      if (GetRank () == 0 and stuckHCs > 0.0)
         std::cout << "Basin_Warning: Hydrocarbons expelled from SR " << getName() << " got trapped in undetected/undefined reservoirs. " << stuckHCs << " kg were eliminated.\n";

      if (genexMinorSnapshots and m_genexData != nullptr)
         m_genexData->restoreData();

      return true;
   }

   bool MigrationFormation::calculateLeakageSeeps (const Interface::Snapshot * end, const bool advancedMigration)
   {
      // LeakingReservoirList should contain only the reservoir corresponding
      // to the formation whose member function is being executed
      DataAccess::Interface::ReservoirList * leakingReservoirList = m_migrator->getReservoirs (this);
      assert (!leakingReservoirList->empty ());

      // leakingReservoir is the reservoir hosted in "this" formation
      const DataAccess::Interface::Reservoir * dataAccessReservoir = (*leakingReservoirList)[0];

      const MigrationReservoir * leakingReservoir = dynamic_cast<const migration::MigrationReservoir *> (dataAccessReservoir);

      MigrationFormation *topActiveFormation = m_migrator->getTopActiveFormation (end);
      MigrationFormation *formationAbove = getTopFormation ();

      int depthIndex = m_formationNodeArray->depth () - 1;

      RequestHandling::StartRequestHandling (getMigrator (), "calculateLeakageSeeps");

      double stuckHCMass = 0.0;

      for (unsigned int i = m_formationNodeArray->firstILocal (); i <= m_formationNodeArray->lastILocal (); ++i)
      {
         for (unsigned int j = m_formationNodeArray->firstJLocal (); j <= m_formationNodeArray->lastJLocal (); ++j)
         {
            LocalColumn * leakingColumn = leakingReservoir->getLocalColumn (i, j);
            if (!IsValid (leakingColumn) or leakingReservoir->isOnBoundary ((Column*) leakingColumn)) continue;

            // The "leaking" node probably isEndOfPath (e.g. because it's a trap crest).
            // Then the HC path will be forced to be lateral.
            // But we know it should leak so we force it to do so
            // by probing the first non-zero thickness node  above it.
            LocalFormationNode * formationNode = validSealNode (i, j, formationAbove, topActiveFormation);
            if (!IsValid (formationNode))
            {
               // If that doesn't work, check if there's any zero-thickness elements above
               LocalFormationNode * leakingNode = getLocalFormationNode (i, j, depthIndex);
               formationNode = leakingNode->getTopFormationNode();

               // If that also doesn't work, try the leaking node, it must be at the top.
               if (!IsValid (formationNode))
                  formationNode = leakingNode;
            }

            FormationNode *targetFormationNode;

            if (!advancedMigration)
            {
               int k = topActiveFormation->getNodeDepth () - 1;
               targetFormationNode = topActiveFormation->getFormationNode (i, j, k);
            }
            else
               targetFormationNode = formationNode->getTargetFormationNode ();

            // check if targetReservoir is the reservoir to migrate to for given i, j
            if (targetFormationNode and targetFormationNode->isEndOfPath () and
               !targetFormationNode->goesOutOfBounds () and
               targetFormationNode->getFormation () == topActiveFormation)
            {
               Composition leakingComposition, composition;
               assert (leakingComposition.isEmpty ());
               assert (composition.isEmpty ());

               leakingComposition = leakingColumn->getComposition ();

               // calculate the composition to migrate
               for (int componentId = ComponentId::FIRST_COMPONENT; componentId < ComponentId::NUMBER_OF_SPECIES; ++componentId)
               {
                  if (!ComponentsUsed[componentId])
                     continue;

                  double weight = leakingComposition.getWeight ((ComponentId)componentId);

                  if (weight)
                  {
                     composition.add ((ComponentId)componentId, weight);
                  }
               }

               if (!composition.isEmpty ())
               {
                  targetFormationNode->addComposition (composition);
               }
            }
            else if (isValidNodeBelowFormation(targetFormationNode, topActiveFormation))
            {
               stuckHCMass += leakingColumn->getComposition ().getWeight();
            }
         }
      }

      delete leakingReservoirList;
      RequestHandling::FinishRequestHandling ();

      double stuckHCs = SumAll (stuckHCMass);
      if (GetRank () == 0 and stuckHCs > 0.0)
         std::cout << "Basin_Warning: Hydrocarbons leaked from " << leakingReservoir->getFormation()->getName() << " got trapped in undetected/undefined reservoirs. " << stuckHCs << " kg were elinminated.\n";

      return true;
   }

   bool MigrationFormation::isValidNodeBelowFormation (FormationNode * formationNode, const DataAccess::Interface::Formation * formation) const
   {
      return formationNode and !isOnBoundary(formationNode) and
         (formationNode->getReservoirVapour () or formationNode->getReservoirLiquid ()) and
         formationNode->getFormation () != formation;
   }

   void MigrationFormation::calculateStuckHCs (const double expulsionFraction, const unsigned int i, const unsigned int j, double& stuckHCMass) const
   {
      Composition stuckComposition;
      for (int componentId = ComponentId::FIRST_COMPONENT; componentId < ComponentId::NUMBER_OF_SPECIES; ++componentId)
      {
         if (!ComponentsUsed[componentId])
            continue;

         double expelledMass = getExpelledMass (i, j, componentId);
         if (expelledMass != 0.0)
         {
            double correctedMass = expelledMass * expulsionFraction;
            stuckComposition.add ((ComponentId)componentId, correctedMass);
         }
         stuckHCMass += stuckComposition.getWeight();
      }

      return;
   }

   double MigrationFormation::getExpelledMass (int i, int j, int componentId) const
   {
      if (m_migrator->isGenexRunOnTheFly())
      {
         if (m_genexData == nullptr)
            return 0.0;

         double massPerSurface = m_genexData->getValue ((unsigned int) i, (unsigned int) j, (unsigned int) componentId);

         if (massPerSurface != m_genexData->getUndefinedValue ())
            return massPerSurface * m_genexData->getGrid ()->getSurface(1,1);
         else
            return 0.0;
      }
      else
      {
         if (m_expulsionGridMaps == nullptr or m_expulsionGridMaps[componentId] == nullptr)
            return 0.0;

         double massPerSurface = m_expulsionGridMaps[componentId]->getValue ((unsigned int) i, (unsigned int) j);

         if (massPerSurface != m_expulsionGridMaps[componentId]->getUndefinedValue ())
            return massPerSurface * m_expulsionGridMaps[componentId]->getGrid ()->getSurface(1,1);
         else
            return 0.0;
      }

   }

   bool MigrationFormation::preprocessSourceRock (const double startTime, const bool printDebug)
   {

      const double depoTime = (getTopSurface () ? getTopSurface ()->getSnapshot ()->getTime () : 0);
      bool sourceRockIsActive = (depoTime > startTime) || fabs (depoTime - startTime) < Genex6::Constants::Zero;

      if (sourceRockIsActive)
      {
         if (GetRank () == 0)
         {
            cout << "Preprocessing formation " << getName () << " at the time interval: " << depoTime << " Ma to " << startTime << " Ma ..." << endl;
         }

         if (m_genexData == 0)
         {
            m_genexData = getProjectHandle().getFactory ()->produceGridMap (0, 0, getProjectHandle().getActivityOutputGrid (), Interface::DefaultUndefinedMapValue, ComponentId::NUMBER_OF_SPECIES);
         }
         const GeoPhysics::GeoPhysicsSourceRock * sourceRock = dynamic_cast<const GeoPhysics::GeoPhysicsSourceRock *>(getSourceRock1 ());
         GeoPhysics::GeoPhysicsSourceRock * sourceRock1 = const_cast<GeoPhysics::GeoPhysicsSourceRock *>(sourceRock);

         Interface::SnapshotList * snapshots = getProjectHandle().getSnapshots (Interface::MINOR | Interface::MAJOR);

         // present day map
         FormationPropertyPtr vrProperty = m_migrator->getPropertyManager ().getFormationProperty (m_migrator->getPropertyManager ().getProperty ("Vr"), *(snapshots->begin ()), this);
         const GridMap * gridMapEnd = m_migrator->getPropertyManager ().produceDerivedGridMap (vrProperty);

         if (!gridMapEnd)
         {

            if (GetRank () == 0)
            {
               cout <<"Basin_Warning: "<< getName () << ": Cannot find Vr present day  map" << endl;
            }
            return false;
         }
         m_isInitialised = true;

         sourceRock1->setFormationData (this);
         sourceRock1->initialize (false);
         sourceRock1->preprocess (gridMapEnd, gridMapEnd, false);

         Interface::SnapshotList::reverse_iterator snapshotIter;

         const Interface::Snapshot * start;
         const Interface::Snapshot * end = 0;
         bool status = true;

         for (snapshotIter = snapshots->rbegin (); snapshotIter != snapshots->rend () - 1; ++snapshotIter)
         {

            start = (*snapshotIter);
            end = 0;

            if (start->getTime () > startTime)
            {
               if ((depoTime > start->getTime ()) || (fabs (depoTime - start->getTime ()) < Genex6::Constants::Zero))
               {

                  start = (*snapshotIter);
                  end = *(snapshotIter + 1);

                  status = calculateGenexTimeInterval (start, end, printDebug);
               }
            }
         }

         delete snapshots;

         return status;
      }
      return true;
   }


   bool MigrationFormation::calculateGenexTimeInterval (const Interface::Snapshot * start, const Interface::Snapshot * end, const bool printDebug)
   {

      const double depoTime = (getTopSurface () ? getTopSurface ()->getSnapshot ()->getTime () : 0);
      bool sourceRockIsActive = (depoTime > start->getTime ()) || fabs (depoTime - start->getTime ()) < Genex6::Constants::Zero;

      if (sourceRockIsActive and m_isInitialised)
      {

         bool status = m_isInitialised;

         if (start->getTime () == m_startGenexTime and end->getTime () == m_endGenexTime)
         {
            if (GetRank () == 0 and printDebug)
            {
               cout << getName () << ": Genex interval start = " << start->getTime () << ", end = " << end->getTime () << " is already calculated" << endl;
            }
            return true;
         }

         Genex6::LinearGridInterpolator vesInterp;
         Genex6::LinearGridInterpolator temperatureInterp;
         Genex6::LinearGridInterpolator thicknessInterp;

         Genex6::LinearGridInterpolator hydrostaticPressureInterp;
         Genex6::LinearGridInterpolator lithostaticPressureInterp;
         Genex6::LinearGridInterpolator porePressureInterp;
         Genex6::LinearGridInterpolator porosityInterp;
         Genex6::LinearGridInterpolator permeabilityInterp;
         Genex6::LinearGridInterpolator vreInterp;

         status = extractGenexDataInterval (start, end,
            thicknessInterp,
            vesInterp,
            temperatureInterp,
            hydrostaticPressureInterp,
            lithostaticPressureInterp,
            porePressureInterp,
            porosityInterp,
            permeabilityInterp,
            vreInterp);

         if (!status)
         {
            if (GetRank () == 0)
            {
               cout <<"Basin_Warning: "<< getName () << ": Cannot extract  genex interval start = " << start->getTime () << ", end = " << end->getTime () << endl;
            }

            return false;
         }

         if (GetRank () == 0 and printDebug)
         {
            cout << getName () << ": Calculate genex interval start = " << start->getTime () << ", end = " << end->getTime () << endl;
         }

         const GeoPhysics::GeoPhysicsSourceRock * sourceRock = dynamic_cast<const GeoPhysics::GeoPhysicsSourceRock *>(getSourceRock1 ());
         GeoPhysics::GeoPhysicsSourceRock * sourceRock1 = const_cast<GeoPhysics::GeoPhysicsSourceRock *>(sourceRock);

         sourceRock1->computeTimeInterval (start->getTime (), end->getTime (),
            &vesInterp,
            &temperatureInterp,
            &thicknessInterp,
            &lithostaticPressureInterp,
            &hydrostaticPressureInterp,
            &porePressureInterp,
            &porosityInterp,
            &permeabilityInterp,
            &vreInterp,
            m_genexData);

         m_startGenexTime = start->getTime ();
         m_endGenexTime = end->getTime ();

         return status;
      }

      return true;
   }

   bool MigrationFormation::extractGenexDataInterval (const Snapshot *intervalStart,
      const Snapshot *intervalEnd,
      Genex6::LinearGridInterpolator& thickness,
      Genex6::LinearGridInterpolator& ves,
      Genex6::LinearGridInterpolator& temperature,
      Genex6::LinearGridInterpolator& hydrostaticPressure,
      Genex6::LinearGridInterpolator& lithostaticPressure,
      Genex6::LinearGridInterpolator& porePressure,
      Genex6::LinearGridInterpolator& porosity,
      Genex6::LinearGridInterpolator& permeability,
      Genex6::LinearGridInterpolator& vre)
   {

      const DataModel::AbstractProperty* property = m_migrator->getPropertyManager ().getProperty ("ErosionFactor");
      FormationMapPropertyPtr startProperty = m_migrator->getPropertyManager ().getFormationMapProperty (property, intervalStart, this);
      FormationMapPropertyPtr endProperty = m_migrator->getPropertyManager ().getFormationMapProperty (property, intervalEnd, this);

      bool status = true;
      if (startProperty and endProperty)
      {
         startProperty->retrieveData ();
         endProperty->retrieveData ();

         thickness.compute (intervalStart, startProperty, intervalEnd, endProperty);

         startProperty->restoreData ();
         endProperty->restoreData ();
      }
      else
      {
         if (GetRank () == 0)
         {
            if (!startProperty)
            {
               cout << "Basin_Warning: Property map ErosionFactor " << getTopSurface ()->getName () << " at " << intervalStart->getTime () << " not found" << endl;
            }
            if (!endProperty)
            {
               cout << "Basin_Warning: Property map ErosionFactor " << getTopSurface ()->getName () << " at " << intervalEnd->getTime () << " not found" << endl;
            }
         }
         status = false;
      }

      status = computeInterpolator ("Ves", intervalStart, intervalEnd, ves) and status;
      status = computeInterpolator ("Temperature", intervalStart, intervalEnd, temperature) and status;
      status = computeInterpolator ("HydroStaticPressure", intervalStart, intervalEnd, hydrostaticPressure) and status;
      status = computeInterpolator ("LithoStaticPressure", intervalStart, intervalEnd, lithostaticPressure) and status;
      status = computeInterpolator ("Pressure", intervalStart, intervalEnd, porePressure) and status;

      status = computeInterpolator ("Porosity", intervalStart, intervalEnd, porosity) and status;
      status = computeInterpolator ("Permeability", intervalStart, intervalEnd, permeability) and status;
      status = computeInterpolator ("VrVec2", intervalStart, intervalEnd, vre) and status;

      return status;
   }

   bool MigrationFormation::computeInterpolator (const string & propertyName, const Snapshot *intervalStart, const Snapshot *intervalEnd, Genex6::LinearGridInterpolator& interpolator)
   {

      const DataModel::AbstractProperty* property;

      property = m_migrator->getPropertyManager ().getProperty (propertyName);


      if (property->getPropertyAttribute () == DataModel::CONTINUOUS_3D_PROPERTY)
      {
         SurfacePropertyPtr startSurfaceProperty = m_migrator->getPropertyManager ().getSurfaceProperty (property, intervalStart, getTopSurface ());
         SurfacePropertyPtr endSurfaceProperty = m_migrator->getPropertyManager ().getSurfaceProperty (property, intervalEnd, getTopSurface ());

         startSurfaceProperty->retrieveData ();
         endSurfaceProperty->retrieveData ();

         interpolator.compute (intervalStart, startSurfaceProperty, intervalEnd, endSurfaceProperty);

         startSurfaceProperty->restoreData ();
         endSurfaceProperty->restoreData ();
      }
      else if (property->getPropertyAttribute () == DataModel::DISCONTINUOUS_3D_PROPERTY)
      {
         FormationSurfacePropertyPtr startProperty = m_migrator->getPropertyManager ().getFormationSurfaceProperty (property, intervalStart,
            this, getTopSurface ());
         FormationSurfacePropertyPtr endProperty = m_migrator->getPropertyManager ().getFormationSurfaceProperty (property, intervalEnd,
            this, getTopSurface ());

         if (startProperty and endProperty)
         {
            startProperty->retrieveData ();
            endProperty->retrieveData ();

            interpolator.compute (intervalStart, startProperty, intervalEnd, endProperty);

            startProperty->restoreData ();
            endProperty->restoreData ();
         }
         else
         {
            if (GetRank () == 0)
            {
               if (!startProperty)
               {
                  cout << "Basin_Warning: Property map " << propertyName << " " << getTopSurface ()->getName () << " at " << intervalStart->getTime () << " not found" << endl;
               }
               if (!endProperty)
               {
                  cout << "Basin_Warning: Property map " << propertyName << " " << getTopSurface ()->getName () << " at " << intervalEnd->getTime () << " not found" << endl;
               }
            }
            return false;
         }

      }
      else
      {

         if (GetRank () == 0)
         {
            cout << "Basin_Warning: Property map " << propertyName << " " << getTopSurface ()->getName () << " at " << intervalStart->getTime () << " not found" << endl;
         }
         return false;
      }

      return true;
   }

   double MigrationFormation::getDeltaI (void)
   {
      return getGrid ()->deltaI ();
   }

   double MigrationFormation::getDeltaJ (void)
   {
      return getGrid ()->deltaJ ();
   }

   const Grid * MigrationFormation::getGrid (void) const
   {
      return getProjectHandle().getHighResolutionOutputGrid (); // not to be changed to getActivityOutputGrid ()!!
   }

   void MigrationFormation::manipulateFormationNodeComposition (FormationNodeCompositionRequest & compositionRequest)
   {
      LocalFormationNode * localFormationNode = getLocalFormationNode (compositionRequest.i, compositionRequest.j, compositionRequest.k);

      if (localFormationNode)
         localFormationNode->manipulateComposition (compositionRequest.valueSpec, compositionRequest.phase, compositionRequest.composition);
   }

   void MigrationFormation::getFormationNodeComposition (FormationNodeCompositionRequest & compositionRequest, FormationNodeCompositionRequest & compositionResponse)
   {
      compositionResponse.formationIndex = getIndex ();
      compositionResponse.valueSpec = compositionRequest.valueSpec;

      getLocalFormationNode (compositionRequest.i, compositionRequest.j, compositionRequest.k)->getComposition (compositionRequest.valueSpec,
         compositionRequest.phase,
         compositionResponse.composition);
   }

   const Interface::GridMap * MigrationFormation::getPropertyGridMap (const string & propertyName,
      const Interface::Snapshot * snapshot) const

   {
      return m_migrator->getPropertyGridMap (propertyName, snapshot, 0, this, 0);
   }

}
