//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef _GEOPHYSICS__PERMEABILITY_H_
#define _GEOPHYSICS__PERMEABILITY_H_

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#include "Interface.h"

#include "ArrayDefinitions.h"

namespace GeoPhysics
{

   class Permeability
   {
   public:
      typedef DataAccess::Interface::PermeabilityModel Model;

      /// Factory method: Intended to be used from SimpleLithology only
      static Permeability create(
            Model permeabilityModel, double permAniso,
            double depoPorosityPercentage, double depoPermeability,
            double permeabilityIncr, double permeabilityDecr,
            const std::string & porosityMultiPointSamplesString,
            const std::string & permeabilityMultiPointSamplesString,
            int numberOfMultiPointSamples
            );

      /// Factory methods: Intended to be used from LithologyManager only
      static Permeability createMultiPoint(
            double permAniso,
            double depoPorosityPercentage,
            const std::vector<double> &porosityPercentageSamples,
            const std::vector<double> & permeabilitySamples
            );

      /// Compute the permeability function
      double calculate ( const double ves,
                         const double maxVes,
                         const double calculatedPorosityFraction ) const;


      /// \brief Compute the permeability for an array of values
      ///
      /// \param  [in] n                     The number of values in the array.
      /// \param  [in] ves                   The ves values for which the permeability is to be calculated.
      /// \param  [in] maxVes                The max-ves values for which the permeability is to be calculated.
      /// \param  [in] calculatedPorosities  The porosity values for which the permeability is to be calculated.
      /// \param [out] permeabilities        The calculated permeability values.
      /// \pre The dimension of all vectors must be n.
      void calculate ( const unsigned int       n,
                       ArrayDefs::ConstReal_ptr ves,
                       ArrayDefs::ConstReal_ptr maxVes,
                       ArrayDefs::ConstReal_ptr calculatedPorosity,
                       ArrayDefs::Real_ptr      permeabilities ) const;

      /// Compte the derivative of the permeability function.
      void calculateDerivative ( const double  ves,
                                 const double  maxVes,
                                 const double  calculatedPorosityFraction,
                                 const double  porosityDerivativeWrtVes,
                                       double& permeability,
                                       double& derivative ) const;

      /// \brief Compute the permeability and its derivative for an array of values.
      ///
      /// \param  [in] n                         The number of values in the array.
      /// \param  [in] ves                       The ves values for which the permeability is to be calculated.
      /// \param  [in] maxVes                    The max-ves values for which the permeability is to be calculated.
      /// \param  [in] calculatedPorosities      The porosity values for which the permeability is to be calculated.
      /// \param  [in] porosityDerivativeWrtVes  The derivative of the porosity for which the permeability is to be calculated.
      /// \param [out] permeabilities            The calculated permeability values.
      /// \param [out] derivatives               The calculated permeability derivative values.
      /// \pre The dimension of all vectors must be n.
      void calculateDerivative ( const unsigned int       n,
                                 ArrayDefs::ConstReal_ptr ves,
                                 ArrayDefs::ConstReal_ptr maxVes,
                                 ArrayDefs::ConstReal_ptr calculatedPorosity,
                                 ArrayDefs::ConstReal_ptr porosityDerivativeWrtVes,
                                 ArrayDefs::Real_ptr      permeabilities,
                                 ArrayDefs::Real_ptr      derivatives ) const;

      /// @brief Overwrite default assginment operator to avoid bitwise copy
      Permeability& operator= (const Permeability& permeability);
      /// @brief Overwrite default copy constructor to avoid bitwise copy
      Permeability( const Permeability& permeability );

      Model  getPermModel() const;
      double getDepoPerm()  const;
      double getPermAniso() const;

      /// Abstract class
      class Algorithm
      {
      public:
         virtual ~Algorithm() {}

         /// Coompute the permeability
         virtual double calculate( const double ves,
                                   const double maxVes,
                                   const double calculatedPorosityFraction ) const = 0;

         /// \brief Compute the permeability for an array of values.
         ///
         /// \param  [in] n                     The number of values in the array.
         /// \param  [in] ves                   The ves values for which the permeability is to be calculated.
         /// \param  [in] maxVes                The max-ves values for which the permeability is to be calculated.
         /// \param  [in] calculatedPorosities  The porosity values for which the permeability is to be calculated.
         /// \param [out] permeabilities        The calculated permeability values.
         /// \pre The dimension of all vectors must be n.
         virtual void calculate ( const unsigned int       n,
                                  ArrayDefs::ConstReal_ptr ves,
                                  ArrayDefs::ConstReal_ptr maxVes,
                                  ArrayDefs::ConstReal_ptr calculatedPorosity,
                                  ArrayDefs::Real_ptr      permeabilities ) const = 0;

