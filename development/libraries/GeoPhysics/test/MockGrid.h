/**
 * \file SchmeiderCompactionCalculatorTest.C
 * \brief Test the SchneiderCompactionCalculator class
 * \author Magali Cougnenc magali.cougnenc@pds.nl
 * \date May 2014
 *
 * Test of the chemical compaction through Schneider model
 * Test of extreme time values, extreme parameters values, no valid nodes...
 */


#ifndef MOCK_GRID
#define MOCK_GRID

using namespace GeoPhysics;

class MockGrid : public ChemicalCompactionCalculator::Grid
{
public:

   inline MockGrid ( const int size,
         const double previousTime,
         const double currentTime,
         double * chemicalCompaction,
         const double * porosity,
         const double * temperature,
         const double * ves,
         const int numberValidNodes,
         const int * validNodes,
         const int * lithoMap,
         const int numberLithologies,
         const double * activationEnergy,
         const double * referenceViscosity
   );//For Schneider chemical compaction

   inline MockGrid ( const int size,
         const double previousTime,
         const double currentTime,
         double * chemicalCompaction,
         const double * porosity,
         const double * previousTemperature,
         const double * currentTemperature,
         const int numberValidNodes,
         const int * validNodes,
         const int * lithoMap,
         const int numberLithologies,
         const double * fractionQuartz,
         const double * grainSize,
         const double * coatingClay
   );//For Walderhaug chemical compaction

   ~MockGrid()
   {
      //destructor
   }

   /*!
    * \brief get the time of the previous snapshot in Ma (million of years)
    */
   inline double getPreviousTime() const { return m_previousTime; }

   /*!
    * \brief get time of the current snapshot in Ma (million of years)
    */
   inline double getCurrentTime() const { return m_currentTime; }

   /*!
    * \brief get number of nodes in the grid
    */
   inline int getSize() const { return m_size; }

   /*!
    * \brief get the active nodes in the grid
    */
   inline const int * getActiveNodes() const { return m_validNodes; }

   /*!
    * \brief get the number of active nodes which belong to chemical compactable layers
    */
   inline int getNumberOfActiveNodes() const { return m_numberValidNodes; }

   /*!
    * \brief get the temperature of the previous snapshot
    */
   inline const double * getPreviousTemperature() const { return m_previousTemperature; }

   /*!
    * \brief get the temperature of the current snapshot
    */
   inline const double * getCurrentTemperature() const { return m_currentTemperature; }

   /*!
    * \brief get the porosity
    */
   inline const double * getPorosity() const { return m_porosity; }

   /*!
    * \brief get the vertical effective stress
    */
   inline const double * getVES() const { return m_ves; }

   /*!
    * \brief get and set the chemicalcompaction
    */
   inline double * setChemicalCompaction() { return m_chemicalCompaction; }

   /*!
    * \brief get the number of different lithologies in the grid
    */
   inline int getNumberOfLithologies() const { return m_numberLithologies; }

   /*!
    * \brief get the array of the reference lithology number of all nodes in the basin
    *
    * The size of the vector of int return by the function is the number of nodes in the whole basin
    *
    */
   inline void getLithologyMap(std::vector< int > & lithoMap) const { lithoMap = m_lithoMap; }

   /*!
    * \brief get the activation energy of the lithology
    */
   inline const double * getActivationEnergy() const { return m_activationEnergy; }

   /*!
    * \brief get the reference viscocity of the lithology
    */
   inline const double * getReferenceViscosity() const { return m_referenceViscosity; }

   /*!
    * \brief get the quartz grain size of the lithology
    */
   inline const double * getQuartzGrainSize() const { return m_grainSize; }

   /*!
    * \brief get the fraction of quartz of the lithology
    */
   inline const double * getQuartzFraction() const { return m_fractionQuartz; }

   /*!
    * \brief get the fraction of surface quartz area coated by clay of the lithology
    */
   inline const double * getCoatingClayFactor() const { return m_coatingClay; }



private:
   const int m_size;
   const double m_previousTime;
   const double m_currentTime;

   double * m_chemicalCompaction;
   const double * m_porosity;
   const double * m_previousTemperature;
   const double * m_currentTemperature;
   const double * m_ves;

   const int m_numberValidNodes;
   const int * m_validNodes;
   std::vector < int > m_lithoMap;
   const int m_numberLithologies;

   //For Schneider model
   const double * m_activationEnergy;
   const double * m_referenceViscosity;

   //For Walderhaug model
   const double * m_grainSize;
   const double * m_coatingClay;
   const double * m_fractionQuartz;

};

/*!
 * \brief constructor of the grid for Schneider chemical compaction model
 */
inline MockGrid::MockGrid(
      const int size,
      const double previousTime,
      const double currentTime,
      double * chemicalCompaction,
      const double * porosity,
      const double * temperature,
      const double * ves,
      const int numberValidNodes,
      const int * validNodes,
      const int * lithoMap,
      const int numberLithologies,
      const double * activationEnergy,
      const double * referenceViscosity ) :
			              m_size( size ),
			              m_previousTime( previousTime ),
			              m_currentTime( currentTime ),
			              m_chemicalCompaction( chemicalCompaction ),
			              m_porosity( porosity ),
			              m_currentTemperature( temperature ),
			              m_ves( ves ),
			              m_numberValidNodes( numberValidNodes ),
			              m_validNodes( validNodes ),
			              m_numberLithologies( numberLithologies ),
			              m_activationEnergy( activationEnergy ),
			              m_referenceViscosity( referenceViscosity )
{
   m_lithoMap.insert( m_lithoMap.end(), lithoMap, lithoMap + size );
}

/*!
 * \brief constructor of the grid for Walderhaug chemical compaction model
 */
inline MockGrid::MockGrid( const int size,
      const double previousTime,
      const double currentTime,
      double * chemicalCompaction,
      const double * porosity,
      const double * previousTemperature,
      const double * currentTemperature,
      const int numberValidNodes,
      const int * validNodes,
      const int * lithoMap,
      const int numberLithologies,
      const double * fractionQuartz,
      const double * grainSize,
      const double * coatingClay ) :
			         m_size( size ),
			         m_previousTime( previousTime ),
			         m_currentTime( currentTime ),
			         m_chemicalCompaction( chemicalCompaction  ),
			         m_porosity( porosity ),
			         m_previousTemperature( previousTemperature ),
			         m_currentTemperature( currentTemperature ),
			         m_numberValidNodes( numberValidNodes ),
			         m_validNodes( validNodes ),
			         m_numberLithologies( numberLithologies ),
			         m_fractionQuartz( fractionQuartz ),
			         m_grainSize( grainSize ),
			         m_coatingClay( coatingClay )
{
   m_lithoMap.insert( m_lithoMap.end(), lithoMap, lithoMap + size );
}

#endif
