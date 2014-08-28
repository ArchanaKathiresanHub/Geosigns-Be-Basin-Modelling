/**
 * \file WalderhaugCompactionCalculator.h
 * \brief WalderhaugCompactionCalculator class which computes quartz cementation in a sedimentary basin
 * \author Magali Cougnenc magali.cougnenc@pds.nl
 * \date August 2014
 *
 */

#ifndef _GEOPHYSICS__WALDERHAUGCOMPACTIONCALCULATOR_H_
#define _GEOPHYSICS__WALDERHAUGCOMPACTIONCALCULATOR_H_

#include "ChemicalCompactionCalculator.h"
#include <vector>


namespace GeoPhysics{

/*! \class WalderhaugCompactionCalculator
 * \brief Class computing quartz cementation in the whole basin
 *
 * Class derived from ChemicalCompactionCalculator class.
 * Computes chemical compaction thanks to logarithmic type equation 
 * described in [Walderhaug, 1996, Kinetic modeling of quartz cementation and porosity loss in deeply buried sandstones reservoirs]
 *
 */
class WalderhaugCompactionCalculator : public ChemicalCompactionCalculator
{
public:

   /*!
    * \brief Constructor
    * 
    * Constructor of the WalderhaugCompactionCalculator class
    */
   WalderhaugCompactionCalculator();

   /*!
    * \brief Desctructor
    * 
    * Destructor of the WalderhaugCompactionCalculator class
    */
   virtual ~WalderhaugCompactionCalculator();

   /*!
    * \brief Computes chemical compaction on the basin
    *
    * Computes the quartz cementation on every valid needle of every valid layer of the basin
    * between two snapshots ie between two times.
    *
    * \param previousGrid: Grid & grid contains all (and only) the needed information
    * in order to compute the chemical compaction according to Walderhaug model.
    */
   virtual void computeOnTimeStep( Grid & grid );


private:

   /*!
    * \brief Get the list of lithologies and the lithoMap which is a vector of the lithology IDs for every node of the basin
    * 
    * Creates container and store all the needed data about lithologies
    * \param grid: const Grid &  3D grid
    */
   void setLithologies( const Grid & grid );

   static const double m_coefA;              /*!< double pre-exponential factor */
   static const double m_coefB;              /*!< double exponential factor */
   static const double m_startTemperature;   /*!< double temperature when the compaction starts */
   static const double m_limitTemp;          /*!< double maximal temperature, higher temperature will lead to defect */

   std::vector < int > m_lithologyMap;        /*!< vector of int giving the lithology ID for each pillar of the grid */
   std::vector < double > m_lithologyList;   /*!< vector where all the lithologies of the basin are stored */
   double m_constantCoef;                    /*!< double constant for the whole basin */
   std::vector < double > m_initialPorosity; /*!< vector of double to store the porosity at the beginning of the compaction */
};

};//end of namespace

#endif
