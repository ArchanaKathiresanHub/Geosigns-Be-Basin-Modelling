// Copyright (C) 2010-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <iostream>
using namespace std;

#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif
#include <math.h>

#include "FormationNode.h"

#include "Migrator.h"
#include "Formation.h"
#include "Composition.h"

#include "RequestHandling.h"
#include "array.h"
#include "rankings.h"

#include "NumericFunctions.h"
#include "FiniteElementTypes.h"
#include "BasisFunction.h"

#include "FiniteElement.h"

#include "Interface/FluidType.h"

#include "BrooksCorey.h"

using namespace FiniteElementMethod;
using namespace CBMGenerics;
using namespace capillarySealStrength;

// utilities library
#include "ConstantsPhysics.h"
using Utilities::Physics::AccelerationDueToGravity;
#include "ConstantsMathematics.h"
using Utilities::Maths::PaToMegaPa;

#include "LogHandler.h"

//#define USECAPILLARYPRESSUREMAPS 1
//#define GAS_DENSITY_FLOW_DIRECTION

#define OFFSETOFF(attribute) ((int) ((long) (&attribute) - (long) (this)))

namespace migration
{
#ifdef USELOGSTREAM
   extern ofstream logstream;
#endif

   /// constructor
   FormationNode::FormationNode (unsigned int i, unsigned int j, int k, Formation * formation) :
      m_formation (formation), m_iGlobal (i), m_jGlobal (j), m_k (k)
   {
      clearProperties ();
      clearReservoirProperties ();
   }

   /// Destructor
   FormationNode::~FormationNode (void)
   {
   }

   Migrator * FormationNode::getMigrator (void)
   {
      return getFormation ()->getMigrator ();
   }

   Formation * FormationNode::getFormation (int index)
   {
      return getMigrator ()->getFormation (index);
   }

   /// constructor
   ProxyFormationNode::ProxyFormationNode (unsigned int i, unsigned int j, int k, Formation * formation) :
      FormationNode (i, j, k, formation), m_compositionToBeMigrated (0), m_analogFlowDirection (0)
   {
      clearProperties ();
      clearReservoirProperties ();
   }

   /// Destructor
   ProxyFormationNode::~ProxyFormationNode (void)
   {
      if (m_compositionToBeMigrated)
      {
         delete m_compositionToBeMigrated;
         m_compositionToBeMigrated = 0;
      }

      if (m_analogFlowDirection)
      {
         delete m_analogFlowDirection;
         m_analogFlowDirection = 0;
      }
   }

   void ProxyFormationNode::clearCache ()
   {
      m_cachedValues.clearAll ();
   }

   bool ProxyFormationNode::isCached (FormationNodeCacheBit bit) const
   {
      return m_cachedValues.valueIsTrue ((unsigned int)bit);
   }

   void ProxyFormationNode::setCached (FormationNodeCacheBit bit) const
   {
      m_cachedValues.setValue ((unsigned int)bit, true);
   }

   void ProxyFormationNode::resetCached (FormationNodeCacheBit bit) const
   {
      m_cachedValues.setValue ((unsigned int)bit, false);
   }

   bool ProxyFormationNode::computeTargetFormationNode (void)
   {
      return false;
   }

   void ProxyFormationNode::fillFormationNodeRequest (FormationNodeRequest & request, ValueSpec valueSpec)
   {
      request.i = getI ();
      request.j = getJ ();
      request.k = getK ();
      request.formationIndex = m_formation->getIndex ();
      request.valueSpec = valueSpec;
   }


   template <class T>
   T ProxyFormationNode::getScalarValue (ValueSpec valueSpec)
   {
      FormationNodeValueRequest valueRequest;
      FormationNodeValueRequest valueResponse;

      fillFormationNodeRequest (valueRequest, valueSpec);

      RequestHandling::SendFormationNodeValueRequest (valueRequest, valueResponse);

      return static_cast<T>((int)valueResponse.value);
   }

   template <class T>
   T ProxyFormationNode::getCachedScalarValue (ValueSpec valueSpec, FormationNodeCacheBit cacheBit, T & value)
   {
      if (!isCached (cacheBit))
      {
         value = getScalarValue<T> (valueSpec);
         setCached (cacheBit);
      }

      return value;
   }

   //dedicated function for chached double values that does not truncate the decimal part 
   double ProxyFormationNode::getCachedDoubleValue (ValueSpec valueSpec, FormationNodeCacheBit cacheBit, double & value)
   {
      if (!isCached (cacheBit))
      {
         value = getDoubleValue (valueSpec);
         setCached (cacheBit);
      }
      return value;
   }

   double ProxyFormationNode::getDoubleValue (ValueSpec valueSpec)
   {
      FormationNodeValueRequest valueRequest;
      FormationNodeValueRequest valueResponse;

      fillFormationNodeRequest (valueRequest, valueSpec);

      RequestHandling::SendFormationNodeValueRequest (valueRequest, valueResponse);

      return valueResponse.value;
   }

   Composition & ProxyFormationNode::getComposition (void)
   {
      FormationNodeCompositionRequest chargesRequest;
      FormationNodeCompositionRequest chargesResponse;

      chargesRequest.i = getI ();
      chargesRequest.j = getJ ();
      chargesRequest.k = getK ();

      chargesRequest.formationIndex = m_formation->getIndex ();
      chargesRequest.valueSpec = GETCHARGES;

      RequestHandling::SendFormationNodeCompositionRequest (chargesRequest, chargesResponse);
      m_compositionToBeMigrated = &chargesResponse.composition;
      return *m_compositionToBeMigrated;
   }

   void ProxyFormationNode::addComposition (const Composition & composition)
   {
      FormationNodeCompositionRequest chargesRequest;
      FormationNodeCompositionRequest chargesResponse;

      chargesRequest.i = getI ();
      chargesRequest.j = getJ ();
      chargesRequest.k = getK ();

      chargesRequest.formationIndex = m_formation->getIndex ();

      chargesRequest.valueSpec = ADDCOMPOSITIONTOBEMIGRATED;

      chargesRequest.composition = composition;

      RequestHandling::SendFormationNodeCompositionRequest (chargesRequest, chargesResponse);
   }

   bool ProxyFormationNode::isValid (void)
   {
      return getCachedScalarValue<bool> (ISVALID, ISVALIDCACHE, m_isValid);
   }

   bool ProxyFormationNode::isImpermeable (void)
   {
      return getCachedScalarValue<bool> (ISIMPERMEABLE, ISIMPERMEABLECACHE, m_isImpermeable);
   }

   bool ProxyFormationNode::hasNoThickness (void)
   {
      return getCachedScalarValue<bool> (HASNOTHICKNESS, HASNOTHICKNESSCACHE, m_hasNoThickness);
   }

   bool ProxyFormationNode::hasNowhereToGo (void)
   {
      return getCachedScalarValue<bool> (HASNOWHERETOGO, HASNOWHERETOGOCACHE, m_hasNowhereToGo);
   }

   bool ProxyFormationNode::goesOutOfBounds (void)
   {
      return getCachedScalarValue<bool> (GOESOUTOFBOUNDS, GOESOUTOFBOUNDSCACHE, m_goesOutOfBounds);
   }

   bool ProxyFormationNode::getReservoirVapour ()
   {
      return getScalarValue<bool> (ISRESERVOIRVAPOUR);
   }

   bool ProxyFormationNode::getReservoirLiquid ()
   {
      return getScalarValue<bool> (ISRESERVOIRLIQUID);
   }

   bool ProxyFormationNode::isEndOfPath ()
   {
      return getScalarValue<bool> (ISENDOFPATH);
   }

   //use getCachedDoubleValue for doubles 
   double ProxyFormationNode::getDepth ()
   {
      return getCachedDoubleValue (DEPTH, DEPTHCACHE, m_depth);
   }

   double ProxyFormationNode::getHeightVapour ()
   {
      return getCachedDoubleValue (HEIGHTVAPOUR, HEIGHTVAPOURCACHE, m_heightVapour);
   }

   double ProxyFormationNode::getHeightLiquid ()
   {
      return getCachedDoubleValue (HEIGHTLIQUID, HEIGHTLIQUIDCACHE, m_heightLiquid);
   }

   FaultStatus ProxyFormationNode::getFaultStatus ()
   {
      return getScalarValue<FaultStatus> (FAULTSTATUS);
   }

   FormationNode * ProxyFormationNode::getTargetFormationNode (void)
   {
      if (!isCached ((FormationNodeCacheBit)(TARGETFORMATIONNODECACHE)))
      {
         FormationNodeValueRequest valueRequest;
         FormationNodeValueRequest valueResponse;

         fillFormationNodeRequest (valueRequest, TARGETFORMATIONNODE);

         RequestHandling::SendFormationNodeValueRequest (valueRequest, valueResponse);

         if (valueResponse.i >= 0 and valueResponse.j >= 0 and valueResponse.k >= 0) // may not have been calculated yet!!
         {
            m_targetFormationNode = getFormation (valueResponse.formationIndex)->getFormationNode (valueResponse.i, valueResponse.j, valueResponse.k);
            if (m_targetFormationNode)
            {
               setCached ((FormationNodeCacheBit)(TARGETFORMATIONNODECACHE));
            }
         }
      }

      return m_targetFormationNode;
   }

