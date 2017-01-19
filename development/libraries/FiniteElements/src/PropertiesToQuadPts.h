//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _PROPERTIES_TO_QUAD_PTS_H_
#define _PROPERTIES_TO_QUAD_PTS_H_

#include <algorithm>
#include <vector>
#include <assert.h>

#include "AlignedDenseMatrix.h"
#include "BasisFunctionInterpolator.h"
#include "CpuInfo.h"

namespace FiniteElementMethod
{
   /// Forward declaration
   class ElementVector;

   /// \enum Property type
   enum CauldronPropertyName{ Ph = 0,        // Hydrostatic pressure
                              Pl,            // Lithostatic pressure
                              Po,            // Overpressure
                              Pp,            // Pore pressure
                              Ves,           // Vertical effective stress
                              MaxVes,        // Max vertical effective stress
                              ChemComp,      // Chemical compaction
                              Temp,          // Temparature
                              No_Property }; // Total number of properties in this enum

   /// \enum Time step
   enum TimeStep{ Curr, Prev };

   /// \brief Class to compute provided properties on all quadrature points
   class PropertiesToQuadPts
   {
   public :

      /// \brief Destructor
      ~PropertiesToQuadPts();

      /// \brief Constructor
      /// \param [in] basisMat Matrix storing all basis functions.
      explicit PropertiesToQuadPts( const Numerics::AlignedDenseMatrix & basisMat );

      /// \brief Compute the properties in each quadrature point, basically it's a matrix-matrix product
      /// \param [in] propName Enum for the property type
      /// \param [in] propVal ElementVector containing property values on all DOFs
      template<typename T, typename V, typename... Args>
      void compute( const T propName, const V & propVal, const Args & ... args );

      /// \brief Gets the number of properties to be computed at each quadrature point
      unsigned int getNumberOfProperties() const;

      /// \brief Gets the stored basis function matrix
      const Numerics::AlignedDenseMatrix & getBasisFunctionsMatrix() const;

      /// \brief Gets the stored property matrix
      const Numerics::AlignedDenseMatrix & getPropertyDofMatrix() const;

      /// \brief Gets the stored property matrix on quadrature points
      const Numerics::AlignedDenseMatrix & getPropertyQuadMatrix() const;

      /// \brief Get property names stored inside the object
      const std::vector< CauldronPropertyName > & getPropertyNames() const;

      /// \brief Retrieve (by index) property values in each quadrature point
      /// \param [in] idx Index of the property to be retrieved (consistent with the order used in compute).
      const double * getProperty( const unsigned int idx ) const;

      /// \brief Retrieve (by name) property values in each quadrature point
      /// \param [in] idx Enum name of the property to be retrieved
      const double * getProperty( CauldronPropertyName propName ) const;

   private :

      /// \brief Number of properties to be computed at each quadrature point
      unsigned int m_numProps;

      /// \brief Vector containing property types
      std::vector< CauldronPropertyName > m_propNames;

      /// \brief Aligned matrix containing the basis function values (#DOFs x #quad)
      Numerics::AlignedDenseMatrix m_basisMat;

      /// \brief Aligned matrix containing the transpose of the basis function values (#quad x #DOFs)
      Numerics::AlignedDenseMatrix m_basisMatTranspose;

      /// \brief Aligned matrix containing the property values (#DOFs x #prop)
      Numerics::AlignedDenseMatrix m_propOnDofsMat;

      /// \brief Aligned matrix containing the properties computed on each quadrature point (#prop x #quad)
      Numerics::AlignedDenseMatrix m_propOnQuadMat;

      /// \brif An instance to the CpuInfo class to get the information about the CPU
      cpuInfo m_cpuInfo;

      /// \brief
      void addSingleProperty( const unsigned int propIdx,
                              CauldronPropertyName propName,
                              const ElementVector & propVal );

      /// \brief
      template <typename T, typename V>
      void addProperties( const unsigned int propIdx,
                          const T propName,
                          const V & propVal );

      /// \brief
      template<typename T, typename V, typename... Args>
      void addProperties( const unsigned int propIdx,
                          const T propName,
                          const V & propVal,
                          const Args & ... args );

   };


   inline unsigned int PropertiesToQuadPts::getNumberOfProperties() const { return m_numProps; }


   inline const Numerics::AlignedDenseMatrix & PropertiesToQuadPts::getBasisFunctionsMatrix() const { return m_basisMat; }


   inline const Numerics::AlignedDenseMatrix & PropertiesToQuadPts::getPropertyDofMatrix() const { return m_propOnDofsMat; }


   inline const Numerics::AlignedDenseMatrix & PropertiesToQuadPts::getPropertyQuadMatrix() const { return m_propOnQuadMat; }


   inline const std::vector< CauldronPropertyName > & PropertiesToQuadPts::getPropertyNames() const { return m_propNames; }


   template <typename T, typename V>
   void PropertiesToQuadPts::addProperties( const unsigned int propIdx,
                                            const T propName,
                                            const V & propVal )
   {
      // Check that all types are correct
      static_assert( std::is_same<T,CauldronPropertyName>::value and std::is_same<V,ElementVector>::value,
                     "Only allowed for Fundamental_Property and ElementVector");
      addSingleProperty( propIdx, propName, propVal );
   }


   template <typename T, typename V, typename... Args>
   void PropertiesToQuadPts::addProperties( const unsigned int propIdx,
                                            const T propName,
                                            const V & propVal,
                                            const Args & ... args )
   {
      // Check that all types are correct
      static_assert( std::is_same<T,CauldronPropertyName>::value and std::is_same<V,ElementVector>::value,
                     "Only allowed for Fundamental_Property and ElementVector");
      addSingleProperty( propIdx, propName, propVal );
      addProperties( propIdx + 1, args... ); // recursive variadic template
   }

   inline const double * PropertiesToQuadPts::getProperty( const unsigned int idx ) const
   {
      assert( idx < m_numProps );
      return m_propOnQuadMat.getColumn( idx );
   }

   inline const double * PropertiesToQuadPts::getProperty( CauldronPropertyName propName ) const
   {
      const unsigned int idx = static_cast<unsigned int>( std::find( m_propNames.begin(),
                                                                     m_propNames.end(), propName )
                                                          - m_propNames.begin() );
      return getProperty( idx );
   }



   template <typename T, typename V, typename... Args>
   void PropertiesToQuadPts::compute( const T propName,
                                      const V & propVal,
                                      const Args & ... args )
   {
      // Number of provided pairs of property name and value
#if defined(_MSC_VER) && _MSC_VER < 1900
      const int numInputs = sizeof...(Args) + 2;
#else
      constexpr int numInputs = sizeof...(Args) + 2;
#endif
      static_assert( numInputs % 2 == 0, "Wrong number of input properties provided." );

      // Storing the number of provided properties
      m_numProps = numInputs / 2;
      // Resizing vector of property names and matrix
      m_propNames.resize( m_numProps );
      m_propOnDofsMat.resize( m_basisMat.rows(), m_numProps );

      // Filling the matrix containing the property values
      unsigned int propIdx = 0;
      addProperties( propIdx, propName, propVal, args...);

      // Computing values in each quadrature point
      m_propOnQuadMat.resize( m_basisMat.cols(), m_numProps );

      FiniteElementMethod::BasisFunctionInterpolator interpolator;
      interpolator.compute( m_basisMatTranspose, m_propOnDofsMat, m_propOnQuadMat, m_cpuInfo );
   }

}

#endif // _PROPERTIES_TO_QUAD_PTS_H_
