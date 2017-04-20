// Copyright (C) 2010-2017 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_FORMATIONNODE_H_
#define _MIGRATION_FORMATIONNODE_H_


#include "BitField.h"
#include "Composition.h"

#include "RequestDefs.h"

#include "FiniteElementTypes.h"
#include "FiniteElement.h"

#include "migration.h"

#include <vector>
#include <boost/array.hpp>

namespace migration
{
   class Migrator;

   class FormationNode :public BitField
   {
   public:
      /// Constructor
      FormationNode (unsigned int i, unsigned int j, int k, Formation * formation);

      /// Destructor
      virtual ~FormationNode (void);

      virtual bool isValid (void) = 0;

      virtual inline void clearProperties (void);
      virtual inline void clearReservoirProperties (void);

      inline unsigned int getI (void) const;
      inline unsigned int getJ (void) const;
      inline unsigned int getK (void) const;
      inline Formation *getFormation (void) const;

      //these are functions used to compare depths in the detectReservoirCrests algorithm
      int compareDepths (FormationNode * node, bool useTieBreaker = true);
      inline bool isDeeperThan (FormationNode * node, bool useTieBreaker = true);
      inline bool isShallowerThan (FormationNode * node, bool useTieBreaker = true);

      //bool isOnBoundary (void);

      virtual double getFiniteElementValue (double iOffset, double jOffset, double kOffset, PropertyIndex propertyIndex) = 0;

      virtual FormationNode *getTargetFormationNode (void) = 0;
      virtual bool computeTargetFormationNode (void) = 0;

      virtual FiniteElementMethod::ThreeVector & getAnalogFlowDirection (void) = 0;

      virtual double getFiniteElementMinimumValue (PropertyIndex propertyIndex) = 0;
      virtual FiniteElementMethod::ThreeVector getFiniteElementGrad (PropertyIndex propertyIndex) = 0;

      virtual double getDepth (void) = 0;

      inline bool hasThickness ();

      virtual bool isImpermeable (void) = 0;
      virtual bool hasNoThickness (void) = 0;
      virtual bool hasNowhereToGo (void) = 0;
      virtual bool goesOutOfBounds (void) = 0;

      virtual bool getReservoirVapour (void) = 0;
      virtual bool getReservoirLiquid (void) = 0;

      virtual void addComposition (const Composition &) = 0;
      virtual Composition & getComposition (void) = 0;

      virtual bool isEndOfPath (void) = 0;

      virtual double getHeightVapour (void) = 0;
      virtual double getHeightLiquid (void) = 0;

      virtual FaultStatus getFaultStatus (void) = 0;

      Migrator *getMigrator (void);
      Formation *getFormation (int index);

#ifdef USEPROPERTYVALUES
      virtual double getPropertyValue (int index) = 0;
#endif

   protected:

      Formation * m_formation;

      int m_iGlobal;
      int m_jGlobal;
      int m_k;
   };


   class ProxyFormationNode :public FormationNode
   {
   public:
      /// Constructor
      ProxyFormationNode (unsigned int i, unsigned int j, int k, Formation * formation);

      /// Destructor
      virtual ~ProxyFormationNode (void);

      virtual inline void clearProperties (void);

      template <class T>
      T getScalarValue (ValueSpec valueSpec);

      template <class T>
      T getCachedScalarValue (ValueSpec valueSpec, FormationNodeCacheBit cacheBit, T & value);

      double getCachedDoubleValue (ValueSpec valueSpec, FormationNodeCacheBit cacheBit, double & value);
      double getDoubleValue (ValueSpec valueSpec);

      virtual bool isValid (void);

      void fillFormationNodeRequest (FormationNodeRequest & request, ValueSpec valueSpec);

      virtual void clearCache (void);

      virtual FormationNode *getTargetFormationNode (void);
      virtual bool computeTargetFormationNode (void);

      virtual FiniteElementMethod::ThreeVector & getAnalogFlowDirection (void);

      virtual double getFiniteElementValue (double iOffset, double jOffset, double kOffset, PropertyIndex propertyIndex);

      virtual double getFiniteElementMinimumValue (PropertyIndex propertyIndex);

      virtual FiniteElementMethod::ThreeVector getFiniteElementGrad (PropertyIndex propertyIndex);

      virtual double getDepth (void);

      virtual bool isImpermeable (void);
      virtual bool hasNoThickness (void);
      virtual bool hasNowhereToGo (void);
      virtual bool goesOutOfBounds (void);

