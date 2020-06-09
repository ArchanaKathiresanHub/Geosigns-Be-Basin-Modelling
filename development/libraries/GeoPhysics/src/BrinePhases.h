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
         return s_TabulatedBrineT[idx-1] + s_halfWidth - s_interpolRatio[idx-1] * s_TabulatedBrineP[idx-1];
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


      /// \brief PhaseStateBase serves as the base class for all brine properties that use the same approach of dividing
      ///        the parameter space in three regions, aqueous, vapour and an artificially extended transition region.
      class PhaseStateBase
      {
      public:

         /// Constructor that initializes m_pres vector and assigns values from TabulatedBrineP[] to its elements.
         /// \pre None.
         /// \post Guarantees initialization.
         explicit PhaseStateBase( const double salinity );

         /// Destructor
         ~PhaseStateBase();

         /// Get salinity
         double getSalinity() const;

         /// Get salinity
         void setSalinity( const double salinity );

         /// Uses linear 1D interpolation to return the value of the temperature at the high end of the artificially broadened transition region
         /// using pressure as an argument.
         /// \pre Input value of pressure should be within the allowed pressure range, otherwise the algorithm will effectively extrapolate.
         /// \post Guarantees the return of a temperature value within the range defined by the m_temps[] array.
         static double findT2( const double pressure );

         /// Subtracts ( 2 x s_halfwidth ) from the input temperature to return the temperature value at the low end of the extended transition region.
         /// \preNone.
         /// \post Guarantees the return of a value which is smaller by ( 2 x s_halfwidth ) compared to the input value.
         static double findT1( const double higherTemperature );

      protected:

         /// Checks whether all three of the defining physical parameters (T,P,S) are within the allowed ranges and if not adjusts them
         /// \pre Requires any valid values for the triplet passed by value.
         /// \post Guarantees assignment of valid and within the allowed ranges values to the triplet that is passed by reference.
         static double enforceTemperatureRanges( const double temperature );
         static double enforcePressureRanges( const double pressure );
         static double enforceSalinityRanges( const double salinity );
         
         /// Returns the index of the first tabulated pressure greater than the provided value
         static int getTemperatureUpperBoundIdx( const double pressure );

         /// Salinity
         double m_salinity;
      };


      /// \brief PhaseStateScalar serves as the base class for all brine properties that use the same approach of dividing
      ///        the parameter space in three regions, aqueous, vapour and an artificially extended transition region.
      class PhaseStateScalar : public PhaseStateBase
      {
      public:

         /// Constructor that initializes m_pres vector and assigns values from TabulatedBrineP[] to its elements.
         /// \pre None.
         /// \post Guarantees initialization.
         explicit PhaseStateScalar( const double salinity );

         /// Destructor
         ~PhaseStateScalar();
      
         /// phaseChange() returns the value of a given physical parameter for an input triplet of T, P, S (temperature, pressure, salinity) values.
         /// \pre Input values should be within the defined ranges (see BrinePhases.cpp). In case they are not they will be internally adjusted to the relevant limits.
         /// \post Guarantees the return of a non-negative number.
         void set( const double temperature, const double pressure );

         /// Get pressure value
         double getPressure() const;

         /// Get temperature value
         double getTemperature() const;

         /// Get temperature value
         double getLowEndTransitionTemp() const;

         /// Get temperature value
         double getHighEndTransitionTemp() const;

      private:

         /// Pressure after ranges have been applied
         double m_inRangePres;

         /// Temperature after ranges have been applied
         double m_inRangeTemp;

         /// Termperature at low end of transition region
         double m_lowEndTransitionTemp;

         /// Termperature at high end of transition region
         double m_highEndTransitionTemp;
      };


      /// \brief PhaseStateVec serves as the base class for all brine properties that use the same approach of dividing
      ///        the parameter space in three regions, aqueous, vapour and an artificially extended transition region.
      class PhaseStateVec : public PhaseStateBase
      {
      public:

         /// Constructor that initializes m_pres vector and assigns values from TabulatedBrineP[] to its elements.
         /// \pre None.
         /// \post Guarantees initialization.
         explicit PhaseStateVec( const unsigned int n, const double salinity );

         /// Destructor
         ~PhaseStateVec();
      
         /// phaseChange() returns the value of a given physical parameter for an input triplet of T, P, S (temperature, pressure, salinity) values.
         /// \pre Input values should be within the defined ranges (see BrinePhases.cpp). In case they are not they will be internally adjusted to the relevant limits.
         /// \post Guarantees the return of a non-negative number.
         void set( const unsigned int n,
                   ArrayDefs::ConstReal_ptr temperature,
                   ArrayDefs::ConstReal_ptr pressure );

         /// Get size of working vectors
         int getVectorSize() const;

         /// Get number of phases in the aqueous state
         int getAqueousPhasesNum() const;

         /// Get number of phases in the vapour state
         int getVapourPhasesNum() const;

         /// Get number of phases in the transition state
         int getTransitionPhasesNum() const;

         /// Get aqueous state indices
         ArrayDefs::Int_ptr getAqueousIndices() const;

         /// Get vapour state indices
         ArrayDefs::Int_ptr getVapourIndices() const;

         /// Get transition state indices
         ArrayDefs::Int_ptr getTransitionIndices() const;

         /// Get pressure values
         ArrayDefs::Real_ptr getPressure() const;

         /// Get temperature values
         ArrayDefs::Real_ptr getTemperature() const;

         /// Get temperature values
         ArrayDefs::Real_ptr getLowEndTransitionTemp() const;

         /// Get temperature values
         ArrayDefs::Real_ptr getHighEndTransitionTemp() const;

         /// Uses linear 1D interpolation to return the value of the temperature at the high end of the artificially broadened transition region
         /// using pressure as an argument.
         /// \pre Input value of pressure should be within the allowed pressure range, otherwise the algorithm will effectively extrapolate.
         /// \post Guarantees the return of a temperature value within the range defined by the m_temps[] array.
         static void findT2( const int n,
                             ArrayDefs::ConstReal_ptr pressure,
                             ArrayDefs::Real_ptr t2 );

         /// Subtracts ( 2 x s_halfwidth ) from the input temperature to return the temperature value at the low end of the extended transition region.
         /// \preNone.
         /// \post Guarantees the return of a value which is smaller by ( 2 x s_halfwidth ) compared to the input value.
         static void findT1( const int n,
                             ArrayDefs::ConstReal_ptr higherTemperature,
                             ArrayDefs::Real_ptr t1 );

      private:
         /// Checks whether all three of the defining physical parameters (T,P,S) are within the allowed ranges and if not adjusts them
         /// \pre Requires any valid values for the triplet passed by value.
         /// \post Guarantees assignment of valid and within the allowed ranges values to the triplet that is passed by reference.
         void enforceRanges( ArrayDefs::ConstReal_ptr temperature,
                             ArrayDefs::ConstReal_ptr pressure,
                             ArrayDefs::Real_ptr cutTemp,
                             ArrayDefs::Real_ptr cutPres );

         /// Counters of vector elements in each state
         int m_aqueousNum;
         int m_vapourNum;
         int m_transitionNum;
      
         /// Indices of vector elements in each state
         ArrayDefs::Int_ptr m_aqueousIdx;
         ArrayDefs::Int_ptr m_vapourIdx;
         ArrayDefs::Int_ptr m_transitionIdx;

         /// Size of arrays used in the vectorized function calls
         const unsigned int m_size;

         /// Array of values of pressure after ranges have been applied
         ArrayDefs::Real_ptr m_inRangePres;

         /// Array of values of temperature after ranges have been applied
         ArrayDefs::Real_ptr m_inRangeTemp;

         /// Array of values of termperature at low end of transition region
         ArrayDefs::Real_ptr m_lowEndTransitionTemp;

         /// Array of values of termperature at high end of transition region
         ArrayDefs::Real_ptr m_highEndTransitionTemp;

         /// Method for allocating variable size arrays needed by the class
         void allocateArrays();

         /// Method for deallocating variable size arrays needed by the class
         void deallocateArrays();

         /// Method for deallocating arrays needed by the class
         void updatePhaseStatesVector( ArrayDefs::ConstReal_ptr );
      };

   } /// end Brine

} /// end GeoPhysics