   ThreeVector & ProxyFormationNode::getAnalogFlowDirection (void)
   {
      if (!m_analogFlowDirection) m_analogFlowDirection = new FiniteElementMethod::ThreeVector;
      if (!isCached ((FormationNodeCacheBit)(ANALOGFLOWDIRECTIONCACHE)))
      {
         FormationNodeThreeVectorRequest threeVectorRequest;
         FormationNodeThreeVectorRequest threeVectorResponse;

         fillFormationNodeRequest (threeVectorRequest, ANALOGFLOWDIRECTION);

         RequestHandling::SendFormationNodeThreeVectorRequest (threeVectorRequest, threeVectorResponse);


         (*m_analogFlowDirection) = threeVectorResponse.values;
         setCached ((FormationNodeCacheBit)(ANALOGFLOWDIRECTIONCACHE));
      }

      return (*m_analogFlowDirection);
   }

   ThreeVector ProxyFormationNode::getFiniteElementGrad (PropertyIndex propertyIndex)
   {
      FormationNodeThreeVectorRequest threeVectorRequest;
      FormationNodeThreeVectorRequest threeVectorResponse;

      fillFormationNodeRequest (threeVectorRequest, GETFINITEELEMENTGRAD);

      threeVectorRequest.values[0] = double (propertyIndex);

      RequestHandling::SendFormationNodeThreeVectorRequest (threeVectorRequest, threeVectorResponse);

      return threeVectorResponse.values;
   }

   // New getFiniteElementValue 
   double ProxyFormationNode::getFiniteElementValue (double iOffset, double jOffset, double kOffset, PropertyIndex propertyIndex)
   {
      FormationNodeThreeVectorValueRequest threeVectorValueRequest;
      FormationNodeThreeVectorValueRequest threeVectorValueResponse;

      fillFormationNodeRequest (threeVectorValueRequest, GETFINITEELEMENTVALUE);

      threeVectorValueRequest.values[0] = iOffset;
      threeVectorValueRequest.values[1] = jOffset;
      threeVectorValueRequest.values[2] = kOffset;
      threeVectorValueRequest.value = double (propertyIndex);

      RequestHandling::SendFormationNodeThreeVectorValueRequest (threeVectorValueRequest, threeVectorValueResponse);

      return threeVectorValueResponse.value;
   }

#ifdef REGISTERPROXIES
   void ProxyFormationNode::registerWithLocal (void)
   {
      FormationNodeValueRequest valueRequest;
      FormationNodeValueRequest valueResponse;

      fillFormationNodeRequest (valueRequest, REGISTER);

      valueRequest.value = (double) GetRank ();
      RequestHandling::SendFormationNodeValueRequest (valueRequest, valueResponse);
   }

   void ProxyFormationNode::deregisterWithLocal (void)
   {
      FormationNodeValueRequest valueRequest;
      FormationNodeValueRequest valueResponse;

      fillFormationNodeRequest (valueRequest, DEREGISTER);

      valueRequest.value = (double) GetRank ();
      RequestHandling::SendFormationNodeValueRequest (valueRequest, valueResponse);
   }
#endif

#ifdef USEPROPERTYVALUES
   double ProxyFormationNode::getPropertyValue (int index)
   {
      return -1;
   }
#endif

   /// Constructor
   LocalFormationNode::LocalFormationNode (unsigned int i, unsigned int j, int k, Formation * formation) :
      FormationNode (i, j, k, formation), m_topFormationNode (0), m_targetFormationNode (0), m_selectedDirectionIndex (-1),
      m_depth (Interface::DefaultUndefinedMapValue), m_horizontalPermeability (-1), m_porosity (-1), m_pressure (-1), m_temperature (-1), m_adjacentNodeIndex (0),
      m_entered (false), m_tried (0), m_hasNoThickness (false), m_cosines (0), m_isCrestLiquid (true), m_isCrestVapour (true), m_isEndOfPath (false),
      m_compositionToBeMigrated (0), m_analogFlowDirection (0), m_finiteElementsDepths (0)
   {
      m_verticalPermeability[0] = -1.0;
      m_verticalPermeability[1] = -1.0;

      m_capillaryEntryPressureVapour[0] = 0.0;
      m_capillaryEntryPressureVapour[1] = 0.0;

      m_capillaryEntryPressureLiquid[0] = 0.0;
      m_capillaryEntryPressureLiquid[1] = 0.0;

      m_vapourDensity = -1.0;
      m_liquidDensity = -1.0;
   }

   /// Destructor
   LocalFormationNode::~LocalFormationNode (void)
   {

      if (m_compositionToBeMigrated)
      {
         delete m_compositionToBeMigrated;
         m_compositionToBeMigrated = 0;
      }

      if (m_cosines)
      {
         delete m_cosines;
         m_cosines = 0;
      }

      if (m_analogFlowDirection)
      {
         delete m_analogFlowDirection;
         m_analogFlowDirection = 0;

      }

      if (m_finiteElementsDepths)
      {
         delete[] m_finiteElementsDepths;
         m_finiteElementsDepths = 0;
      }

   }

   bool LocalFormationNode::isValid (void)
   {
      return m_depth != Interface::DefaultUndefinedMapValue;
   }

   void LocalFormationNode::clearReservoirProperties (void)
   {
      m_isReservoirVapour = false;
      m_isReservoirLiquid = false;
      m_isCrestVapour = true;
      m_isCrestLiquid = true;
      m_isEndOfPath = false;

      m_heightLiquid = m_heightVapour = 0;
   }

   void LocalFormationNode::clearProperties (void)
   {
      FormationNode::clearProperties ();
      m_horizontalPermeability = -1.0;

      m_verticalPermeability[0] = -1.0;
      m_verticalPermeability[1] = -1.0;

      m_depth = Interface::DefaultUndefinedMapValue;
      m_porosity = -1.0;
      m_pressure = -1.0;
      m_temperature = -1.0;

      m_capillaryEntryPressureVapour[0] = -1.0;
      m_capillaryEntryPressureLiquid[0] = -1.0;
      m_capillaryEntryPressureVapour[1] = -1.0;
      m_capillaryEntryPressureLiquid[1] = -1.0;

      m_targetFormationNode = 0;
      m_selectedDirectionIndex = -1;
      m_adjacentNodeIndex = 0;
      m_entered = false;
      m_tried = 0;
      m_hasNoThickness = false;

      if (m_compositionToBeMigrated)
      {
         delete m_compositionToBeMigrated;
         m_compositionToBeMigrated = 0;
      }

      if (m_cosines)
      {
         delete m_cosines;
         m_cosines = 0;
      }

      if (m_analogFlowDirection)
      {
         delete m_analogFlowDirection;
         m_analogFlowDirection = 0;

      }

      if (m_finiteElementsDepths)
      {
         delete[] m_finiteElementsDepths;
         m_finiteElementsDepths = 0;
      }

      clearReservoirProperties ();
   }

   void LocalFormationNode::computeProperties (void)
   {
      int i = getI ();
      int j = getJ ();
      int k = getK ();

      if (!m_topFormationNode) m_topFormationNode = m_formation->getLocalFormationNode (i, j, k + 1);

      determineThicknessProperties (); // computes m_hasNoThickness

      if (!m_formation->computeFiniteElement (i, j, k))
      {
         // undefined node, not to be used in flow path calculations
         m_depth = Interface::DefaultUndefinedMapValue;
         return;
      }

      // Replace node depth with depth at the centre of the element
      m_depth = getFiniteElementValue (0.0, 0.0, 0.0, DEPTHPROPERTY);

      const GeoPhysics::FluidType * fluid = (GeoPhysics::FluidType *) (m_formation->getFluidType ());
      m_waterDensity = fluid->density (m_temperature, m_pressure);
   }

   bool LocalFormationNode::performAdvancedMigration (void)
   {
      return getFormation ()->performAdvancedMigration ();
   }

   bool LocalFormationNode::performHDynamicAndCapillary (void)
   {
      return getFormation ()->performHDynamicAndCapillary ();
   }

   double LocalFormationNode::getBlockingPorosity (void)
   {
      return getFormation ()->getBlockingPorosity ();
   }

   double LocalFormationNode::getBlockingPermeability (void)
   {
      return getFormation ()->getBlockingPermeability ();
   }

   void LocalFormationNode::setValue (FormationNodeValueRequest & request)
   {
      switch (request.valueSpec)
      {
#ifdef REGISTERPROXIES
      case REGISTER:
         addProxy ((int) request.value);
         break;
#endif
      default:
         cerr << "ERROR: illegal request: " << request.valueSpec << endl;
         assert (0);
      }
   }