      virtual bool getReservoirVapour (void);
      virtual bool getReservoirLiquid (void);

      virtual void addComposition (const Composition &);
      virtual Composition & getComposition (void);

      virtual bool isEndOfPath (void);

      virtual double getHeightVapour (void);
      virtual double getHeightLiquid (void);

      virtual FaultStatus getFaultStatus (void);

#ifdef USEPROPERTYVALUES
      virtual double getPropertyValue (int index);
#endif

#ifdef REGISTERPROXIES
      void registerWithLocal (void);
      void deregisterWithLocal (void);
#endif

   private:

      bool m_isValid;
      bool m_isImpermeable;
      bool m_hasNoThickness;
      bool m_hasNowhereToGo;
      bool m_goesOutOfBounds;
      double m_depth;
      double m_heightVapour;
      double m_heightLiquid;

      virtual bool isCached (FormationNodeCacheBit bit) const;
      virtual void setCached (FormationNodeCacheBit bit) const;
      virtual void resetCached (FormationNodeCacheBit bit) const;

      mutable BitField m_cachedValues;

      Composition * m_compositionToBeMigrated;
      FormationNode * m_targetFormationNode;
      FiniteElementMethod::ThreeVector *  m_analogFlowDirection;
   };

   typedef pair < int, double >IntDoublePair;

   class LocalFormationNode :public FormationNode
   {
   public:
      /// Constructor
      LocalFormationNode (unsigned int i, unsigned int j, int k, Formation * formation);

      /// Destructor
      virtual ~LocalFormationNode (void);

      virtual bool isValid (void);

      /// @brief Whether to use the advanced mode of the BPA2 engine
      bool performAdvancedMigration (void);
      /// @brief Whether to account for pore-pressure and capillary-pressure gradients in flow-pathy calculation
      bool performHDynamicAndCapillary (void);
      /// @return The value of blocking porosity
      double getBlockingPorosity (void);
      /// @return The value of blocking permeability
      double getBlockingPermeability (void);

      virtual void clearProperties (void);
      virtual void clearReservoirProperties (void);
      virtual void computeProperties (void);

      virtual void setValue (FormationNodeValueRequest & request);
      virtual void getValue (FormationNodeValueRequest & request, FormationNodeValueRequest & response);

      virtual void getThreeVectorValue (FormationNodeThreeVectorValueRequest & request, FormationNodeThreeVectorValueRequest & response);
      virtual void getThreeVector (FormationNodeThreeVectorRequest & request, FormationNodeThreeVectorRequest & response);

      virtual FaultStatus getFaultStatus (void);
      virtual void setFaultStatus (FaultStatus newFaultStatus);

      void addChargeToBeMigrated (int componentId, double weight);

      // evaluating the change in capillary pressure across the boundary
      bool computeCapillaryPressure (WaterSaturation waterSaturation, double & pressureVapour, double & pressureLiquid);
      double computeBrooksCoreyCorrection (double Sw, double lamda) const;

      bool detectReservoir (LocalFormationNode * topNode, const double minLiquidColumnHeight, const double minVapourColumnHeight, const bool pressureRun);
      double getPressureContrast( const LocalFormationNode * topNode, const PhaseId phase, const bool pressureRun ) const;
      void identifyAsReservoir (void);

      bool isPartOfUndetectedReservoir(void);
      void dealWithStuckHydrocarbons(void);
      bool undetectedCrest(void);
      FormationNode * getLateralAdjacentNode(int & di);

      // check if a LocalFormationNode is a crest node
      bool detectReservoirCrests (PhaseId phase);
      bool getIsCrest (PhaseId phase);

      void computeNodeProperties (void);
      void computeAnalogFlowDirection (void);
      void determineThicknessProperties (void);

      void setDepth (double);
      void setPressure (double);
      void setTemperature (double);
      void setPorosity (double);
      void setVerticalPermeability (double permeability, bool nodeOnTop = false);
      void setHorizontalPermeability (double);
      void setOverPressure (double);

      void setVapourDensity (double);
      void setLiquidDensity (double);

      void setCapillaryEntryPressureVapour (double, bool nodeOnTop = false);
      void setCapillaryEntryPressureLiquid (double, bool nodeOnTop = false);

      void computeAdjacentNode (void);
      void computeCosines (void);
      void computeNextAdjacentNode (void);

      void prescribeTargetFormationNode (void);
      void cleanTargetFormationNode (void);

