//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file VarPrmContinuous.h
/// @brief This file keeps API declaration for handling continuous parameters. 

#ifndef CASA_API_VAR_PRM_CONTINOUS_H
#define CASA_API_VAR_PRM_CONTINOUS_H

#include "Range.h"

#include <memory>

/// @page CASA_VarPrmContinuousPage Continuous variable parameter
///
/// Continuous parameter - a parameter that can take any value between certain bounds
/// (for instance, a fault-sealing factor that varies in the [0, 1] range)
/// CASA API allows to have the following subtypes of continuous parameters:
///   - \subpage CASA_SimpleRangePage
///   - \subpage CASA_MapRangePage
///   - \subpage CASA_CurveRangePage
///
/// The following list of simple range variable parameters is implemented in CASA API
/// - @link CASA_SourceRockTOCPage Source rock initial Total Organic Contents (TOC) parameter @endlink
/// - @link CASA_TopCrustHeatProductionPage Top crust heat production rate parameter @endlink
///
namespace casa
{
   class Parameter;

   /// @brief Variable parameter with continuous value range
   class VarPrmContinuous
   {
   public:
      /// @brief Probability Density Function (PDF) shape for the parameter. It is used in casa::MCSolver
      enum PDF
      {
         Block,    ///< PDF is uniform in parameter range
         Triangle, ///< triangle shape of the PDF. Maximum position of PDF is defined by parameter value in base case
         Normal    ///< Gauss shape of the pdf
      };

      /// @brief Destructor
      virtual ~VarPrmContinuous() {;}

      /// @brief Get minimal variable parameter value as double
      /// @return minimal value for variable parameter
      double minValueAsDouble() const { return m_valueRange->minRangeValueAsDouble(); }

      /// @brief Get maximal variable parameter value as double
      /// @return maximal value for variable parameter
      double maxValueAsDouble() const { return m_valueRange->maxRangeValueAsDouble(); }

      /// @brief Get base value for the variable parameter as double
      /// @return base value
      virtual double baseValueAsDouble() const = 0;

      /// @brief Create a copy of the parameter and assign to the given value. If value is outside of the parameter range,\n
      ///        the method will return a zero pointer
      /// @param val new value for parameter
      /// @return the new parameter object which should be deleted by the caller itself
      virtual Parameter * createNewParameterFromDouble( double val ) const = 0;

   protected:
      VarPrmContinuous() : m_pdf(Block) {;}

      std::auto_ptr<Parameter> m_baseValue;    ///< Base parameter value, used also as object factory for concrete parameter value
      std::auto_ptr<Range>     m_valueRange;   ///< Range for the parameter (min,max) or (left,right) values

      PDF                       m_pdf;          ///< Probability density function for parameter. Block is default value
   
   private:
   };

}

#endif // CASA_API_VAR_PRM_CONTINOUS_H