   void LocalFormationNode::getValue (FormationNodeValueRequest & request, FormationNodeValueRequest & response)
   {
      response.valueSpec = request.valueSpec;

      FormationNode * targetFormationNode;

      switch (request.valueSpec)
      {
      case TARGETFORMATIONNODE:
         targetFormationNode = getTargetFormationNode ();
         if (targetFormationNode)
         {
            response.i = targetFormationNode->getI ();
            response.j = targetFormationNode->getJ ();
            response.k = targetFormationNode->getK ();
            response.formationIndex = targetFormationNode->getFormation ()->getIndex ();
         }
         else
         {
            response.i = -1;
            response.j = -1;
            response.k = -1;
            response.formationIndex = -1;
         }
         response.value = 0;
         break;
      case DEPTH:
         response.i = getI ();
         response.j = getJ ();
         response.k = getK ();
         response.formationIndex = getFormation ()->getIndex ();
         response.value = getDepth ();
         break;
      case ISVALID:
         response.i = getI ();
         response.j = getJ ();
         response.k = getK ();
         response.formationIndex = getFormation ()->getIndex ();
         response.value = isValid ();
         break;
      case ISIMPERMEABLE:
         response.i = getI ();
         response.j = getJ ();
         response.k = getK ();
         response.formationIndex = getFormation ()->getIndex ();
         response.value = isImpermeable ();
         break;
      case HASNOWHERETOGO:
         response.i = getI ();
         response.j = getJ ();
         response.k = getK ();
         response.formationIndex = getFormation ()->getIndex ();
         response.value = hasNowhereToGo ();
      case HASNOTHICKNESS:
         response.i = getI ();
         response.j = getJ ();
         response.k = getK ();
         response.formationIndex = getFormation ()->getIndex ();
         response.value = hasNoThickness ();
         break;
      case GOESOUTOFBOUNDS:
         response.i = getI ();
         response.j = getJ ();
         response.k = getK ();
         response.formationIndex = getFormation ()->getIndex ();
         response.value = goesOutOfBounds ();
         break;
      case ISRESERVOIRVAPOUR:
         response.i = getI ();
         response.j = getJ ();
         response.k = getK ();
         response.formationIndex = getFormation ()->getIndex ();
         response.value = getReservoirVapour ();
         break;
      case ISRESERVOIRLIQUID:
         response.i = getI ();
         response.j = getJ ();
         response.k = getK ();
         response.formationIndex = getFormation ()->getIndex ();
         response.value = getReservoirLiquid ();
         break;
      case ISENDOFPATH:
         response.i = getI ();
         response.j = getJ ();
         response.k = getK ();
         response.formationIndex = getFormation ()->getIndex ();
         response.value = isEndOfPath ();
         break;
      case HEIGHTVAPOUR:
         response.i = getI ();
         response.j = getJ ();
         response.k = getK ();
         response.formationIndex = getFormation ()->getIndex ();
         response.value = getHeightVapour ();
         break;
      case HEIGHTLIQUID:
         response.i = getI ();
         response.j = getJ ();
         response.k = getK ();
         response.formationIndex = getFormation ()->getIndex ();
         response.value = getHeightLiquid ();
         break;
      case FAULTSTATUS:
         response.i = getI ();
         response.j = getJ ();
         response.k = getK ();
         response.formationIndex = getFormation ()->getIndex ();
         response.value = getFaultStatus ();
         break;
      default:
         cerr << "ERROR: illegal request: " << request.valueSpec << endl;
         assert (false);
      }
   }

   void LocalFormationNode::manipulateComposition (ValueSpec valueSpec, int phase, Composition & composition)
   {
      switch (valueSpec)
      {
      case ADDCOMPOSITIONTOBEMIGRATED:
         addComposition (composition);
         break;
      default:
         assert (0);
      }
   }

   void LocalFormationNode::getComposition (ValueSpec valueSpec, int phase, Composition & composition)
   {
      switch (valueSpec)
      {
      case GETCHARGES:
         composition = getComposition ();
         break;
      default:
         assert (0);
      }
   }

   migration::Composition & LocalFormationNode::getComposition (void)
   {
      if (!m_compositionToBeMigrated) m_compositionToBeMigrated = new Composition;
      return *m_compositionToBeMigrated;
   }

   void LocalFormationNode::getThreeVectorValue (FormationNodeThreeVectorValueRequest & request, FormationNodeThreeVectorValueRequest & response)
   {
      switch (request.valueSpec)
      {
      case GETFINITEELEMENTVALUE:
         response.value = getFiniteElementValue (request.values[0], request.values[1], request.values[2], (PropertyIndex)(int)request.value);
         break;
      default:
         cerr << "ERROR: illegal request: " << request.valueSpec << endl;
         assert (0);
      }
   }

   void LocalFormationNode::getThreeVector (FormationNodeThreeVectorRequest & request, FormationNodeThreeVectorRequest & response)
   {
      response.valueSpec = request.valueSpec;

      switch (request.valueSpec)
      {
      case ANALOGFLOWDIRECTION:
         response.i = getI ();
         response.j = getJ ();
         response.k = getK ();
         response.values[0] = (*m_analogFlowDirection)  (1);
         response.values[1] = (*m_analogFlowDirection) (2);
         response.values[2] = (*m_analogFlowDirection) (3);
         break;
      case GETFINITEELEMENTGRAD:
      {
         response.i = getI ();
         response.j = getJ ();
         response.k = getK ();
         response.formationIndex = getFormation ()->getIndex ();
         ThreeVector grad = getFiniteElementGrad ((PropertyIndex)(int)(request.values[0]));
         response.values[0] = grad (1);
         response.values[1] = grad (2);
         response.values[2] = grad (3);
      }
      break;
      default:
         cerr << "ERROR: illegal request: " << request.valueSpec << endl;
         assert (false);
      }
   }

#ifdef REGISTERPROXIES
   void LocalFormationNode::addProxy (int rank)
   {
      m_proxies.push_back (rank);
   }

   void LocalFormationNode::deleteProxy (int rank)
   {
      for (vector<int>::iterator proxyIter = m_proxies.begin (); proxyIter != m_proxies.end (); ++proxyIter)
      {
         if (*proxyIter == rank)
         {
            m_proxies.erase (proxyIter);
            return;
         }
      }
   }

   void LocalFormationNode::resetProxy (int rank)
   {
      FormationNodeValueRequest valueRequest;

      valueRequest.i = getI ();
      valueRequest.j = getJ ();
      valueRequest.k = getK ();
      valueRequest.valueSpec = RESETPROXY;
      RequestHandling::SendProxyFormationNodeReset (rank, valueRequest);
   }

   void LocalFormationNode::resetProxies (void)
   {
      for (vector<int>::iterator proxyIter = m_proxies.begin (); proxyIter != m_proxies.end (); ++proxyIter)
      {
         resetProxy (* proxyIter);
      }
   }
#endif

   void LocalFormationNode::addChargeToBeMigrated (int componentId, double weight)
   {
      if (!m_compositionToBeMigrated) m_compositionToBeMigrated = new Composition;
      m_compositionToBeMigrated->add ((ComponentId)componentId, weight);
   }

   void LocalFormationNode::addComposition (const Composition & composition)
   {
      if (!m_compositionToBeMigrated) m_compositionToBeMigrated = new Composition;
      m_compositionToBeMigrated->add (composition);
   }

   //
   // Check change of capillary pressure across boundary to determine potential reservoir
   //
   bool LocalFormationNode::detectReservoir (LocalFormationNode * topNode,
      const double minLiquidColumnHeight, const double minVapourColumnHeight, const bool pressureRun)
   {
      if (!IsValid (this)) return true;

      double pressureContrastVapour, pressureContrastLiquid;

      bool vapourFlag = false;
      bool liquidFlag = false;

      m_heightLiquid = m_heightVapour = 0;

      if (hasThickness () and !isImpermeable ())
      {
         if (topNode->isImpermeable ())
         {
            vapourFlag = true;
            liquidFlag = true;
            setReservoirVapour (vapourFlag);
            setReservoirLiquid (liquidFlag);

            return true;
         }

         // calculate the density contrast
         double densityContrast = m_waterDensity - m_vapourDensity;
         // get the vapour pressure contrast 
         pressureContrastVapour = getPressureContrast( topNode, GAS, pressureRun );

         // Positive pressure contrast and negative density contrast
         // implies downward force, i.e. reservoir node
         if (pressureContrastVapour > 0.0 and densityContrast <= 0.0)
            vapourFlag = true;
         else
         {
            // calculate maximum height of the hydrocarbons column for gas
            m_heightVapour = pressureContrastVapour / ( densityContrast * AccelerationDueToGravity );

            // if actual height is greater than the user-defined minimum - raise potential reservoir flag
            if (m_heightVapour > minVapourColumnHeight)
            {
               vapourFlag = true;
            }
         }

         // re-calculate the density contrast
         densityContrast = m_waterDensity - m_liquidDensity;
         // get the liquid pressure contrast 
         pressureContrastLiquid = getPressureContrast( topNode, OIL, pressureRun );

         // Positive pressure contrast and negative density contrast
         // implies downward force, i.e. reservoir node
         if (pressureContrastLiquid > 0.0 and densityContrast <= 0.0)
            liquidFlag = true;
         else
         {
            // calculate maximum height of the hydrocarbons column for oil
            m_heightLiquid = pressureContrastLiquid / ( densityContrast * AccelerationDueToGravity );

            // if actual height is greater than the user-defined minimum - raise potential reservoir flag
            if ( m_heightLiquid > minLiquidColumnHeight )
            {
               liquidFlag = true;
            }
         }
      }

      setReservoirVapour (vapourFlag);
      setReservoirLiquid (liquidFlag);

      return (vapourFlag or liquidFlag);
   }