      void setEndOfPath (void);

      void setDirectionIndex (int);

      FormationNode *getAdjacentFormationNode (int directionIndex = -1);
      int getAdjacentFormationNodeGridOffset (int dimension);

      virtual FormationNode *getTargetFormationNode (void);
      virtual bool computeTargetFormationNode (void);

      virtual FiniteElementMethod::ThreeVector & getAnalogFlowDirection (void);

      virtual double getFiniteElementValue (double iOffset, double jOffset, double kOffset, PropertyIndex propertyIndex);

      virtual double getFiniteElementMinimumValue (PropertyIndex propertyIndex);

      virtual FiniteElementMethod::ThreeVector getFiniteElementGrad (PropertyIndex propertyIndex);

      void setFiniteElementDepths (double * depths);
      bool setFiniteElement (FiniteElementMethod::FiniteElement& finiteElement);

      double getDepth (void);

      virtual bool isImpermeable (void);
      virtual bool hasNowhereToGo (void);
      virtual bool goesOutOfBounds (void);

      inline bool hasNoThickness ();

      double getHorizontalPermeability ();
      double getVerticalPermeability (bool nodeOnTop = false);
      double getPorosity ();
      double getPressure ();
      double getTemperature ();
      double getWaterDensity ();
      double getOverPressure ();

      double getVapourDensity ();
      double getLiquidDensity ();

      double getCapillaryEntryPressureVapour (bool nodeOnTop = false);
      double getCapillaryEntryPressureLiquid (bool nodeOnTop = false);

      void setReservoirVapour (bool flag);
      void setReservoirLiquid (bool flag);

      virtual bool getReservoirVapour (void);
      virtual bool getReservoirLiquid (void);

      void manipulateComposition (ValueSpec valueSpec, int phase, Composition & composition);
      virtual void addComposition (const Composition &);
      virtual Composition & getComposition (void);
      void getComposition (ValueSpec valueSpec, int phase, Composition & composition);

      virtual bool isEndOfPath (void);

      virtual double getHeightVapour (void);
      virtual double getHeightLiquid (void);

#ifdef USEPROPERTYVALUES
      void setPropertyValue (int index, double value);
      virtual double getPropertyValue (int index);
#endif

#ifdef REGISTERPROXIES
      void addProxy (int rank);
      void deleteProxy (int rank);
      void resetProxy (int rank);
      void resetProxies (void);
#endif

      inline double getHeightLiquid () const;
      inline double getHeightVapour () const;

   private:

#ifdef USEBOTTOMFORMATIONNODE
      LocalFormationNode * m_bottomFormationNode;
#endif

      FormationNode *m_targetFormationNode;

#ifdef REGISTERPROXIES
      std::vector < int > m_proxies;
#endif

#ifdef USEPROPERTYVALUES
      std::vector < double > m_propertyValues;
#endif

      double m_depth;
      double m_temperature;
      double m_pressure;
      double m_horizontalPermeability;
      double m_porosity;
      double m_overPressure;
      double m_liquidDensity;
      double m_vapourDensity;
      double m_heightLiquid;                   // actual height of liquid column (only for uppermost cells!)
      double m_heightVapour;                   // actual height of vapour column (only for uppermost cells!)
      double m_waterDensity;

      FaultStatus m_faultStatus;

      int m_adjacentNodeIndex;                 // index into m_cosines of current adjacent node to use in computeAdjacentNode ()
      int m_selectedDirectionIndex;            // index into NeighbourOffsets3D of current adjacent node to use in computeTargetFormationNode (),
      // derived using m_adjacentNodeIndex
      int m_tried;

      bool m_entered;                          // Whether computeTargetFormationNode () loops
      bool m_hasNoThickness;                   // whether the node has thickness

      bool m_isReservoirVapour;                // true - if node is potential trap
      bool m_isReservoirLiquid;                // true - if node is potential trap
      bool m_isCrestLiquid;                    // true - if node is a crest for liquid
      bool m_isCrestVapour;                    // true - if node is a crest for vapour

      bool m_isEndOfPath;                      // true - if node is end of path. May even be a leaking (or zero-thickness) node
      // but it needs to be the end of the path to register it in the leaking reservoir.

      vector < IntDoublePair > *  m_cosines;     // cosines of angles between the analog flow direction and the feasible discretized flow directions  

