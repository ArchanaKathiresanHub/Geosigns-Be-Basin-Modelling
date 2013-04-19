#ifndef _GEOPHYSICS__PERMEABILITY_H_
#define _GEOPHYSICS__PERMEABILITY_H_

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#include "Interface/Interface.h"

namespace GeoPhysics
{

   class Permeability
   {
   public:
      typedef DataAccess::Interface::PermeabilityModel
         Model;

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
      double permeability ( const double ves,
                            const double maxVes,
                            const double calculatedPorosityFraction ) const;

      /// Compte the derivative of the permeability function.
      void permeabilityDerivative ( const double  ves,
                                    const double  maxVes,
                                    const double  calculatedPorosityFraction, 
                                          double& permeability, 
                                          double& derivative ) const;

      Model getPermModel() const;
      double getDepoPerm() const;
      double getPermAniso() const;

      /// Abstract class 
      class Algorithm
      {
      public:
         virtual ~Algorithm() {}

         /// Coompute the permeability
         virtual double permeability ( const double ves,
                                       const double maxVes,
                                       const double calculatedPorosityFraction ) const = 0;

         /// Compute the derivative of the permeability function.
         virtual void permeabilityDerivative ( const double  ves,
                                               const double  maxVes,
                                               const double  calculatedPorosityFraction, 
                                                     double& permeability, 
                                                     double& derivative ) const = 0;

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

      DataAccess::Interface::PermeabilityModel m_permeabilityModel;
      double m_depoPermeability;
      double m_permeabilityAniso;
   };

   inline DataAccess::Interface::PermeabilityModel 
   Permeability
      :: getPermModel () const 
   {
      return m_algorithm->model();
   }

   inline double 
   Permeability
      :: getDepoPerm () const 
   {
      return m_algorithm->depoPerm();
   }

   inline double 
   Permeability
      :: getPermAniso () const 
   {
      return m_permeabilityAniso;
   }

   inline double 
   Permeability
      :: permeability( const double ves, const double maxVes, const double calculatedPorosityFraction) const
   {
      return m_algorithm->permeability(ves, maxVes, calculatedPorosityFraction);
   }

   inline void 
   Permeability
      :: permeabilityDerivative( const double ves, const double maxVes, const double calculatedPorosityFraction,
            double & permeability, double & derivative) const
   {
      return m_algorithm->permeabilityDerivative(ves, maxVes, calculatedPorosityFraction, permeability, derivative);
   }

}

#endif