   //
   // Calculate the pressure contrast between the seal and the reservoir formation, including the capillary pressure and the overpressure
   //
   double LocalFormationNode::getPressureContrast( const LocalFormationNode * topNode, const PhaseId phase, const bool pressureRun ) const
   {
      double capillaryPressureContrast;
      // correction factor for capillary pressure in the reservoir: assume 30% water saturation
      double lambdaPC = m_formation->getCompoundLithology( getI( ), getJ( ) )->LambdaPc( );
      // If the project file does not contain values for Lambda_Pc assign an 'avarage' value of 1.
      if ( lambdaPC == Interface::DefaultUndefinedMapValue or lambdaPC == Interface::DefaultUndefinedScalarValue )
         lambdaPC = 1.0;

      GeoPhysics::BrooksCorey brooksCorey;
      double resCorr = brooksCorey.computeBrooksCoreyCorrection( 0.3, lambdaPC );

      // calculate overpressure difference
      double dOverPressure;
      if ( pressureRun )
         dOverPressure = (topNode->m_overPressure - m_overPressure) * Utilities::Maths::MegaPaToPa;
      else
         dOverPressure = 0.0;

      if ( phase == GAS )
      {
         // An array of capillary-pressure values is only calculated for the top element.
         // But here 'this' may actually be an element below the top one, in case of erosion etc.
         double capillaryEntryPressureVapour = ( getK() == (getFormation()->getMaximumNumberOfElements() - 1) ) ? 
            m_capillaryEntryPressureVapour[1] : m_topFormationNode->m_capillaryEntryPressureVapour[0];
         
         // calculate actual capillary sealing pressure for vapour
         capillaryPressureContrast = topNode->m_capillaryEntryPressureVapour[0] - capillaryEntryPressureVapour * resCorr;
      }
      else
      {
         // An array of capillary-pressure values is only calculated for the top element.
         // But here 'this' may actually be an element below the top one, in case of erosion etc.
         double capillaryEntryPressureLiquid = ( getK() == (getFormation()->getMaximumNumberOfElements() - 1) ) ? 
            m_capillaryEntryPressureLiquid[1] : m_topFormationNode->m_capillaryEntryPressureLiquid[0];

         // calculate actual capillary sealing pressure  for liquid
         capillaryPressureContrast = topNode->m_capillaryEntryPressureLiquid[0] - capillaryEntryPressureLiquid * resCorr;
      }

      return capillaryPressureContrast + dOverPressure;
   }

   // Check if the node is a crest node for the phaseId, similarly to what is done in Reservoir::getAdjacentColumn
   bool LocalFormationNode::detectReservoirCrests (PhaseId phase)
   {
      // if the node can not gas or oil, skip the calculations
      if (phase == GAS and !getReservoirVapour ())
      {
         m_isCrestVapour = false;
         return false;
      }
      if (phase == OIL and !getReservoirLiquid ())
      {
         m_isCrestLiquid = false;
         return false;
      }

      // if node is not valid, skip the calculations
      if (!IsValid (this))
      {
         if (phase == GAS)
            m_isCrestVapour = false;
         else
            m_isCrestLiquid = false;
         return false;
      }

      //Calculations performed as in Reservoir::getAdjacentColumn. m_isCrestGas and m_isCrestOil are set true in the constructor. 
      //if the neighbours nodes are zero thickness nodes does not matter, the highest depth is always taken
      int top = m_formation->getNodeDepth () - 1;

      for (int n = 0; n < NumNeighbours; ++n)
      {
         int neighbourI = getI () + NeighbourOffsets2D[n][I];
         int neighbourJ = getJ () + NeighbourOffsets2D[n][J];
         FormationNode * neighbourNode = m_formation->getFormationNode (neighbourI, neighbourJ, top);

         if (!IsValid (neighbourNode))
         {
            // If the neighbourNode object (i.e. element) is invalid we are checking whether the top node
            // corresponding to that element is valid and deeper than that of the candidate crest element
            if (m_formation->isShallowerThanNeighbour(this, neighbourI, neighbourJ))
            {
               continue;
            }
            else
            {
               // neighbour corner node either undefined (genuine edge case)
               // or at a smaller depth than corner node of 'this' element
               if (phase == GAS)
                  m_isCrestVapour = false;
               else
                  m_isCrestLiquid = false;
               return false;
            }
         }
         
         // Account for pinch-out traps
         if (neighbourNode->hasNoThickness())
            continue;

         bool isSealingNeighbourNode = false;

         if (phase == GAS)
            isSealingNeighbourNode = (neighbourNode->getFaultStatus () == SEAL);
         else
            isSealingNeighbourNode = (neighbourNode->getFaultStatus () == SEAL or neighbourNode->getFaultStatus () == SEALOIL);

         if (isSealingNeighbourNode)
         {
            // a SealingNeighbourNode cannot be a trap crest 
            continue;
         }

         if (isShallowerThan (neighbourNode))
         {
            // neighbour is deeper
            continue;
         }
         else if (isDeeperThan (neighbourNode))
         {
            // neighbour is shallower 
            if (phase == GAS)
               m_isCrestVapour = false;
            else
               m_isCrestLiquid = false;
            return false;
         }
      }

      // return true if the node is crest column AND can hold gas or oil 
      if (phase == GAS)
         return (m_isCrestVapour and getReservoirVapour ());
      else
         return (m_isCrestLiquid and getReservoirLiquid ());

   };

   /// compareDepths returns -1 if this is shallower, 0 if equally deep and 1 if this is deeper
   /// Also used to break the tie between columns with equal top depth
   /// Does not take into account whether columns are sealing or wasting
   int FormationNode::compareDepths (FormationNode * node, bool useTieBreaker)
   {
      // top depth of the current node and neighbour node 
      Formation * formation = getFormation ();
      double depth = formation->getPropertyValue (DEPTHPROPERTY, getI (), getJ (), getK () + 1);
      double nodedepth = formation->getPropertyValue (DEPTHPROPERTY, node->getI (), node->getJ (), getK () + 1);

      if (depth < nodedepth) return -1;
      if (depth > nodedepth) return 1;

      if (useTieBreaker)
      {
         if (getI () + getJ () > node->getI () + node->getJ ()) return -1;
         if (getI () + getJ () < node->getI () + node->getJ ()) return 1;

         if (getI () > node->getI ()) return -1;
         if (getI () < node->getI ()) return 1;
         if (getJ () > node->getJ ()) return -1;
         if (getJ () < node->getJ ()) return 1;

         assert (this == node);
      }

      return 0;
   }

   bool LocalFormationNode::getIsCrest (PhaseId phase)
   {
      if (phase == GAS)
         return m_isCrestVapour;
      else
         return m_isCrestLiquid;
   };

   void LocalFormationNode::identifyAsReservoir (const bool advancedMigration)
   {
      if (!advancedMigration)
      {
         setReservoirVapour (true);
         setReservoirLiquid (true);
      }

      setEndOfPath ();

      return;
   }

   /// Compute the permeability anisotropy based flow direction for this node
   /// In the given coordinate system down is positive so gravity force is positive.
   /// All other forces follow straight from the Darcy equation
   void LocalFormationNode::computeAnalogFlowDirection (void)
   {
      if (!IsValid (this)) return;

      int i = getI ();
      int j = getJ ();
      int k = getK ();
      
      if (!performAdvancedMigration () or hasNoThickness ())
      {
         // let's assume everything will go straight up in this case
         if (!m_analogFlowDirection) m_analogFlowDirection = new FiniteElementMethod::ThreeVector;
         (*m_analogFlowDirection).zero ();
         (*m_analogFlowDirection) (3) = -1.0;

         return;
      }

      Matrix3x3 permeabilityTensor;

      FiniteElement finiteElement;

      setFiniteElement (finiteElement);

      // setTensor does not modify any private member of finiteElement
      finiteElement.setTensor (1.0, getHorizontalPermeability () / getVerticalPermeability (), permeabilityTensor);

      ThreeVector capPressureGrad;

#ifdef GAS_DENSITY_FLOW_DIRECTION
      capPressureGrad = getFiniteElementGrad (CAPILLARYENTRYPRESSUREVAPOURPROPERTY);
#else
      capPressureGrad = getFiniteElementGrad (CAPILLARYENTRYPRESSURELIQUIDPROPERTY);
#endif

      if (!performHDynamicAndCapillary () or capPressureGrad (1) == Interface::DefaultUndefinedMapValue)
      {

         ThreeVector vertical;

         vertical.zero ();
         vertical (3) = -1.0;
         if (!m_analogFlowDirection) m_analogFlowDirection = new FiniteElementMethod::ThreeVector;
         matrixVectorProduct (permeabilityTensor, vertical, *m_analogFlowDirection);
      }

      else
      {
         ThreeVector pressureGrad = getFiniteElementGrad (PRESSUREPROPERTY);
         pressureGrad += capPressureGrad * PaToMegaPa;

#ifdef GAS_DENSITY_FLOW_DIRECTION
         double hc_density = m_vapourDensity;
#else
         double hc_density = m_liquidDensity;
#endif
         double gravity = hc_density * AccelerationDueToGravity * PaToMegaPa;

         ThreeVector force;
         force (1) = -pressureGrad (1);
         force (2) = -pressureGrad (2);
         force (3) = -pressureGrad (3) + gravity;
         if (!m_analogFlowDirection) m_analogFlowDirection = new FiniteElementMethod::ThreeVector;
         matrixVectorProduct (permeabilityTensor, force, *m_analogFlowDirection);
      }

      normalise (*m_analogFlowDirection);
   }

   void LocalFormationNode::determineThicknessProperties (void)
   {
      int i = getI ();
      int j = getJ ();
      int k = getK ();

      m_hasNoThickness = true;

      int boi, toi;
      for (toi = FirstTopNodeCorner, boi = FirstBottomNodeCorner; toi <= LastTopNodeCorner /* and boi <= LastBottomNodeCorner */; ++toi, ++boi)
      {
         double topDepth = m_formation->getPropertyValue (DEPTHPROPERTY, i + NodeCornerOffsets[toi][0], j + NodeCornerOffsets[toi][1], k + NodeCornerOffsets[toi][2]);
         double bottomDepth = m_formation->getPropertyValue (DEPTHPROPERTY, i + NodeCornerOffsets[boi][0], j + NodeCornerOffsets[boi][1], k + NodeCornerOffsets[boi][2]);

         if (bottomDepth != topDepth)
         {
            m_hasNoThickness = false;
            break;
         }
      }
   }

