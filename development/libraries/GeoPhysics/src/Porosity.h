#ifndef _GEOPHYSICS__POROSITY_H_
#define _GEOPHYSICS__POROSITY_H_
#include<iostream>
using namespace std;
#include <boost/shared_ptr.hpp>
#include <string>

#include "Interface/Interface.h"

namespace GeoPhysics
{

   class Porosity
   {
   public:
      Porosity();
      typedef DataAccess::Interface::PorosityModel Model;

      /// Factory method: Intended to be used from SimpleLithology only and CompoundLithology after mixing 
      static Porosity create(Model PorosityModel,
      double depoPorosity,
      double minimumMechanicalPorosity,
      double compactionincr,
      double compactiondecr,
      double soilMechanicsCompactionCoefficient);

      /// Compute the porosity function
      double porosity(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const;
      /// Get Functions
      Model getPorosityModel()    const;
      double getSurfacePorosity() const;
      double getFullCompThickness(const double MaxVesValue, const double thickness, const double densitydiff, const double vesScaleFactor, const bool overpressuredCompaction) const;
      double getCompactionCoefficent() const;
      double getDVoidRatioDVes(const double computedVoidRatio, const double ves, const double maxVes, const bool loadingPhase) const;
      double getPorosityDerivativeWrtVes(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const;
      double getPorosityDerivative(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const;
      double getDVoidRatioDP(const double ves, const double maxVes, const bool loadingPhase, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const;
      double getMinimumMechanicalPorosity() const;

      /// Abstract class
      class Algorithm
      {
      public:
         virtual ~Algorithm() {}

         /// Compute the permeability
         virtual double porosity(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const = 0;

         /// return the depositional permeability
         virtual double surfacePorosity() const = 0;

         /// Return the permeability model
         virtual Model  model()    const = 0;

         /// Return FullCompThickness
         virtual double  FullCompThickness(const double MaxVesValue, const double thickness, const double densitydiff, const double vesScaleFactor, const bool overpressuredCompaction) const = 0;

         /// Return compactIncrement
         virtual double  CompactionCoefficent() const = 0;

         /// Return ExponentialDVoidRatioDP
         virtual double DVoidRatioDVes(const double computedVoidRatio, const double ves, const double maxVes, const bool loadingPhase) const = 0;

         /// Return PorosityDerivativeWrtVes
         virtual double PorosityDerivativeWrtVes(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const = 0;

         /// Return PorosityDerivative
         virtual double PorosityDerivative(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const = 0;

         /// Return DVoidRatioDP
         virtual double DVoidRatioDP(const double ves, const double maxVes, const bool loadingPhase, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const = 0;
         /// Return MinimumMechanicalPorosity
         virtual double  MinimumMechanicalPorosity( ) const = 0;

      };

   private:

      ///Default constructor is a pointer to the Algorithm class, which have only public virtual functions
      Porosity(Algorithm *);

      // The pointer pointing to the object that can do the computations.
      // It can be a shared pointer, because all its methods are constant.
      boost::shared_ptr< Algorithm > m_algorithm;

   };

   /// functions to return the algorithm value

   inline DataAccess::Interface::PorosityModel
      Porosity
      ::getPorosityModel() const
   {
      return m_algorithm->model();
   }

   inline double
      Porosity
      ::getSurfacePorosity() const
   {
      return m_algorithm->surfacePorosity();
   }

   inline double Porosity
      ::porosity(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const
   {
      return m_algorithm->porosity(ves, maxVes, includeChemicalCompaction, chemicalCompactionTerm);
   }

   inline double Porosity
      ::getFullCompThickness(const double MaxVesValue, const double thickness, const double densitydiff, const double vesScaleFactor, const bool overpressuredCompaction) const
   {
      return m_algorithm->FullCompThickness(MaxVesValue, thickness, densitydiff, vesScaleFactor, overpressuredCompaction);
   }

   inline double Porosity
      ::getCompactionCoefficent() const{
      return m_algorithm->CompactionCoefficent();
   }

   inline double Porosity
      ::getDVoidRatioDVes(const double computedVoidRatio, const double ves, const double maxVes, const bool loadingPhase) const{
      return m_algorithm->DVoidRatioDVes(computedVoidRatio, ves, maxVes, loadingPhase);
   }

   inline double Porosity
      ::getPorosityDerivativeWrtVes(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const{
      return m_algorithm->PorosityDerivativeWrtVes(ves, maxVes, includeChemicalCompaction, chemicalCompactionTerm);
   }

   inline double Porosity
      ::getPorosityDerivative(const double ves, const double maxVes, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const {
      return m_algorithm->PorosityDerivative(ves, maxVes, includeChemicalCompaction, chemicalCompactionTerm);
   }

   inline double Porosity
      ::getDVoidRatioDP(const double ves, const double maxVes, const bool loadingPhase, const bool includeChemicalCompaction, const double chemicalCompactionTerm) const{
      return m_algorithm->DVoidRatioDP(ves, maxVes, loadingPhase, includeChemicalCompaction, chemicalCompactionTerm);
   }

   inline double Porosity
      ::getMinimumMechanicalPorosity( ) const{
      return m_algorithm->MinimumMechanicalPorosity( );
   }

}
#endif