      boost::array<double, 2> m_verticalPermeability;
      boost::array<double, 2> m_capillaryEntryPressureLiquid;
      boost::array<double, 2> m_capillaryEntryPressureVapour;

      Composition * m_compositionToBeMigrated;
      FiniteElementMethod::ThreeVector * m_analogFlowDirection;
      LocalFormationNode * m_topFormationNode;

      // we only need 8 depths (64 bytes), we can create finite elements (816 bytes) in the scope of the functions. 
      // In this way we save 752 bytes per grid node. 
      double * m_finiteElementsDepths;
   };

   bool IsValid (FormationNode * formationNode);

   /// To be made less memory-hungry ....
   class FormationNodeArray
   {
   public:
      FormationNodeArray (Formation * formation, int numIGlobal, int numJGlobal, int firstILocal, int firstJLocal, int lastILocal, int lastJLocal, int depth);

      virtual ~FormationNodeArray (void);

      FormationNode *getFormationNode (int i, int j, int k);

      LocalFormationNode *getLocalFormationNode (int i, int j, int k);
      ProxyFormationNode *getProxyFormationNode (int i, int j, int k);

      inline unsigned int numIGlobal (void);
      inline unsigned int numJGlobal (void);
      inline unsigned int firstILocal (void);
      inline unsigned int firstJLocal (void);
      inline unsigned int lastILocal (void);
      inline unsigned int lastJLocal (void);
      inline unsigned int depth (void);

      inline int getNumberOfProxyFormationNodes (void);

      bool clearProperties (void);
      bool clearReservoirProperties (void);
      bool computeProperties (void);

   private:
      unsigned int m_numIGlobal;
      unsigned int m_numJGlobal;
      unsigned int m_firstILocal;
      unsigned int m_lastILocal;
      unsigned int m_firstJLocal;
      unsigned int m_lastJLocal;
      unsigned int m_depth;
      int m_numberOfProxyFormationNodes;

      typedef map < IndexPair, ProxyFormationNode * >ProxyFormationNodeMap;
      ProxyFormationNodeMap *m_proxyFormationNodes;

      FormationNode ****m_localFormationNodes;
      Formation * m_formation;
   };

   ostream & operator<< (ostream & stream, FormationNode & node);
   ostream & operator<< (ostream & stream, FormationNode * node);

   unsigned int FormationNode::getI (void) const
   {
      return m_iGlobal;
   }

   unsigned int FormationNode::getJ (void) const
   {
      return m_jGlobal;
   }

   unsigned int FormationNode::getK (void) const
   {
      return m_k;
   }


   Formation *FormationNode::getFormation (void) const
   {
      return m_formation;
   }

   void FormationNode::clearProperties (void)
   {
      clearAll ();
   }

   void FormationNode::clearReservoirProperties (void)
   {
   }

   bool FormationNode::hasThickness ()
   {
      return !hasNoThickness ();
   }


   // ------------------------------------------------------------------------------


   bool LocalFormationNode::hasNoThickness ()
   {
      return m_hasNoThickness;
   }

   double LocalFormationNode::getHeightLiquid () const
   {
      return m_heightLiquid;
   }

   double LocalFormationNode::getHeightVapour () const
   {
      return m_heightVapour;
   }

   void ProxyFormationNode::clearProperties (void)
   {
      FormationNode::clearProperties ();
      clearCache ();
      m_targetFormationNode = 0;

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

   unsigned int FormationNodeArray::numIGlobal (void)
   {
      return m_numIGlobal;
   }

   unsigned int FormationNodeArray::numJGlobal (void)
   {
      return m_numJGlobal;
   }

   unsigned int FormationNodeArray::firstILocal (void)
   {
      return m_firstILocal;
   }

   unsigned int FormationNodeArray::firstJLocal (void)
   {
      return m_firstJLocal;
   }

   unsigned int FormationNodeArray::lastILocal (void)
   {
      return m_lastILocal;
   }

   unsigned int FormationNodeArray::lastJLocal (void)
   {
      return m_lastJLocal;
   }

   unsigned int FormationNodeArray::depth (void)
   {
      return m_depth;
   }

   bool FormationNode::isDeeperThan (FormationNode * node, bool useTieBreaker)
   {
      return compareDepths (node, useTieBreaker) == 1;
   }

   bool FormationNode::isShallowerThan (FormationNode * node, bool useTieBreaker)
   {
      return compareDepths (node, useTieBreaker) == -1;
   }

}

#endif // _MIGRATION_FORMATIONNODE_H_