   void LocalFormationNode::computeNextAdjacentNode (void)
   {
      FormationNode * adjacentNode = getAdjacentFormationNode (m_selectedDirectionIndex);

      if (hasThickness ())
      {
         ++m_adjacentNodeIndex;
         computeAdjacentNode ();
      }
      else if (m_topFormationNode)
         m_topFormationNode->computeNextAdjacentNode ();
      else
         assert (false);

   }

   typedef pair<int, double> IntDoublePair;

   bool IntDoublePairSorter (const IntDoublePair & pair1, const IntDoublePair & pair2)
   {
      return pair1.second > pair2.second;
   }

   void LocalFormationNode::computeAdjacentNode (void)
   {
      if (!IsValid (this))
      {
         // Set target formation node to "this" for invalid nodes
         prescribeTargetFormationNode();
         return;
      }

      // compare against ... nearest neighbour directions gridwise above and sideways

      if (hasNoThickness () or getFaultStatus () == WASTE)
      {
         // it should just bypass this node upward as it either has no thickness or is a wasting node.
         m_selectedDirectionIndex = 0;
         return;
      }

      // If at a reservoir node, don't compute cosines, follow the path of steepest ascent.
      if (getReservoirVapour () or getReservoirLiquid ())
      {
         int di = 0;
         FormationNode * adjacentFormationNode = getLateralAdjacentNode(di);
         if (adjacentFormationNode)
         {
            // Setting the m_selectedDirectionIndex defines the adjacentNode
            // which is then used to compute the targetFormationNode.
            m_selectedDirectionIndex = di;
         }
         // If no suitable lateral adjacent node is found, path ends here.
         else
         {
            m_selectedDirectionIndex = -1;
            m_targetFormationNode = this;
         }
         return;
      }

      if (!m_cosines or m_cosines->empty ())
         computeCosines ();

      if ((unsigned int)m_adjacentNodeIndex < (*m_cosines).size ())
      {
         m_selectedDirectionIndex = (*m_cosines)[m_adjacentNodeIndex].first;
      }
      else
      {
         // No useful discretized flow direction found, the flow path stops here.
         m_selectedDirectionIndex = -1;
      }
   }

   /// compute the cosines of the angles between the analog flow direction and the possible discretized flow directions and
   /// sort the possible discretized flow directions accordingly.
   void LocalFormationNode::computeCosines (void)
   {
      assert (IsValid (this));
      assert (hasThickness ());

      const int i = getI ();
      const int j = getJ ();
      const int k = getK ();

      int diStart = 0;

      double dx = m_formation->getDeltaI ();
      double dy = m_formation->getDeltaJ ();

      double dxSquare = dx * dx;
      double dySquare = dy * dy;

      if (!m_cosines)
      {
         m_cosines = new vector < IntDoublePair >;
      }

      ThreeVector analogFDnode = getAnalogFlowDirection ();

      for (int di = diStart; di < NumberOfNeighbourOffsetsUsed; ++di)
      {
         FormationNode * neighbourNode = getAdjacentFormationNode (di);

         if (neighbourNode and neighbourNode->isImpermeable ())
         {
            // path cannot go into an impermeable node
            continue;
         }

         ThreeVector discretizedFlowDirection;
         ThreeVector normalizedDiscretizedFlowDirection;

         int iOffset = NeighbourOffsets3D[di][0];
         int jOffset = NeighbourOffsets3D[di][1];
         int kOffset = NeighbourOffsets3D[di][2];

         int iNeighbour = i + iOffset;
         int jNeighbour = j + jOffset;
         int kNeighbour = k + kOffset;

         // calculate discretizedFlowDirection

         discretizedFlowDirection.zero ();
         discretizedFlowDirection (1) = iOffset * dx;
         discretizedFlowDirection (2) = jOffset * dy;

         // calculate discretizedFlowDirection (3)

         double iQP, jQP, kQP;     // Quadrature point coordinates

         // Depth of neighbouring element calculated as the average of its 8 vertices' depth
         double neighbourNodeDepth;
         if (IsValid (neighbourNode))
         {
            neighbourNodeDepth = neighbourNode->getDepth ();

            // Avoid loops in the path by going only upwards in depth
            if (neighbourNodeDepth >= m_depth)
               continue;

            // Avoid going to a reservoir node ("this" cannot be reservoir)
            // where from charge can (steepest) ascend back to "this".
            if ( (neighbourNode->getReservoirVapour() or neighbourNode->getReservoirLiquid()) and
                 isShallowerThan(neighbourNode,false))
               continue;
         }
         else
         {
            // Neighbour is invalid, we move to the vertex/edge or face of the current element,
            // depending on whether all 3, 2, or 1 of i, j and k have an offset.
            // Delta x and Delta y will be half of what they would be if moving by a full element.
            discretizedFlowDirection (1) /= 2;
            discretizedFlowDirection (2) /= 2;

            iQP = double (iOffset);
            jQP = double (jOffset);
            kQP = double (-kOffset);

            // May not be an actual node whose depth we are using but an average of a face or edge.
            neighbourNodeDepth = getFiniteElementValue (iQP, jQP, kQP, DEPTHPROPERTY);
         }

         discretizedFlowDirection (3) = neighbourNodeDepth - m_depth;

         normalizedDiscretizedFlowDirection = discretizedFlowDirection;
         normalise (normalizedDiscretizedFlowDirection);

         // compute the compound analog flow direction of this node and the neighbour node.

         ThreeVector compoundAnalogFlowDirection;

         // If the neighbouring node is valid we will use the analog flow direction of the current AND
         // the neighbour. The weighting factors will be the relative distances that HCs cover in each
         // of the two elements involved.
         if (IsValid (neighbourNode))
         {
            iQP = double (-iOffset);
            jQP = double (-jOffset);
            kQP = double (kOffset); // Notice how z does the opposite of the others.

            // Depth of origin of flow vector (iOffset,jOffset,kOffset) inside the current node.
            // May be depth of vertex, or averaged depth along an edge or over a face.
            double myDepth = getFiniteElementValue (iQP, jQP, kQP, DEPTHPROPERTY);

            assert (myDepth != Interface::DefaultUndefinedMapValue);

            iQP = double (iOffset);
            jQP = double (jOffset);
            kQP = double (-kOffset);

            // Depth of entry point of the flow vector in the neighbouring element.
            // May be depth of vertex, or averaged depth along an edge or over a face.
            double neighbourDepth = getFiniteElementValue (iQP, jQP, kQP, DEPTHPROPERTY);

            assert (neighbourDepth != Interface::DefaultUndefinedMapValue);

            iQP = double (iOffset);
            jQP = double (jOffset);
            kQP = double (-kOffset);

            // Exit point of the flow vector from the neighbouring element.
            // Can also be seen as the vertex/edge or face of the neighbour
            // of the neighbour if we keep on moving in the same direction.

            double neighbourNeighbourDepth = m_formation->getFiniteElementValue (iNeighbour, jNeighbour, kNeighbour, iQP, jQP, kQP, DEPTHPROPERTY);
            if (neighbourNeighbourDepth == Interface::DefaultUndefinedMapValue) continue;

            // diagonals are not always diagonals
            double diagonalSquared = (Abs (NeighbourOffsets3D[di][0]) * dxSquare +
               Abs (NeighbourOffsets3D[di][1]) * dySquare + Square (neighbourDepth - myDepth));
            double diagonal = sqrt (diagonalSquared);

            double neighbourDiagonalSquared = (Abs (NeighbourOffsets3D[di][0]) * dxSquare +
               Abs (NeighbourOffsets3D[di][1]) * dySquare + Square (neighbourNeighbourDepth - neighbourDepth));
            double neighbourDiagonal = sqrt (neighbourDiagonalSquared);

            // get the analog flow direction of the neighbour node.
            ThreeVector neighbourAnalogFlowDirection = m_formation->getAnalogFlowDirection (iNeighbour, jNeighbour, kNeighbour);

            /// compound analog flow direction is weighted sum of this node's analog flow direction of (i, j, k) and neighbour node's analog flow direction
            compoundAnalogFlowDirection = (*m_analogFlowDirection) * diagonal + neighbourAnalogFlowDirection * neighbourDiagonal;

            normalise (compoundAnalogFlowDirection);
         }
         else
         {
            // neighbour node is undefined or invalid, compound analog flow direction equals analog flow direction of current element.
            compoundAnalogFlowDirection = *m_analogFlowDirection;
         }

         double cosine = innerProduct (compoundAnalogFlowDirection, normalizedDiscretizedFlowDirection);

         (*m_cosines).push_back (IntDoublePair (di, cosine));
      }

      // the larger the cosine, the smaller the angle between the compund analog flow direction and the discretized flow direction
      sort ((*m_cosines).begin (), (*m_cosines).end (), IntDoublePairSorter);

   }