/// PhaseStateBase inline methods

inline double GeoPhysics::Brine::PhaseStateBase::getSalinity() const
{
   return m_salinity;
}

inline void GeoPhysics::Brine::PhaseStateBase::setSalinity( const double salinity )
{
   m_salinity = enforceSalinityRanges( salinity );
}


inline double GeoPhysics::Brine::PhaseStateBase::enforceTemperatureRanges( const double temperature )
{
   return (temperature > s_MaxTemperature) ? s_MaxTemperature : ( temperature < s_MinTemperature ? s_MinTemperature : temperature );
}


inline double GeoPhysics::Brine::PhaseStateBase::enforcePressureRanges( const double pressure )
{
   return (pressure > s_MaxPressure) ? s_MaxPressure : ( pressure < s_MinPressure ? s_MinPressure : pressure );
}


inline double GeoPhysics::Brine::PhaseStateBase::enforceSalinityRanges( const double salinity )
{
   return (salinity > s_MaxSalinity) ? s_MaxSalinity : ( salinity < s_MinSalinity ? s_MinSalinity : salinity );
}


inline double GeoPhysics::Brine::PhaseStateBase::findT1( const double higherTemperature )
{
   return higherTemperature - 2.0 * s_halfWidth;
}


/// PhaseStateScalar inline methods

