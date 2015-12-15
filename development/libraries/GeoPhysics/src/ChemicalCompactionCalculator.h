/**
 * \file ChemicalCompactionCalculator.h
 * \brief ChemicalCompactionCalculator class which computes quartz cementation (=chemical compaction) in a sedimentary basin
 * \author Magali Cougnenc, email: magali.cougnenc@pds.nl
 * \date August 2014
 *
 * Refactoring of previous chemical compaction model [Schneider, 1996] which used to be computed through different classes.
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
    * \brief Abstract class. Grid mocking the behavior of a basin
    * 
    * Allows the interaction between
    * the code in fem_grid class and the algorithm of chemical compaction.
    */
   class Grid
   {
   public:

      /*!
       * Destructor of the abstract class Grid.
       */
      virtual ~ Grid() {}

      /*!
       * \brief Get the previous time in million years [my].
       * 
       * Return the age of the previous snapshot.
       */
      virtual double getPreviousTime() const = 0;

      /*!
       * \brief Get the current time in million years [my].
       * 
       * Return the age of the next snapshot .
       */
      virtual double getCurrentTime() const = 0;

      /*!
       * \brief Get the number of nodes in the grid.
       * 
       */
      virtual int getSize() const = 0;

      /*!
       * \brief Get the array of active nodes in the grid.
       * 
       * The size of the array is the one returned by getNumberOfActiveNodes() ie the number of active nodes.
       * An active node is part of an active needle, belongs to a chemically compactable layer and is below
       * the current bottom of the sea.
       */
      virtual const int * getActiveNodes() const = 0;

      /*!
       * \brief Get the number of active nodes.
       * 
       * Return the size of the array of active nodes.
       */
      virtual int getNumberOfActiveNodes() const = 0;

      /*!
       * \brief Get the temperature array [celsius degree].
       * 
       * The size of the array is the one returned by getSize() ie the number of nodes.
       */
      virtual const double * getPreviousTemperature() const = 0;

      /*!
       * \brief Get the temperature array [celsius degree].
       * 
       * The size of the array is the one returned by getSize() ie the number of nodes.
       */
      virtual const double * getCurrentTemperature() const = 0;

      /*!
       * \brief Get the porosity array [fraction of the total volume].
       * 
       * The size of the array is the one returned by getSize() ie the number of nodes.
       */ 
      virtual const double * getPorosity() const = 0;

      /*!
       * \brief Get the vertical effective stress array [Pa].
       * 
       * The size of the array is the one returned by getSize() ie the number of nodes.
       */
      virtual const double * getVES() const = 0;

      /*!
       * \brief Get and set the chemical compaction array [fraction of the total volume].
       * 
       * The size of the array is the one returned by getSize() ie the number of nodes.
       */
      virtual double * setChemicalCompaction() = 0;

      /*!
       * \brief Get the number of different lithologies.
       * 
       * Return the number of different CompoundLithology used in the grid which is 
       * only composed of the chemically compactable layers. 
       */
      virtual int getNumberOfLithologies() const = 0;

      /*!
       * \brief Get the array of the reference lithology identification number for every node in the basin.
       * 
       * The size of the array is the one returned by getSize() ie the number of nodes in the grid.
       * \param lithoMap The array containing the identification numbers for every node
       */
      virtual void getLithologyMap( std::vector<int> & lithoMap ) const = 0;

      /*!
       * \brief Get the activation energy [kJ/mol].
       * 
       * Used by the Schneider model of chemical compaction.
       * The size of the array is the one returned by getNumberOfLithologies() ie the number of different lithologies through the grid.
       */
      virtual const double * getActivationEnergy() const { return 0; }

      /*!
       * \brief Get the reference viscosity [Pa.s].
       * 
       * Used by the Schneider model of chemical compaction.
       * The size of the array is the one returned by getNumberOfLithologies() ie the number of different lithologies through the grid.
       */
      virtual const double * getReferenceViscosity() const { return 0; }

      /*!
       * \brief Get the size of the grain of quartz [cm].
       * 
       * Used by the Walderhaug model of chemical compaction.
       * The size of the array is the one returned by getNumberOfLithologies() ie the number of different lithologies through the grid.
       */
      virtual const double * getQuartzGrainSize() const { return 0; }

      /*!
       * \brief Get the fraction of quartz [fraction of total volume].
       * 
       * Used by the Walderhaug model of chemical compaction.
       * The size of the array is the one returned by getNumberOfLithologies() ie the number of different lithologies through the grid.
       */
      virtual const double * getQuartzFraction() const { return 0; }

      /*!
       * \brief Get the coating clay factor [fraction of quartz surface area].
       * 
       * Used by the Walderhaug model of chemical compaction.
       * The size of the array is the one returned by getNumberOfLithologies() ie the number of different lithologies through the grid.
       */
      virtual const double * getClayCoatingFactor() const { return 0; }

   };

   /*!
    * \brief Virtual destructor
    * 
    * Destructor of the abstract ChemicalCompactionCalculator class.
    */
   virtual ~ChemicalCompactionCalculator()
   {
      //Delete in case of " ChemicalCompactionCalculator * chemComp = new SchneiderChemicalCompaction() " for example
   }

   /*!
    * \brief Compute chemical compaction on the basin.
    *
    * From a geological point of view, evaluate the quartz cementation of a basin between two times.
    * Compute the chemical compaction on every valid node of every chemically compactable and layer between two snapshots ie between two times.
    * 
    * \param grid: Grid &  3D grid contains all (but only) the information needed for the computation
    */
   virtual void computeOnTimeStep( Grid & grid ) = 0;

   /*!
    * \brief Create a chemical compaction object according to the name of the algorithm.
    *
    * Return an error if not a valid algorithm name.
    * Valid names are "Walderhaug", "Schneider" (August 2014)
    *
    * \param algorithmName: const std::string &  Name of the algorithm
    */
   static ChemicalCompactionCalculator * create( const std::string & algorithmName );


};

};//end of namespace

#endif