   /// @todo Simplify
   bool LocalFormationNode::computeTargetFormationNode (void)
   {
      if (!IsValid (this))
      {
         return true;
      }

      static int MaxTries = -1;

      if (m_isEndOfPath)
      {
         m_targetFormationNode = this;
         return true;
      }

      if (hasNoThickness ())
      {
         if (IsValid(m_topFormationNode))
            m_targetFormationNode = m_topFormationNode->getTargetFormationNode();
         else
            m_targetFormationNode = this;
         return true;
      }

      if (m_targetFormationNode) // already found
         return true;

      // m_tried > MaxTries used to be a condition for getting in the "if" block that follows.
      // Not anymore. Results need to be the same for runs with different numbers of processors.
      //if (MaxTries < 0)
      //   MaxTries = Max (2, NumProcessors () + 8);

      // We will call computeNextAdjacentNode() only if there is a closed loop in the path.
      if (m_entered)
      {
         LogHandler (LogHandler::WARNING_SEVERITY) << "A loop in the flowlines calculation was found. " <<
            "Adjacent node will be changed for node (" << getI() << "," << getJ() << "," << getK() << ") of formation " << m_formation->getName();
         m_tried = 0;
         computeNextAdjacentNode (); // selects next adjacent formation node; changes output of getAdjacentFormationNode ()
         return false;
      }

      if (hasNowhereToGo())
      {
         // If Hcs are trapped in what would otherwise be a detected reservoir
         if (isPartOfUndetectedReservoir())
            dealWithStuckHydrocarbons();
         // No useful target node. Hcs are eliminated
         else
         {
            m_selectedDirectionIndex = -1;
            m_targetFormationNode = this;
         }
      }
      else
      {
         FormationNode * adjacentFormationNode = getAdjacentFormationNode ();
         assert (adjacentFormationNode);

         m_entered = true; // allows us to check for re-entrancy

         adjacentFormationNode->computeTargetFormationNode ();
         m_targetFormationNode = adjacentFormationNode->getTargetFormationNode ();

         m_entered = false;
      }

      return (m_targetFormationNode != 0);
   }

   bool LocalFormationNode::isPartOfUndetectedReservoir()
   {
      // Check whether the node has the reservoir flag and its formation is a reservoir
      bool isReservoirNode         = (m_isReservoirLiquid or m_isReservoirVapour);
      bool formationIsNotReservoir = m_formation->getMigrator()->getReservoirs(m_formation)->empty();

      bool isPartOfUndetectedReservoir = isReservoirNode and formationIsNotReservoir;

      return isPartOfUndetectedReservoir;
   }

   void LocalFormationNode::dealWithStuckHydrocarbons ()
   {
      bool eliminateHCs = EliminateStuckHCs;

      // If at the crest of a trap that we want to remain undetected, then just funnel all HCs through that crest or eliminate them
      if (undetectedCrest())
      {
         if (m_topFormationNode and !m_topFormationNode->isImpermeable())
         {
            if (eliminateHCs)
            {
               m_selectedDirectionIndex = -1;
               m_targetFormationNode = this;
            }
            else
            {
               m_selectedDirectionIndex = 0;
               m_topFormationNode->computeTargetFormationNode();
               m_targetFormationNode = m_topFormationNode->getTargetFormationNode();
            }
         }
         else
         {
            m_selectedDirectionIndex = -1;
            m_targetFormationNode = this;
         }
      }
      else
      {
         int di = 0;
         FormationNode * adjacentFormationNode = getLateralAdjacentNode(di);
         // If an adjacent node is found use it.
         if (adjacentFormationNode)
         {
            // As above, setting the m_selectedDirectionIndex defines the adjacentNode
            // which is then used to compute the targetFormationNode.
            m_selectedDirectionIndex = di;
            adjacentFormationNode->computeTargetFormationNode();
            m_targetFormationNode = adjacentFormationNode->getTargetFormationNode();
         }
         // Otherwise the path ends here
         else
         {
            m_selectedDirectionIndex = -1;
            m_targetFormationNode = this;
         }
      }
      return;
   }

   bool LocalFormationNode::undetectedCrest()
   {
      // Can't be a crest if on boundary
      if (m_formation->isOnBoundary(this))
         return false;

      bool onlyDeeperNodes = true;
      for (int di = NumberOfUpwardNeighbourOffsets; di < NumberOfNeighbourOffsetsUsed; ++di)
      {
         FormationNode * neighbourNode = getAdjacentFormationNode(di);

         if (neighbourNode and neighbourNode->isImpermeable())
         {
            // path cannot go into an impermeable node
            continue;
         }

         if (IsValid(neighbourNode))
         {
            // Avoid loops in the path by going only upwards in depth
            if (compareDepths(neighbourNode, true) == 1)
            {
               onlyDeeperNodes = false;
               break;
            }
         }
      }

      return onlyDeeperNodes;
   }

   // Replicates the behaviour of columns. Uses depth of top left corner node
   // to calculate the path of steepest ascent. If at the boundary of a valid
   // region, then all HCs are eliminated, similar to outward leakage.
   FormationNode * LocalFormationNode::getLateralAdjacentNode(int & di)
   {
      int diToReturn = di;
      double minGradient = 0.0;
      FormationNode * nodeToReturn = nullptr;
      for (di = NumberOfUpwardNeighbourOffsets; di < NumberOfNeighbourOffsetsUsed; ++di)
      {
         FormationNode * neighbourNode = getAdjacentFormationNode(di);

         if (!IsValid(neighbourNode))
         {
            // As in the case of columns in reservoirs, if we are at the edge of the valid grid,
            // don't bother, charge in this node stays here and eventually gets eliminated
            nodeToReturn = nullptr;
            break;
         }

         if (neighbourNode and neighbourNode->isImpermeable())
         {
            // path cannot go into an impermeable node
            continue;
         }

         double dx = m_formation->getDeltaI ();
         double dy = m_formation->getDeltaJ ();

         double depth = m_formation->getPropertyValue (DEPTHPROPERTY, getI (), getJ (), getK () + 1);
         double neighbourDepth = m_formation->getPropertyValue (DEPTHPROPERTY, neighbourNode->getI (), neighbourNode->getJ (), getK () + 1);

         if (isShallowerThan (neighbourNode))
         {
            // neighbour is deeper
            continue;
         }
         else if (isDeeperThan (neighbourNode, false))
         {
            // neighbour is shallower in absolute terms
            double gradient = (neighbourDepth - depth) / std::sqrt(std::pow(dx * std::abs(NeighbourOffsets3D[di][0]), 2.0) + std::pow(dy * std::abs(NeighbourOffsets3D[di][1]), 2.0) );

            if (gradient <= minGradient)
            {
               // neighbourColumn has steepest upward gradient from column, so far.
               // set it to be the adjacent column.
               minGradient = gradient;
               nodeToReturn = neighbourNode;
               diToReturn = di;
            }
         }
         else
         {
            assert (depth == neighbourDepth);
            if (!nodeToReturn or nodeToReturn->isDeeperThan (neighbourNode))
            {
               minGradient = 0;
               nodeToReturn = neighbourNode;
               diToReturn = di;
            }
         }
      }

      di = diToReturn;
      return nodeToReturn;
   }


   FormationNode * LocalFormationNode::getAdjacentFormationNode (int directionIndex)
   {
      if (directionIndex < 0) directionIndex = m_selectedDirectionIndex;
      if (directionIndex < 0) return 0;

      // this may still return 0!!!
      return m_formation->getFormationNode (getI () + NeighbourOffsets3D[directionIndex][0],
         getJ () + NeighbourOffsets3D[directionIndex][1],
         getK () + NeighbourOffsets3D[directionIndex][2]);
   }

   int LocalFormationNode::getAdjacentFormationNodeGridOffset (int dimension)
   {
      if (m_selectedDirectionIndex < 0) return 0;

      return NeighbourOffsets3D[m_selectedDirectionIndex][dimension];
   }

   FormationNode * LocalFormationNode::getTargetFormationNode (void)
   {
      // if this is a top node of the top active formation, m_targetFormationNode will be set, even if it has no thickness.
      // If it has thickness, m_targetFormatioNode should be set if target formation nodes have been computed
      if (m_targetFormationNode or hasThickness ())
         return m_targetFormationNode;
      else if (m_topFormationNode)
         return m_topFormationNode->getTargetFormationNode ();
      else
         return 0;
   }

   void LocalFormationNode::prescribeTargetFormationNode (void)
   {
      m_targetFormationNode = this;
      m_selectedDirectionIndex = -1; // set m_selectedDirectionIndex to match the target formation node
   }

   void LocalFormationNode::setEndOfPath (void)
   {
      m_isEndOfPath = true;
      prescribeTargetFormationNode ();
   }

   void LocalFormationNode::cleanTargetFormationNode ()
   {
      m_targetFormationNode = 0;
   }

   bool LocalFormationNode::isImpermeable (void)
   {
      bool impermeable;

      if (hasThickness ())
      {
         impermeable =
            (getVerticalPermeability () <= getBlockingPermeability () or
            getPorosity () <= getBlockingPorosity () or
            getFaultStatus () == SEAL or
            getFaultStatus () == SEALOIL);
      }
      else if (IsValid(m_topFormationNode))
         impermeable = m_topFormationNode->isImpermeable ();
      else
         impermeable = false;

      return impermeable;
   }

   bool LocalFormationNode::hasNowhereToGo (void)
   {
      if (isImpermeable () or getAdjacentFormationNode () == 0)
      {
         return true;
      }
      else
      {
         return false;
      }
   }

   bool LocalFormationNode::goesOutOfBounds (void)
   {
      return (getAdjacentFormationNode () == 0 and m_selectedDirectionIndex >= 0 and m_selectedDirectionIndex < NumberOfNeighbourOffsetsUsed);
   }

   double LocalFormationNode::getDepth ()
   {
      return m_depth;
   }

   double LocalFormationNode::getPressure ()
   {
      if (hasThickness ())
         return m_pressure;
      else if (m_topFormationNode)
         return m_topFormationNode->getPressure ();
      else
         return Interface::DefaultUndefinedMapValue;
   }

   double LocalFormationNode::getTemperature ()
   {
      if (hasThickness ())
         return m_temperature;
      else if (m_topFormationNode)
         return m_topFormationNode->getTemperature ();
      else
         return Interface::DefaultUndefinedMapValue;
   }

