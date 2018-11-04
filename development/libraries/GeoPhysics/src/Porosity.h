//
// Copyright (C) 2015-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef _GEOPHYSICS__POROSITY_H_
#define _GEOPHYSICS__POROSITY_H_

#include <iostream>
using namespace std;
#include <boost/shared_ptr.hpp>
#include <string>

#include "Interface/Interface.h"

#include "ArrayDefinitions.h"

namespace GeoPhysics
{
   // The argument of this attribute specifies the alignment for what the pointer points to.
   // This keyword can be added to a pointer typedef declaration to specify the alignment value of pointers declared for that pointer type.
   // These typedefs are required to tell the compiler (in addition to the #pragma simd aligned) that the arrays provided in input
   // and coming from a different translation unit have aligned memory allocation.


   /*! \class Porosity
    * \brief Abstract class defining the algorithm to compute the porosity value
    *
    * Porosity is computed taking into account the mechanical and chemical compactions.
    * The mechanical compaction can be modelled with soil mechanics, single exponential or double exponential models. (October 2014)
    *
    */
   class Porosity
   {
   public:

      /// \brief Tolerance used in the iterative method used to calculate the solid thickness.
      static const double SolidThicknessIterationTolerance;

      /*!
       * Create porosity object.
       */
      Porosity();
      typedef DataAccess::Interface::PorosityModel Model;

      /*!
       * \brief Create porosity model.
       *
       * Either SoilMechaniscs, Exponential or DoubleExponential class through Factory design pattern.
       * Intended to be used from SimpleLithology and CompoundLithology after mixing
       *
       * \param PorosityModel Model to compute porosity. Can be soil mechanics, single exponential or double exponential (October 2014).
       * \param depoPorosity Porosity at deposition [fraction of volume]
       * \param compactionIncr Compaction coefficient during loading phase for single exponential model [Pa-1]
       * \param compactionIncrA First compaction coefficient during loading phase for double exponential model [Pa-1]
       * \param compactionIncrB Second compaction coefficient during loading phase for double exponential model [Pa-1]
       * \param compactionDecr Compaction coefficient during stress release phase for single exponential model [Pa-1]
       * \param compactionDecrA First compaction coefficient during stress release phase for double exponential model [Pa-1]
       * \param compactionDecrB Second compaction coefficient during stress release phase for double exponential model [Pa-1]
       * \param compactionRatio Compaction ratio used for double exponential model 
       * \param soilMechanicsCompactionCoefficient Compaction coefficient for soil mechanics model [1]
       */
      static Porosity create(Model PorosityModel,
         double depoPorosity,
         double minimumMechanicalPorosity,
         double compactionIncr,
         double compactionIncrA,
         double compactionIncrB,
         double compactionDecr,
         double compactionDecrA,
         double compactionDecrB,
         double compactionRatio,
         double soilMechanicsCompactionCoefficient,
         bool   isLegacy);

      /*!
       * \brief Compute the porosity function.
       *
       * \param ves Vertical Effective Stress in [Pa]
       * \param maxVes Maximum VES reached until current time. If equals to the ves, it is a loading phase in [Pa]
       * \param includeChemicalCompaction Is chemical compaction allowed?
       * \param chemicalCompactionTerm Value of chemical compaction property, ie volume of quartz cemented  [fraction of volume]
       *
       */
      double calculate( const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm ) const;

      /*!
      * \brief Get the derivative of porosity at the current time [Pa-1]
      *
      * \param ves Vertical Effective Stress [Pa]
      * \param maxVes Maximum VES reached until current time. If equals to the ves, it is a loading phase [Pa]
      * \param includeChemicalCompaction Is chemical compaction allowed?
      * \param chemicalCompactionTerm Value of chemical compaction property, ie volume of quartz cemented  [fraction of volume]
      *
      */
      double calculateDerivative( const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm ) const;

      /*!
      * \brief Get the porosity at the current time [Pa-1] (vectorized version)
      *
      * \param n Number of values to be computed
      * \param ves Vertical Effective Stress [Pa]
      * \param maxVes Maximum VES reached until current time. If equals to the ves, it is a loading phase [Pa]
      * \param includeChemicalCompaction Is chemical compaction allowed?
      * \param chemicalCompactionTerm Value of chemical compaction property, ie volume of quartz cemented  [fraction of volume]
      * \param porosities Values of porosity
      * \param porosityDers Values of porosity derivative
      *
      * \pre arrays MUST BE ALIGNED
      */
      void calculate( const unsigned int       n,
                      ArrayDefs::ConstReal_ptr ves,
                      ArrayDefs::ConstReal_ptr maxVes,
                      const bool               includeChemicalCompaction,
                      ArrayDefs::ConstReal_ptr chemicalCompactionTerm,
                      ArrayDefs::Real_ptr      porosities ) const;

