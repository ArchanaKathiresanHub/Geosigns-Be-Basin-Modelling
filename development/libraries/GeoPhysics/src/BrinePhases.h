//
// Copyright (C) 2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef GEOPHYSICS_BRINE_PHASES_H_
#define GEOPHYSICS_BRINE_PHASES_H_

#include "ArrayDefinitions.h"

namespace GeoPhysics
{
   namespace Brine
   {
      /// Allowed ranges for temperature, pressure and salinity.
      constexpr double s_MinTemperature = 0.0;
      constexpr double s_MaxTemperature = 1500.0;
      constexpr double s_MinPressure    = 0.1;
      constexpr double s_MaxPressure    = 200.0;
      constexpr double s_MinSalinity    = 0.0;
      constexpr double s_MaxSalinity    = 0.35;
      
      /// 11 values of pressure have tabulated transition temperatures.
      constexpr int s_TabulatedTransitions = 11;

      /// Half of the width of the transition region.
      constexpr double s_halfWidth= 20.0;

      /// Values of pressure and the corresponding transition temperature (middle of the transition region)
      /// Table for Marine water, representative of all brines at a given pressure
      /// Values of the temperature at phase change are taken from PVTsim (P in MPa, T in Celsius)
      /// Values are statically allocated aligned in the memory to ARRAY_ALIGNMENT byte boundary
      alignas(ARRAY_ALIGNMENT) constexpr double s_TabulatedBrineP[s_TabulatedTransitions] = {
         GeoPhysics::Brine::s_MinPressure,
         0.2,
         0.6,
         1.0,
         2.0,
         6.0,
         10.0,
         20.0,
         60.0,
         100.0,
         GeoPhysics::Brine::s_MaxPressure };
      alignas(ARRAY_ALIGNMENT) constexpr double s_TabulatedBrineT[s_TabulatedTransitions] = {
         105.0,
         126.0,
         165.0,
         186.0,
         220.0,
         287.0,
         328.0,
         392.0,
         395.0,
         400.0,
         492.0 };

      /// Computes at copmile time the slope for linear interpolation in the transition region.
      template<int idx>
      constexpr double slope()
      {
         static_assert( idx > 0, "Invalid index provided to Brine::slope" );
         static_assert( idx < s_TabulatedTransitions, "Invalid index provided to Brine::slope" );
         return ( s_TabulatedBrineT[idx] - s_TabulatedBrineT[idx-1] ) / ( s_TabulatedBrineP[idx] - s_TabulatedBrineP[idx-1] );
      }

      /// Useful quantities for the interpolation function in the transition region.
      alignas(ARRAY_ALIGNMENT) constexpr double s_interpolRatio[s_TabulatedTransitions-1] = {
         slope<1>(),
         slope<2>(),
         slope<3>(),
         slope<4>(),
         slope<5>(),
         slope<6>(),
         slope<7>(),
         slope<8>(),
         slope<9>(),
         slope<10>()
      };

      /// Computes at copmile time the intercept for linear interpolation in the transition region.
      template<int idx>
      constexpr double intercept()
      {
         static_assert( idx > 0, "Invalid index provided to Brine::intercept" );
         static_assert( idx < s_TabulatedTransitions, "Invalid index provided to Brine::intercept" );
         return s_TabulatedBrineT[idx-1] + s_halfWidth - s_interpolRatio[idx-1] * s_TabulatedBrineP[idx-1];;
      }

      /// Useful quantities for the interpolation function in the transition region.
      alignas(ARRAY_ALIGNMENT) constexpr double s_interpolTerm[s_TabulatedTransitions-1] = {
         intercept<1>(),
         intercept<2>(),
         intercept<3>(),
         intercept<4>(),
         intercept<5>(),
         intercept<6>(),
         intercept<7>(),
         intercept<8>(),
         intercept<9>(),
         intercept<10>()
      };


      /// \brief Phases serves as the base class for all brine properties that use the same approach of dividing
      ///        the parameter space in three regions, aqueous, vapour and an artificially extended transition region.
      class Phases
      {
      public:

         /// Constructor that initializes m_pres vector and assigns values from TabulatedBrineP[] to its elements.
         /// \pre None.
         /// \post Guarantees initialization.
         Phases( const double salinity );
         /// Virtual destructor
         virtual ~Phases();
      
         /// phaseChange() returns the value of a given physical parameter for an input triplet of T, P, S (temperature, pressure, salinity) values.
         /// \pre Input values should be within the defined ranges (see BrinePhases.C). In case they are not they will be internally adjusted to the relevant limits.
         /// \post Guarantees the return of a non-negative number.
         double phaseChange( const double temperature, const double pressure ) const;
         void   phaseChange( const unsigned int n,
                             ArrayDefs::ConstReal_ptr temperature,
                             ArrayDefs::ConstReal_ptr pressure,
                             ArrayDefs::Real_ptr brineProp );

         /// Set size of working vectors
         void setVectorSize( const unsigned int size );