         /// Compute the derivative of the permeability function.
         virtual void calculateDerivative ( const double  ves,
                                            const double  maxVes,
                                            const double  calculatedPorosityFraction,
                                            const double  porosityDerivativeWrtVes,
                                                  double& permeability,
                                                  double& derivative ) const = 0;

         /// \brief Compute the permeability and its derivative for an array of values.
         ///
         /// \param  [in] n                         The number of values in the array.
         /// \param  [in] ves                       The ves values for which the permeability is to be calculated.
         /// \param  [in] maxVes                    The max-ves values for which the permeability is to be calculated.
         /// \param  [in] calculatedPorosities      The porosity values for which the permeability is to be calculated.
         /// \param  [in] porosityDerivativeWrtVes  The derivative of the porosity for which the permeability is to be calculated.
         /// \param [out] permeabilities            The calculated permeability values.
         /// \param [out] derivatives               The calculated permeability derivative values.
         virtual void calculateDerivative ( const unsigned int       n,
                                            ArrayDefs::ConstReal_ptr ves,
                                            ArrayDefs::ConstReal_ptr maxVes,
                                            ArrayDefs::ConstReal_ptr calculatedPorosity,
                                            ArrayDefs::ConstReal_ptr porosityDerivativeWrtVes,
                                            ArrayDefs::Real_ptr      permeabilities,
                                            ArrayDefs::Real_ptr      derivatives ) const = 0;

         /// return the depositional permeability
         virtual double depoPerm() const = 0;

         /// Return the permeability model
         virtual Model  model()    const = 0;
      };

   private:

      Permeability( double permAniso, Algorithm * );

      static std::vector<double> parseCoefficientsFromString( const std::string & string);

      // The pointer pointing to the object that can do the computations.
      // It can be a shared pointer, because all its methods are constant.
      boost::shared_ptr< Algorithm > m_algorithm;

      DataAccess::Interface::PermeabilityModel m_permeabilityModel; ///< The permeability model
      double m_depoPermeability;  ///< The depositional permeability
      double m_permeabilityAniso; ///< The permeability anisotropy
   };


   ///////////////////////////////////////////////
   // Inline functions

   inline DataAccess::Interface::PermeabilityModel Permeability::getPermModel () const
   {
      return m_algorithm->model();
   }

   inline double Permeability::getDepoPerm () const
   {
      return m_algorithm->depoPerm();
   }

   inline double Permeability::getPermAniso () const
   {
      return m_permeabilityAniso;
   }

   inline double Permeability::calculate( const double ves, const double maxVes, const double calculatedPorosityFraction ) const
   {
      return m_algorithm->calculate( ves, maxVes, calculatedPorosityFraction );
   }

   inline void Permeability::calculate ( const unsigned int       n,
                                         ArrayDefs::ConstReal_ptr ves,
                                         ArrayDefs::ConstReal_ptr maxVes,
                                         ArrayDefs::ConstReal_ptr calculatedPorosity,
                                         ArrayDefs::Real_ptr      permeabilities ) const {
      m_algorithm->calculate ( n, ves, maxVes, calculatedPorosity, permeabilities );
   }


   inline void Permeability::calculateDerivative ( const double ves,
                                                   const double maxVes,
                                                   const double calculatedPorosityFraction,
                                                   const double porosityDerivativeWrtVes,
                                                   double&      permeability,
                                                   double&      derivative ) const
   {
      m_algorithm->calculateDerivative( ves, maxVes, calculatedPorosityFraction, porosityDerivativeWrtVes, permeability, derivative );
   }

   inline void Permeability::calculateDerivative ( const unsigned int       n,
                                                   ArrayDefs::ConstReal_ptr ves,
                                                   ArrayDefs::ConstReal_ptr maxVes,
                                                   ArrayDefs::ConstReal_ptr calculatedPorosity,
                                                   ArrayDefs::ConstReal_ptr porosityDerivativeWrtVes,
                                                   ArrayDefs::Real_ptr      permeabilities,
                                                   ArrayDefs::Real_ptr      derivatives ) const {
      m_algorithm->calculateDerivative ( n, ves, maxVes, calculatedPorosity, porosityDerivativeWrtVes, permeabilities, derivatives );
   }

}

#endif