      /*!
      * \brief Get the porosity and its derivative at the current time [Pa-1] (vectorized version)
      *
      * \param n Number of values to be computed
      * \param ves Vertical Effective Stress [Pa]
      * \param maxVes Maximum VES reached until current time. If equals to the ves, it is a loading phase [Pa]
      * \param includeChemicalCompaction Is chemical compaction allowed?
      * \param chemicalCompactionTerm Value of chemical compaction property, ie volume of quartz cemented  [fraction of volume]
      * \param porosities Values of porosity
      * \param porosityDers Values of porosity derivative
      *
      * \pre arrays MUST BE ALIGNED
      */
      void calculate( const unsigned int n,
                      ArrayDefs::ConstReal_ptr ves,
                      ArrayDefs::ConstReal_ptr maxVes,
                      const bool includeChemicalCompaction,
                      ArrayDefs::ConstReal_ptr chemicalCompactionTerm,
                      ArrayDefs::Real_ptr porosities,
                      ArrayDefs::Real_ptr porosityDers ) const;

      /// @brief Overwrite default assginment operator to avoid bitwise copy
      Porosity& operator= (const Porosity& porosity);
      /// @brief Overwrite default copy constructor to avoid bitwise copy
      Porosity( const Porosity& porosity );

      /*!
       * \brief Get the porosity model
       */
      Model getPorosityModel()    const;

      /*!
       * \brief Get the porosity at deposition [fraction of volume]
       */
      double getSurfacePorosity() const;

      /*!
       * \brief Get the full thickness value, i.e. the thickness of the solid material [m]
       *
       * \param maxVesValue Maximum VES reached until current time. If equals to the ves, it is a loading phase in [Pa]
       * \param thickness Current thickness of the layer [m]
       * \param densitydiff Difference between the density of the rock and the density of the fluid in porosity [kg/m3]
       * \param vesScaleFactor Based on the amount of ves there would be if the basin was hydrostatically pressured [1]
       * \param overpressuredCompaction Is the basin overpressured?
       */
      double getFullCompThickness(const double maxVesValue, const double thickness, const double densitydiff, const double vesScaleFactor, const bool overpressuredCompaction) const;

      /*!
       * \brief Get the compaction coefficient during loading phase for single exponential model [Pa-1] or the compaction coefficient for soil mechanics model [1]
       */
      double getCompactionCoefficient() const;

      /*!
       * \brief Get the first compaction coefficient during loading phase for double exponential model [Pa-1]
       */
      double getCompactionCoefficientA() const;

      /*!
       * \brief Get the second compaction coefficient during loading phase for double exponential model [Pa-1]
       */
      double getCompactionCoefficientB() const;

      /*!
      * \brief Get the compaction ratio used for double exponential model
      */
      double getCompactionRatio() const;

      /*!
       * \brief Get the minimum porosity that the model can reach (for example 3%) [fraction of volume]
       */
      double getMinimumMechanicalPorosity() const;

      /*! \class Algorithm
       * \brief Abstract class. Compute porosity.
       *
       * Allow to compute porosity whith one of the models: soil mechanics, single exponential or double exponential (October 2014).
       * These algorithms are modeling mechanical compaction. Chemical compaction can also be added if relevant.
       *
       */
      class Algorithm
      {
      public:
        Algorithm (double depoPorosity, double minimumMechanicalPorosity);
         virtual ~Algorithm() {/*Intentionally unimplemented*/}

         /*!
          * \brief Compute the porosity value [fraction of volume]
          */
         virtual double calculate(const double ves, const double maxVes,
                                  const bool includeChemicalCompaction,
                                  const double chemicalCompactionTerm) const = 0;

         /*!
         * \brief Return the porosity derivative [Pa-1]
         */
         virtual double calculateDerivative( const double ves, const double maxVes,
                                             const bool includeChemicalCompaction,
                                             const double chemicalCompactionTerm ) const = 0;

         /*!
         * \brief Return the porosity (vectorized version)
         * \pre arrays MUST BE ALIGNED
         */
         virtual void calculate( const unsigned int n,
                                 ArrayDefs::ConstReal_ptr ves,
                                 ArrayDefs::ConstReal_ptr maxVes,
                                 const bool includeChemicalCompaction,
                                 ArrayDefs::ConstReal_ptr chemicalCompactionTerm,
                                 ArrayDefs::Real_ptr porosities ) const = 0;

         /*!
         * \brief Return the porosity and its derivative [Pa-1] (vectorized version)
         * \pre arrays MUST BE ALIGNED
         */
         virtual void calculate( const unsigned int n,
                                 ArrayDefs::ConstReal_ptr ves,
                                 ArrayDefs::ConstReal_ptr maxVes,
                                 const bool includeChemicalCompaction,
                                 ArrayDefs::ConstReal_ptr chemicalCompactionTerm,
                                 ArrayDefs::Real_ptr porosities,
                                 ArrayDefs::Real_ptr porosityDers ) const = 0;

         /*!
          * \brief Determine if the porosity model is incompressible.
          */
         virtual bool isIncompressible () const = 0;

         /*!
          * \brief Return the deposition porosity [fraction of volume]
          */
         double surfacePorosity() const;

         /*!
          * \brief Return the porosity model
          */
         virtual Model model() const = 0;

