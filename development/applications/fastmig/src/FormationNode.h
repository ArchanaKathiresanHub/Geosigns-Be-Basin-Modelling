// Copyright (C) 2010-2015 Shell International Exploration & Production.
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
      inline Formation *getFormation (void);

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

      virtual bool getReservoirGas (void) = 0;
      virtual bool getReservoirOil (void) = 0;

      virtual double getHeightGas (void) = 0;
      virtual double getHeightOil (void) = 0;

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

      virtual bool getReservoirGas (void);
      virtual bool getReservoirOil (void);

      virtual double getHeightGas (void);
      virtual double getHeightOil (void);

      virtual FaultStatus getFaultStatus (void);

#ifdef USEPROPERTYVALUES
      virtual double getPropertyValue (int index);
#endif

#ifdef REGISTERPROXIES
      void registerWithLocal (void);
      void deregisterWithLocal (void);
#endif

   private:
      FormationNode * m_targetFormationNode;
      FiniteElementMethod::ThreeVector m_analogFlowDirection;

      bool m_isValid;
      bool m_isImpermeable;
      bool m_hasNoThickness;
      bool m_hasNowhereToGo;
      bool m_goesOutOfBounds;

      double m_depth;
      double m_height_gas;
      double m_height_oil;

      virtual bool isCached (FormationNodeCacheBit bit) const;
      virtual void setCached (FormationNodeCacheBit bit) const;
      virtual void resetCached (FormationNodeCacheBit bit) const;

      mutable BitField m_cachedValues;
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

      double performVerticalMigration (void);
      double performHDynamicAndCapillary (void);
      double getBlockingPorosity (void);
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
      bool computeCapillaryPressure (WaterSaturation waterSaturation, double & pressureGas, double & pressureOil);
      double computeBrooksCoreyCorrection (double Sw, double lamda) const;

      bool detectReservoir (LocalFormationNode * topNode, const double minOilColumnHeight, const double minGasColumnHeight, const bool pressureRun);
      void identifyAsReservoir (void);

      // check if a LocalFormationNode is a crest node
      bool detectReservoirCrests (PhaseId phase);
      bool getIsCrest (PhaseId phase);

      //these are functions used to compare depths in the detectReservoirCrests algorithm
      virtual int compareDepths (FormationNode * node, bool useTieBreaker = true);
      inline bool isDeeperThan (FormationNode * node, bool useTieBreaker = true);
      inline bool isShallowerThan (FormationNode * node, bool useTieBreaker = true);

      void computeNodeProperties (void);
      void computeAnalogFlowDirection (void);
      void determineThicknessProperties (void);

      void setOverPressure (double);

      void computeAdjacentNode (void);
      void computeCosines (void);
      void computeNextAdjacentNode (void);

      void prescribeTargetFormationNode (void);
      void cleanTargetFormationNode (void);

      void setDirectionIndex (int);

      FormationNode *getAdjacentFormationNode (int directionIndex = -1);
      int getAdjacentFormationNodeGridOffset (int dimension);

#ifdef USEDISCRETIZEDFLOWDIRECTIONS
      double getDiscretizedFlowDirection (int direction);
#endif

      virtual FormationNode *getTargetFormationNode (void);
      virtual bool computeTargetFormationNode (void);

      virtual FiniteElementMethod::ThreeVector & getAnalogFlowDirection (void);

      virtual double getFiniteElementValue (double iOffset, double jOffset, double kOffset, PropertyIndex propertyIndex);

      virtual double getFiniteElementMinimumValue (PropertyIndex propertyIndex);

      virtual FiniteElementMethod::ThreeVector getFiniteElementGrad (PropertyIndex propertyIndex);

      FiniteElementMethod::FiniteElement & getFiniteElement (void);

      double getDepth (void);

      virtual bool isImpermeable (void);
      virtual bool hasNowhereToGo (void);
      virtual bool goesOutOfBounds (void);

      inline bool hasNoThickness ();

      double getHorizontalPermeability ();
      double getVerticalPermeability ();
      double getPorosity ();
      double getWaterDensity ();
      double getOverPressure ();

      void setReservoirGas (bool flag);
      void setReservoirOil (bool flag);

      virtual bool getReservoirGas (void);
      virtual bool getReservoirOil (void);

      virtual double getHeightGas (void);
      virtual double getHeightOil (void);

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

      inline double getHeightOil () const;
      inline double getHeightGas () const;

   private:
      Composition * m_compositionToBeMigrated;

      FiniteElementMethod::FiniteElement m_finiteElement;

      FiniteElementMethod::ThreeVector m_analogFlowDirection;
#ifdef USEDISCRETIZEDFLOWDIRECTIONS
      FiniteElementMethod::ThreeVector m_discretizedFlowDirection;
#endif

      LocalFormationNode * m_topFormationNode;
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
      double m_horizontalPermeability;
      double m_verticalPermeability;
      double m_porosity;
      double m_overPressure;

      double m_height_oil;                     // actual height of oil (C6-14) column   (only for uppermost cells!)
      double m_height_gas;                     // actual height of gas (methane) column (only for uppermost cells!)

      double m_waterDensity;

      FaultStatus m_faultStatus;

      int m_adjacentNodeIndex;                 // index into m_cosines of current adjacent node to use in computeAdjacentNode ()
      int m_selectedDirectionIndex;            // index into NeighbourOffsets3D of current adjacent node to use in computeTargetFormationNode (),
      // derived using m_adjacentNodeIndex

      int m_tried;                             // Used to check whether computeTargetFormationNode () loops across multiple processors

      bool m_entered;                          // Whether computeTargetFormationNode () loops
      bool m_hasNoThickness;                   // whether the node has thickness

      bool m_isReservoirGas;                   // true - if node is potential trap
      bool m_isReservoirOil;                   // true - if node is potential trap
      bool m_isCrestOil;		       // true - if node is a crest for oil
      bool m_isCrestGas;                       // true - if node is a crest for gas

      vector < IntDoublePair > *m_cosines;     // cosines of angles between the analog flow direction and the feasible discretized flow directions
#ifdef USEDISCRETIZEDFLOWDIRECTIONS
      vector < FiniteElementMethod::ThreeVector > * m_discretizedFlowDirections; // the feasible discretized flow directions
#endif

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
      Formation * m_formation;

      unsigned int m_numIGlobal;
      unsigned int m_numJGlobal;
      unsigned int m_firstILocal;
      unsigned int m_lastILocal;
      unsigned int m_firstJLocal;
      unsigned int m_lastJLocal;
      unsigned int m_depth;

      FormationNode ****m_localFormationNodes;

      typedef map < IndexPair, ProxyFormationNode * >ProxyFormationNodeMap;
      ProxyFormationNodeMap *m_proxyFormationNodes;

      int m_numberOfProxyFormationNodes;
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


   Formation *FormationNode::getFormation (void)
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

   double LocalFormationNode::getHeightOil () const
   {
      return m_height_oil;
   }

   double LocalFormationNode::getHeightGas () const
   {
      return m_height_gas;
   }

   void ProxyFormationNode::clearProperties (void)
   {
      FormationNode::clearProperties ();
      clearCache ();
      m_targetFormationNode = 0;
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

   bool LocalFormationNode::isDeeperThan (FormationNode * node, bool useTieBreaker)
   {
      return compareDepths (node, useTieBreaker) == 1;
   }

   bool LocalFormationNode::isShallowerThan (FormationNode * node, bool useTieBreaker)
   {
      return compareDepths (node, useTieBreaker) == -1;
   }

}

#endif // _MIGRATION_FORMATIONNODE_H_