         /// Get size of working vectors
         int getVectorSize();

         /// Uses linear 1D interpolation to return the value of the temperature at the high end of the artificially broadened transition region
         /// using pressure as an argument.
         /// \pre Input value of pressure should be within the allowed pressure range, otherwise the algorithm will effectively extrapolate.
         /// \post Guarantees the return of a temperature value within the range defined by the m_temps[] array.
         static double findT2( const double pressure );
         static void   findT2( const int n,
                               ArrayDefs::ConstReal_ptr pressure,
                               ArrayDefs::Real_ptr t2 );

         /// Subtracts ( 2 x s_halfwidth ) from the input temperature to return the temperature value at the low end of the extended transition region.
         /// \preNone.
         /// \post Guarantees the return of a value which is smaller by ( 2 x s_halfwidth ) compared to the input value.
         static double findT1( const double higherTemperature );
         static void   findT1( const int n,
                               ArrayDefs::ConstReal_ptr higherTemperature,
                               ArrayDefs::Real_ptr t1 );

      protected:
         /// Checks whether all three of the defining physical parameters (T,P,S) are within the allowed ranges and if not adjusts them
         /// \pre Requires any valid values for the triplet passed by value.
         /// \post Guarantees assignment of valid and within the allowed ranges values to the triplet that is passed by reference.
         void   enforceRanges( const int n,
                               ArrayDefs::ConstReal_ptr temperature,
                               ArrayDefs::ConstReal_ptr pressure,
                               ArrayDefs::Real_ptr cutTemp,
                               ArrayDefs::Real_ptr cutPres );
         double enforceTemperatureRanges( const double temperature ) const;
         double enforcePressureRanges( const double pressure ) const;

         /// Salinity
         const double m_salinity;

         /// Counters of vector elements in each state
         int m_acqueousNum;
         int m_vapourNum;
         int m_transitionNum;
      
         /// Indeces of vector elements in each state
         ArrayDefs::Int_ptr m_acqueousIdx;
         ArrayDefs::Int_ptr m_vapourIdx;
         ArrayDefs::Int_ptr m_transitionIdx;

      private:
         /// Size of arrays used in the vectorized function calls
         unsigned int m_size;

         /// Array of values of pressure after ranges have been applied
         ArrayDefs::Real_ptr m_inRangePres;

         /// Array of values of temperature after ranges have been applied
         ArrayDefs::Real_ptr m_inRangeTemp;

         /// Array of values of termperature at low end of transition region
         ArrayDefs::Real_ptr m_lowEndTransitionTemp;

         /// Array of values of termperature at high end of transition region
         ArrayDefs::Real_ptr m_highEndTransitionTemp;

         /// Depending on the ordering of temperature, higherTemperature and lowerTemperature calls the appropriate function to calculate
         /// the value of the brine parameter of interest. It then returns the value returned by that function without further checks.
         /// \pre Requires the triplet of T,P,S to be within the allowed ranges and lowerTemperature < higherTemperature.
         /// \post Guarantees the return of the return value of the appropriate function to be called depending on temperature, higherTemperature and lowerTemperature.
         virtual double chooseRegion ( const double temperature,
                                       const double pressure,
                                       const double higherTemperature,
                                       const double lowerTemperature ) const = 0;
         virtual void   chooseRegion ( const int n,
                                       ArrayDefs::ConstReal_ptr temperature,
                                       ArrayDefs::ConstReal_ptr pressure,
                                       ArrayDefs::ConstReal_ptr higherTemperature,
                                       ArrayDefs::ConstReal_ptr lowerTemperature,
                                       ArrayDefs::Real_ptr brineProp ) const = 0;

         /// Returns the index of the first tabulated pressure greater than the provided value
         static int getTemperatureUpperBoundIdx( const double pressure );

         /// Method for allocating variable size arrays needed by the class
         void allocateVariableArrays( const unsigned int newSize );

         /// Method for deallocating variable size arrays needed by the class
         void deallocateVariableArrays();

         /// Method for deallocating arrays needed by the class
         void updatePhaseStatesVector( const int n, ArrayDefs::ConstReal_ptr );
      };

   } /// end Brine

} /// end GeoPhysics


inline int GeoPhysics::Brine::Phases::getVectorSize()
{
   return m_size;
}


inline double GeoPhysics::Brine::Phases::findT1( const double higherTemperature )
{
   return higherTemperature - 2.0 * s_halfWidth;
}


inline double GeoPhysics::Brine::Phases::enforceTemperatureRanges( const double temperature ) const
{
   return (temperature > s_MaxTemperature) ? s_MaxTemperature : ( temperature < s_MinTemperature ? s_MinTemperature : temperature );
}


inline double GeoPhysics::Brine::Phases::enforcePressureRanges( const double pressure ) const
{
   return (pressure > s_MaxPressure) ? s_MaxPressure : ( pressure < s_MinPressure ? s_MinPressure : pressure );
}

#endif /// GEOPHYSICS_BRINE_PHASES_H_