   double LocalFormationNode::getHorizontalPermeability ()
   {
      if (hasThickness ())
         return m_horizontalPermeability;
      else if (m_topFormationNode)
         return m_topFormationNode->getHorizontalPermeability ();
      else
         return Interface::DefaultUndefinedMapValue;
   }

   double LocalFormationNode::getVerticalPermeability (bool nodeOnTop)
   {
      if (hasThickness () and !nodeOnTop)
         return m_verticalPermeability[0];
      else if (nodeOnTop)
         return m_verticalPermeability[1];
      else if (m_topFormationNode)
         return m_topFormationNode->getVerticalPermeability ();
      else
         return Interface::DefaultUndefinedMapValue;
   }

   double LocalFormationNode::getPorosity ()
   {
      if (hasThickness ())
         return m_porosity;
      else if (m_topFormationNode)
         return m_topFormationNode->getPorosity ();
      else
         return Interface::DefaultUndefinedMapValue;
   }

   double LocalFormationNode::getWaterDensity ()
   {
      if (hasThickness ())
         return m_waterDensity;
      else if (m_topFormationNode)
         return m_topFormationNode->getWaterDensity ();
      else
         return Interface::DefaultUndefinedMapValue;
   }

   double LocalFormationNode::getOverPressure ()
   {
      if (hasThickness ())
         return m_overPressure;
      else if (m_topFormationNode)
         return m_topFormationNode->getOverPressure ();
      else
         return Interface::DefaultUndefinedMapValue;
   }

   double LocalFormationNode::getVapourDensity ()
   {
      if (hasThickness ())
         return m_vapourDensity;
      else if (m_topFormationNode)
         return m_topFormationNode->getVapourDensity ();
      else
         return Interface::DefaultUndefinedMapValue;
   }

   double LocalFormationNode::getLiquidDensity ()
   {
      if (hasThickness ())
         return m_liquidDensity;
      else if (m_topFormationNode)
         return m_topFormationNode->getLiquidDensity ();
      else
         return Interface::DefaultUndefinedMapValue;
   }

   double LocalFormationNode::getCapillaryEntryPressureVapour (bool nodeOnTop)
   {
      if (hasThickness () and !nodeOnTop)
         return m_capillaryEntryPressureVapour[0];
      else if (nodeOnTop)
         return m_capillaryEntryPressureVapour[1];
      else if (m_topFormationNode)
         return m_topFormationNode->getCapillaryEntryPressureVapour ();
      else
         return Interface::DefaultUndefinedMapValue;
   }

   double LocalFormationNode::getCapillaryEntryPressureLiquid (bool nodeOnTop)
   {
      if (hasThickness () and !nodeOnTop)
         return m_capillaryEntryPressureLiquid[0];
      else if (nodeOnTop)
         return m_capillaryEntryPressureLiquid[1];
      else if (m_topFormationNode)
         return m_topFormationNode->getCapillaryEntryPressureLiquid ();
      else
         return Interface::DefaultUndefinedMapValue;
   }


#ifdef TOBETESTED
   double LocalFormationNode::getValueAtOffset (int offset)
   {
      if (hasThickness ())
         return * ((double *) ((char *) this + offset))
      else if (m_topFormationNode)
      return m_topFormationNode->getValueAtOffset (offset);
      else
         return Interface::DefaultUndefinedMapValue;
   }
#endif

   void LocalFormationNode::setDirectionIndex (int index)
   {
      m_selectedDirectionIndex = index;
   }

   void LocalFormationNode::setReservoirVapour (bool flag)
   {
      m_isReservoirVapour = flag;
   }

   void LocalFormationNode::setReservoirLiquid (bool flag)
   {
      m_isReservoirLiquid = flag;
   }

   void LocalFormationNode::setDepth (double depth)
   {
      m_depth = depth;
   }

   void LocalFormationNode::setPressure (double pressure)
   {
      m_pressure = pressure;
   }

   void LocalFormationNode::setTemperature (double temperature)
   {
      m_temperature = temperature;
   }

   void LocalFormationNode::setPorosity (double porosity)
   {
      m_porosity = porosity;
   }

   void LocalFormationNode::setVerticalPermeability (double verticalPermeability, bool nodeOnTop)
   {
      // We want to set the permeability of 'this' node
      if (!nodeOnTop)
      {
         m_verticalPermeability[0] = verticalPermeability;
      }
      // We want to set the permeability of the node right above,
      // at the interface with the formation above.
      else
      {
         m_verticalPermeability[1] = verticalPermeability;
      }
   }

   void LocalFormationNode::setHorizontalPermeability (double horizontalPermeability)
   {
      m_horizontalPermeability = horizontalPermeability;
   }

   void LocalFormationNode::setOverPressure (double overPressure)
   {
      m_overPressure = overPressure;
   }

   void LocalFormationNode::setVapourDensity (double vapourDensity)
   {
      m_vapourDensity = vapourDensity;
   }

   void LocalFormationNode::setLiquidDensity (double liquidDensity)
   {
      m_liquidDensity = liquidDensity;
   }

   void LocalFormationNode::setCapillaryEntryPressureVapour (double capillaryEntryPressureVapour, bool nodeOnTop)
   {
      // We want to set the Capillary Entry Pressure of 'this' node
      if (!nodeOnTop)
      {
         m_capillaryEntryPressureVapour[0] = capillaryEntryPressureVapour;
      }
      // We want to set the Capillary Entry Pressure of the node right above,
      // at the interface with the formation above.
      else
      {
         m_capillaryEntryPressureVapour[1] = capillaryEntryPressureVapour;
      }
   }

   void LocalFormationNode::setCapillaryEntryPressureLiquid (double capillaryEntryPressureLiquid, bool nodeOnTop)
   {
      // We want to set the Capillary Entry Pressure of 'this' node
      if (!nodeOnTop)
      {
         m_capillaryEntryPressureLiquid[0] = capillaryEntryPressureLiquid;
      }
      // We want to set the Capillary Entry Pressure of the node right above,
      // at the interface with the formation above.
      else
      {
         m_capillaryEntryPressureLiquid[1] = capillaryEntryPressureLiquid;
      }
   }

   bool LocalFormationNode::getReservoirVapour (void)
   {
      // a node can only be a reservoir node if it has thickness
      return m_isReservoirVapour and hasThickness ();
   }

   bool LocalFormationNode::getReservoirLiquid (void)
   {
      // a node can only be a reservoir node if it has thickness
      return m_isReservoirLiquid and hasThickness ();
   }

   bool LocalFormationNode::isEndOfPath (void)
   {
      return m_isEndOfPath;
   }

   double LocalFormationNode::getHeightVapour (void)
   {
      if (m_hasNoThickness) return Interface::DefaultUndefinedMapValue;
      return m_heightVapour;
   }

   double LocalFormationNode::getHeightLiquid (void)
   {
      if (m_hasNoThickness) return Interface::DefaultUndefinedMapValue;
      return m_heightLiquid;
   }

   void LocalFormationNode::setFaultStatus (FaultStatus newFaultStatus)
   {
      m_faultStatus = newFaultStatus;
   }

   FaultStatus LocalFormationNode::getFaultStatus (void)
   {
      return m_faultStatus;
   }

   ThreeVector & LocalFormationNode::getAnalogFlowDirection (void)
   {
      if (!m_analogFlowDirection) m_analogFlowDirection = new FiniteElementMethod::ThreeVector;
      return *m_analogFlowDirection;
   }

   void LocalFormationNode::setFiniteElementDepths (double * depths)
   {
      if (!m_finiteElementsDepths) m_finiteElementsDepths = new double[NumberOfNodeCorners];
      for (int i = 0; i < NumberOfNodeCorners; ++i)
         m_finiteElementsDepths[i] = depths[i];
   }

   bool LocalFormationNode::setFiniteElement (FiniteElementMethod::FiniteElement& finiteElement, const bool computeJacobianInverse, const bool computeGradBasis)
   {
      // now is time to calculate the finite element
      bool returnValue = true;
      double dx = m_formation->getDeltaI ();
      double dy = m_formation->getDeltaJ ();
      int oi;

      if (!m_finiteElementsDepths) m_formation->computeFiniteElement (getI (), getJ (), getK ());

      for (oi = 0; oi < NumberOfNodeCorners; ++oi)
      {
         if (m_finiteElementsDepths[oi] == Interface::DefaultUndefinedMapValue) returnValue = false;
         finiteElement.setGeometryPoint (oi + 1, NodeCornerOffsets[oi][0] * dx, NodeCornerOffsets[oi][1] * dy, m_finiteElementsDepths[oi]);
      }
      finiteElement.setQuadraturePoint (0.0, 0.0, 0.0, computeJacobianInverse, computeGradBasis);

      return returnValue;
   }

   double LocalFormationNode::getFiniteElementValue (double iOffset, double jOffset, double kOffset, PropertyIndex propertyIndex)
   {
      FiniteElement finiteElement;

      // Don't calculate Jacobian for elements with no thickness
      if (!hasThickness ())
      {
         if (!setFiniteElement (finiteElement, false, false)) return Interface::DefaultUndefinedMapValue;
         finiteElement.setQuadraturePoint (iOffset, jOffset, kOffset, false, false);
      }
      else
      {
         if (!setFiniteElement (finiteElement)) return Interface::DefaultUndefinedMapValue;
         finiteElement.setQuadraturePoint (iOffset, jOffset, kOffset);
      }

      // Calculate the property
      ElementVector valueVector;

      int oi;
      for (oi = 0; oi < NumberOfNodeCorners; ++oi)
      {
         double propertyValue = m_formation->getPropertyValue (propertyIndex, getI () + NodeCornerOffsets[oi][0], getJ () + NodeCornerOffsets[oi][1], getK () + NodeCornerOffsets[oi][2]);

         if (propertyValue == Interface::DefaultUndefinedMapValue)
         {
            // undefined node
            return Interface::DefaultUndefinedMapValue;
         }

         valueVector (oi + 1) = propertyValue;
      }

      return finiteElement.interpolate (valueVector);
   }

