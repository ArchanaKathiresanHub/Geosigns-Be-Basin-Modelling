/**
 * \file SchneiderCompactionCalculator.h
 * \brief SchneiderCompactionCalculator class which computes quartz cementation in a sedimentary basin
 * \author Magali Cougnenc magali.cougnenc@pds.nl
 * \date August 2014
 *
 */

#ifndef _GEOPHYSICS__SCHNEIDERCOMPACTIONCALCULATOR_H_
#define _GEOPHYSICS__SCHNEIDERCOMPACTIONCALCULATOR_H_

#include "ChemicalCompactionCalculator.h"

namespace GeoPhysics{


/*! \class SchneiderCompactionCalculator
 * \brief Class computing quartz cementation in the whole basin
 *
 * Class derived from ChemicalCompactionCalculator class.
 * Compute chemical compaction thanks to viscoplastic type equation
 * described in [Schneider, 1996, Mechanical and chemical compaction model for sedimentary basin simulators]
 *
 */
class SchneiderCompactionCalculator : public ChemicalCompactionCalculator
{
public:

   /*!
    * \brief Destructor
    * 
    * Destructor of the SchneiderCompactionCalculator class
    */
   virtual ~SchneiderCompactionCalculator();

   /*!
       * \brief Compute chemical compaction on the basin
       *
       * From a geological point of view, evaluate the quartz cementation of a basin between two times.
       * Compute the chemical compaction on every valid node of every chemically compactable and layer 
       * between two snapshots ie between two times
       * 
       * \param grid: Grid &  3D grid contains all (but only) the information needed for the computation
       * according to the Schneider model
       */
   virtual void computeOnTimeStep( Grid & grid );

private:
   /*!
    * \brief Get the list of lithologies and the lithoMap 
    * 
    * lithoMap is a vector of the lithologies ID for each node of the basin.
    * Create the container and store all the needed data about lithology.
    * 
    * \param grid: const Grid &  3D grid
    */
   void setLithologies( const Grid & grid );

   /*!
    * \brief struct allowing to store the lithology parameters 
    * 
    * Store the activation energy and the reference viscosity, the two lithology parameters required by Schneider model 
    */
   struct SchneiderLithology{
      double m_activationEnergy;
      double m_referenceSolidViscosity;
   };	                                                 
   std::vector < SchneiderLithology > m_lithologyList; /*!< vector where all the lithologies of the basin are stored */
   std::vector < int > m_lithologyMap;                  /*!< vector of int giving the lithology ID for each node of the grid */

};

};//end of namespace

#endif