inline double GeoPhysics::Brine::PhaseStateScalar::getPressure() const
{
   return m_inRangePres;
}


inline double GeoPhysics::Brine::PhaseStateScalar::getTemperature() const
{
   return m_inRangeTemp;
}


inline double GeoPhysics::Brine::PhaseStateScalar::getLowEndTransitionTemp() const
{
   return m_lowEndTransitionTemp;
}


inline double GeoPhysics::Brine::PhaseStateScalar::getHighEndTransitionTemp() const
{
   return m_highEndTransitionTemp;
}

/// PhaseStateVec inline methods


inline int GeoPhysics::Brine::PhaseStateVec::getVectorSize() const
{
   return m_size;
}


inline int GeoPhysics::Brine::PhaseStateVec::getAqueousPhasesNum() const
{
   return m_aqueousNum;
}


inline int GeoPhysics::Brine::PhaseStateVec::getVapourPhasesNum() const
{
   return m_vapourNum;
}


inline int GeoPhysics::Brine::PhaseStateVec::getTransitionPhasesNum() const
{
   return m_transitionNum;
}


inline ArrayDefs::Int_ptr GeoPhysics::Brine::PhaseStateVec::getAqueousIndices() const
{
   return m_aqueousIdx;
}


inline ArrayDefs::Int_ptr GeoPhysics::Brine::PhaseStateVec::getVapourIndices() const
{
   return m_vapourIdx;
}


inline ArrayDefs::Int_ptr GeoPhysics::Brine::PhaseStateVec::getTransitionIndices() const
{
   return m_transitionIdx;
}


inline ArrayDefs::Real_ptr GeoPhysics::Brine::PhaseStateVec::getPressure() const
{
   return m_inRangePres;
}


inline ArrayDefs::Real_ptr GeoPhysics::Brine::PhaseStateVec::getTemperature() const
{
   return m_inRangeTemp;
}


inline ArrayDefs::Real_ptr GeoPhysics::Brine::PhaseStateVec::getLowEndTransitionTemp() const
{
   return m_lowEndTransitionTemp;
}


inline ArrayDefs::Real_ptr GeoPhysics::Brine::PhaseStateVec::getHighEndTransitionTemp() const
{
   return m_highEndTransitionTemp;
}

#endif /// GEOPHYSICS_BRINE_PHASES_H_