   ThreeVector LocalFormationNode::getFiniteElementGrad (PropertyIndex propertyIndex)
   {
      ElementVector valueVector;
      FiniteElement finiteElement;
      setFiniteElement (finiteElement);

      int oi;
      for (oi = 0; oi < NumberOfNodeCorners; ++oi)
      {
         double propertyValue = m_formation->getPropertyValue (propertyIndex,
            getI () + NodeCornerOffsets[oi][0],
            getJ () + NodeCornerOffsets[oi][1],
            getK () + NodeCornerOffsets[oi][2]);
         if (propertyValue == Interface::DefaultUndefinedMapValue)
         {
            // undefined node
            ThreeVector res;
            res (1) = Interface::DefaultUndefinedMapValue;
            res (2) = Interface::DefaultUndefinedMapValue;
            res (3) = Interface::DefaultUndefinedMapValue;
            return res;
         }

         valueVector (oi + 1) = propertyValue;
      }

      return finiteElement.interpolateGrad (valueVector);
   }

#ifdef USEPROPERTYVALUES
   void LocalFormationNode::setPropertyValue (int index, double value)
   {
      m_propertyValues[index] = value;
   }

   double LocalFormationNode::getPropertyValue (int index)
   {
      return m_propertyValues[index];
   }
#endif

   FormationNodeArray::FormationNodeArray (Formation * formation,
      int numIGlobal, int numJGlobal,
      int firstILocal, int lastILocal,
      int firstJLocal, int lastJLocal,
      int depth) :
      m_formation (formation),
      m_numIGlobal (numIGlobal), m_numJGlobal (numJGlobal),
      m_firstILocal (firstILocal),
      m_lastILocal (lastILocal),
      m_firstJLocal (firstJLocal),
      m_lastJLocal (lastJLocal),
      m_depth (depth)
   {
      assert (m_lastILocal <= m_numIGlobal - 1);
      assert (m_lastJLocal <= m_numJGlobal - 1);

      m_localFormationNodes = Array<FormationNode *>::create3d (m_lastILocal + 1 - m_firstILocal, m_lastJLocal + 1 - m_firstJLocal, m_depth, 0);

      int i;
      int j;
      int k;
      for (i = m_lastILocal - m_firstILocal; i >= 0; --i)
      {
         for (j = m_lastJLocal - m_firstJLocal; j >= 0; --j)
         {
            for (k = m_depth - 1; k >= 0; --k)
            {
               m_localFormationNodes[i][j][k] = new LocalFormationNode (i + m_firstILocal, j + m_firstJLocal, k, m_formation);
            }
         }
      }

      m_proxyFormationNodes = new ProxyFormationNodeMap[m_depth];
      m_numberOfProxyFormationNodes = 0;
   }

   FormationNodeArray::~FormationNodeArray (void)
   {
      // sends messages to LocalFormationNode objects to notify of destruction
      // RequestHandling::StartRequestHandling (m_formation->getMigrator (), "destroyProxyFormationNodes");

      int i, j, k;
      for (k = m_depth - 1; k >= 0; --k)
      {
         ProxyFormationNodeMap::iterator proxyIter;

         for (proxyIter = m_proxyFormationNodes[k].begin (); proxyIter != m_proxyFormationNodes[k].end (); ++proxyIter)
         {
            ProxyFormationNode * formationNode = (*proxyIter).second;
            delete formationNode;
         }
         m_proxyFormationNodes[k].clear ();
      }

      delete[] m_proxyFormationNodes;
      m_numberOfProxyFormationNodes = 0;

      // Here to make sure all proxy nodes are destroyed before destruction of local nodes
      // RequestHandling::FinishRequestHandling ();

      if (m_localFormationNodes)
      {
         for (i = m_lastILocal - m_firstILocal; i >= 0; --i)
         {
            for (j = m_lastJLocal - m_firstJLocal; j >= 0; --j)
            {
               for (k = m_depth - 1; k >= 0; --k)
               {
                  if (m_localFormationNodes[i][j][k])
                  {
                     delete m_localFormationNodes[i][j][k];
                     m_localFormationNodes[i][j][k] = 0;
                  }
               }
            }
         }

         Array < FormationNode * >::delete3d (m_localFormationNodes);

         m_localFormationNodes = 0;
      }
   }

   FormationNode * FormationNodeArray::getFormationNode (int i, int j, int k)
   {
      if ((i < 0) or (j < 0) or (k < 0))
         return 0;

      if ((unsigned int)i >= m_numIGlobal or (unsigned int)j >= m_numJGlobal or (unsigned int)k >= m_depth)
         return 0;

      if ((unsigned int)i >= m_firstILocal and (unsigned int)i <= m_lastILocal and (unsigned int)j >= m_firstJLocal and (unsigned int)j <= m_lastJLocal)
      {
         return getLocalFormationNode (i, j, k);
      }
      else
      {
         return getProxyFormationNode (i, j, k);
      }
   }

   LocalFormationNode * FormationNodeArray::getLocalFormationNode (int i, int j, int k)
   {
      if ((i < 0) or (j < 0) or (k < 0)) return 0;

      LocalFormationNode * localFormationNode = 0;

      if ((unsigned int)i >= m_firstILocal and (unsigned int)i <= m_lastILocal and (unsigned int)j >= m_firstJLocal and (unsigned int)j <= m_lastJLocal and (unsigned int)k < m_depth)
      {
         FormationNode * formationNode = m_localFormationNodes[i - m_firstILocal][j - m_firstJLocal][k];
         assert (formationNode);
         localFormationNode = dynamic_cast<LocalFormationNode *> (formationNode);
         assert (localFormationNode);
      }

      return localFormationNode;
   }

   ProxyFormationNode * FormationNodeArray::getProxyFormationNode (int i, int j, int k)
   {
      if ((i < 0) or (j < 0) or (k < 0)) return 0;

      ProxyFormationNode * formationNode = 0;

      if ((unsigned int)i < m_numIGlobal and (unsigned int)j < m_numJGlobal and
         !((unsigned int)i >= m_firstILocal and (unsigned int)i <= m_lastILocal and (unsigned int)j >= m_firstJLocal and (unsigned int)j <= m_lastJLocal) and k < (int)m_depth)
      {
         formationNode = m_proxyFormationNodes[k][IndexPair (i, j)];
         if (!formationNode)
         {
            formationNode = m_proxyFormationNodes[k][IndexPair (i, j)] = new ProxyFormationNode (i, j, k, m_formation);
            ++m_numberOfProxyFormationNodes;
            // need to do this AFTER proxyFormationNode has been created and added to the datastructure.
#ifdef REGISTERPROXIES
            formationNode->registerWithLocal ();
#endif
         }
      }
      return formationNode;
   }

   bool FormationNodeArray::clearProperties (void)
   {
      for (unsigned int i = firstILocal (); i <= lastILocal (); ++i)
      {
         for (unsigned int j = firstJLocal (); j <= lastJLocal (); ++j)
         {
            for (unsigned int k = 0; k < m_depth; ++k)
            {
               getLocalFormationNode (i, j, k)->clearProperties ();
            }
         }
      }

      ProxyFormationNodeMap::iterator proxyIter;

      for (unsigned int k = 0; k < m_depth; ++k)
      {
         for (proxyIter = m_proxyFormationNodes[k].begin (); proxyIter != m_proxyFormationNodes[k].end (); ++proxyIter)
         {
            ProxyFormationNode * proxyFormationNode = (*proxyIter).second;
            proxyFormationNode->clearProperties ();
         }
      }

      return true;
   }

   bool FormationNodeArray::computeProperties (void)
   {
      for (unsigned int i = firstILocal (); i <= lastILocal (); ++i)
      {
         for (unsigned int j = firstJLocal (); j <= lastJLocal (); ++j)
         {
            for (unsigned int k = 0; k < m_depth; ++k)
            {
               getLocalFormationNode (i, j, k)->computeProperties ();
            }
         }
      }

      return true;
   }

   bool FormationNodeArray::clearReservoirProperties (void)
   {
      for (unsigned int i = firstILocal (); i <= lastILocal (); ++i)
      {
         for (unsigned int j = firstJLocal (); j <= lastJLocal (); ++j)
         {
            for (unsigned int k = 0; k < m_depth; ++k)
            {
               getLocalFormationNode (i, j, k)->clearReservoirProperties ();
            }
         }
      }
      return true;
   }

   bool IsValid (FormationNode * formationNode)
   {
      return (formationNode != 0 and formationNode->isValid ());
   }

   ostream & operator<< (ostream & stream, FormationNode &formationNode)
   {
      return stream << &formationNode;
   }

   ostream & operator<< (ostream & stream, FormationNode * formationNode)
   {
      if (!formationNode)
      {
         stream << "null";
      }
      else
      {
         stream << formationNode->getFormation ()->getName ()
            << " ("
            << formationNode->getI ()
            << ", "
            << formationNode->getJ ()
            << ", "
            << formationNode->getK ()
            << ")";
      }
      return stream;
   }

}