         /*!
          * \brief Return the solid thickness of the layer [m]
          */
         virtual double  fullCompThickness(const double MaxVesValue,
                                           const double thickness,
                                           const double densitydiff,
                                           const double vesScaleFactor,
                                           const bool overpressuredCompaction) const;

         /*!
          * \brief Return the compaction coefficient during loading phase for single exponential model [Pa-1] or the compaction coefficient for soil mechanics model [1]
          */
         virtual double compactionCoefficient() const = 0;

         /*!
          * \brief Return the first compaction coefficient during loading phase for double exponential model [Pa-1]
          */
         virtual double compactionCoefficientA () const =0;

         /*!
          * \brief Return the second compaction coefficient during loading phase for double exponential model [Pa-1]
          */
         virtual double compactionCoefficientB () const =0;

         /*!
         * \brief Return the compaction ratio used for double exponential model
         */
         virtual double compactionRatio() const = 0;

         /*!
          * \brief Return the minimum porosity [fraction of volume]
          */
         double  minimumMechanicalPorosity( ) const;

      protected:

         const double m_depoPorosity;                       /*!< Porosity at deposition */
         const double m_minimumMechanicalPorosity;          /*!< Minimum attainable porosity */
         bool   m_isLegacy;                           /*!< Legacy behaviour for minimum porosity?
                                                            * Flag for new rock property library (and new migration engine)
                                                            * 0 is the revised minimum porosity behaviour and additional mixing models
                                                            * 1 is simple minimum porosity behaviour and 2 mixing models*/

      };

   private:

      /*!
       * Default constructor is a pointer to the Algorithm class, which have only public virtual functions
       * \param algorithm. Model of mechanical compaction defining the computation of porosity
       */
      explicit Porosity(Algorithm * algorithm);

      // The pointer pointing to the object that can do the computations.
      // It can be a shared pointer, because all its methods are constant.
      boost::shared_ptr< Algorithm > m_algorithm; /*!< Algorithm used to compute porosity */

   };

   //////////////////////////////////////////////
   // Inline functions

   inline double
   Porosity::Algorithm::minimumMechanicalPorosity( ) const
   {
      return m_minimumMechanicalPorosity;
   }

   inline double
   Porosity::Algorithm::surfacePorosity() const
   {
      return m_depoPorosity;
   }

   inline DataAccess::Interface::PorosityModel Porosity
      ::getPorosityModel() const
   {
      return m_algorithm->model();
   }

   inline double Porosity
      ::getSurfacePorosity() const
   {
      return m_algorithm->surfacePorosity();
   }

   inline double Porosity
      ::calculate( const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm ) const
   {
      return m_algorithm->calculate(ves, maxVes, includeChemicalCompaction, chemicalCompactionTerm);
   }

   inline double Porosity
      ::getFullCompThickness(const double maxVesValue, const double thickness, const double densitydiff, const double vesScaleFactor, const bool overpressuredCompaction) const
   {
      return m_algorithm->fullCompThickness(maxVesValue, thickness, densitydiff, vesScaleFactor, overpressuredCompaction);
   }

   inline double Porosity
      ::getCompactionCoefficient() const{
      return m_algorithm->compactionCoefficient();
   }

   inline double Porosity
         ::getCompactionCoefficientA() const{
         return m_algorithm->compactionCoefficientA();
      }

   inline double Porosity
         ::getCompactionCoefficientB() const{
         return m_algorithm->compactionCoefficientB();
      }

   inline double Porosity
      ::getCompactionRatio() const {
      return m_algorithm->compactionRatio();
   }

   inline double Porosity
     ::calculateDerivative(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const {
     return m_algorithm->calculateDerivative(ves, maxVes, includeChemicalCompaction, chemicalCompactionTerm);
   }

   inline void Porosity
     ::calculate( const unsigned int n,
                  ArrayDefs::ConstReal_ptr ves,
                  ArrayDefs::ConstReal_ptr maxVes,
                  const bool includeChemicalCompaction,
                  ArrayDefs::ConstReal_ptr chemicalCompactionTerm,
                  ArrayDefs::Real_ptr porosities) const {
     return m_algorithm->calculate(n,ves, maxVes, includeChemicalCompaction, chemicalCompactionTerm, porosities);
   }

   inline void Porosity
     ::calculate( const unsigned int n,
                  ArrayDefs::ConstReal_ptr ves,
                  ArrayDefs::ConstReal_ptr maxVes,
                  const bool includeChemicalCompaction,
                  ArrayDefs::ConstReal_ptr chemicalCompactionTerm,
                  ArrayDefs::Real_ptr porosities,
                  ArrayDefs::Real_ptr porosityDers) const {
     return m_algorithm->calculate(n,ves, maxVes, includeChemicalCompaction, chemicalCompactionTerm, porosities, porosityDers);
   }

   inline double Porosity
      ::getMinimumMechanicalPorosity( ) const{
      return m_algorithm->minimumMechanicalPorosity( );
   }

}
#endif
