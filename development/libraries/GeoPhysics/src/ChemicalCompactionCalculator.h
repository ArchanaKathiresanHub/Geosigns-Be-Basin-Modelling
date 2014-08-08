/**
 * \file ChemicalCompactionCalculator.h
 * \brief ChemicalCompactionCalculator class which compute quartz cementation in a sedimentary basin
 * \author Magali Cougnenc magali.cougnenc@pds.nl
 * \date August 2014
 *
 * Refactoring of previous chemical compaction model [Schneider, 1996] which used to be computed through different classes
 * Addition of a new model: Walderhaug model [Walderhaug, 1996]
 *
 */

#ifndef _GEOPHYSICS__CHEMICALCOMPACTIONCALCULATOR_H_
#define _GEOPHYSICS__CHEMICALCOMPACTIONCALCULATOR_H_

#include <vector>
#include <string>


namespace GeoPhysics{

/*! \class ChemicalCompactionCalculator
 * \brief Abstract class containing the algorithm structure of quartz cementation
 *
 * Quartz cementation, also called chemical compaction is computed through derived classes
 * which are SchneiderCompactionCalculator and WalderhaugCompactionCalculator (August 2014)
 *
 */
class ChemicalCompactionCalculator
{
public:

   /*! \class Grid
    * \brief Abstract class for the input grid
    */
   class Grid
   {
   public:

      /*!
       * \brief Destructor of the abstract class Grid
       */
      virtual ~ Grid() {}

      /*!
       * \brief Get the previous time in million years [my]
       */
      virtual double getPreviousTime() const = 0;

      /*!
       * \brief Get the current time in million years [my]
       */
      virtual double getCurrentTime() const = 0;

      /*!
       * \brief Get number of nodes in the grid
       */
      virtual int getSize() const = 0;

      /*!
       * \brief Get the active nodes array in the grid
       * The size is the one returned by getNumberOfActiveNodes() ie the number of active nodes
       */
      virtual const int * getActiveNodes() const = 0;

      /*!
       * \brief Get the number of nodes which both are active and belong to chemical compactable layers
       */
      virtual int getNumberOfActiveNodes() const = 0;

      /*!
       * \brief Get the temperature array [celsius degree]
       * The size is the one returned by getSize() ie the number of nodes
       */
      virtual const double * getPreviousTemperature() const = 0;

      /*!
       * \brief Get the temperature array [celsius degree]
       * The size is the one returned by getSize() ie the number of nodes
       */
      virtual const double * getCurrentTemperature() const = 0;

      /*!
       * \brief Get the porosity array [fraction of the total volume]
       * The size is the one returned by getSize() ie the number of nodes
       */ 
      virtual const double * getPorosity() const = 0;

      /*!
       * \brief Get the vertical effective stress array [Pa]
       * The size is the one returned by getSize() ie the number of nodes
       */
      virtual const double * getVES() const = 0;

      /*!
       * \brief Get and set the chemical compaction array [fraction of the total volume]
       * The size is the one returned by getSize() ie the number of nodes
       */
      virtual double * setChemicalCompaction() = 0;

      /*!
       * \brief Get the number of different lithologies
       */
      virtual int getNumberOfLithologies() const = 0;

      /*!
       * \brief Get the array of the reference lithology number of all nodes in the basin
       * The size is the one returned by getSize() ie the number of nodes in the grid
       * \param lithoMap  The array containing the index allowing access to lithology parameters
       */
      virtual void getLithologyMap( std::vector<int> & lithoMap ) const = 0;

      /*!
       * \brief Get the activation energy [kJ/mol]
       * Used by Schneider chemical compaction
       * The size of the array is the one returned by getNumberOfLithologies() ie the number of different lithologies through the basin
       */
      virtual const double * getActivationEnergy() const { return 0; }

      /*!
       * \brief Get the reference viscosity [Pa.s]
       * Used by Schneider chemical compaction
       * The size of the array is the one returned by getNumberOfLithologies() ie the number of different lithologies through the basin
       */
      virtual const double * getReferenceViscosity() const { return 0; }

      /*!
       * \brief Get the size of the grain of quartz [cm]
       * Used by Walderhaug chemical compaction
       * The size of the array is the one returned by getNumberOfLithologies() ie the number of different lithologies through the basin
       */
      virtual const double * getQuartzGrainSize() const { return 0; }

      /*!
       * \brief Get the fraction of quartz [fraction of total volume]
       * Used by Walderhaug chemical compaction
       * The size is the one returned by getNumberOfLithologies() ie the number of different lithologies through the basin
       */
      virtual const double * getQuartzFraction() const { return 0; }

      /*!
       * \brief Get the coating clay factor [fraction of quartz surface area]
       * Used by Walderhaug chemical compaction
       * The size is the one returned by getNumberOfLithologies() ie the number of different lithologies through the basin
       */
      virtual const double * getCoatingClayFactor() const { return 0; }

   };

   /*!
    * \brief Virtual destructor
    * Destructor of the abstract ChemicalCompactionCalculator class
    */
   virtual ~ChemicalCompactionCalculator()
   {
      //Delete in case of " ChemicalCompactionCalculator * chemComp = new SchneiderChemicalCompaction() " for example
   }

   /*!
    * \brief Compute chemical compaction on the basin
    *
    * Compute the quartz cementation on every valid needle of every valid layer of the basin
    * between two snapshots ie between two times
    * 
    * \param grid: Grid &  3D grid contains all (but only) the information needed for the computation
    */
   virtual void computeOnTimeStep( Grid & grid ) = 0;

   /*!
    * \brief Create a chemical compaction object according to the name of the algorithm
    *
    * Return an error if not a valid algorithm name
    * Are valid names: "Walderhaug", "Schneider" (August 2014)
    *
    * \param algorithmName: const std::string &  Name of the algorithm
    */
   static ChemicalCompactionCalculator * create( const std::string & algorithmName );


};

};//end of namespace

#endif
