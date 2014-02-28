// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_PARAMETERBOUNDS_H
#define SUMLIB_PARAMETERBOUNDS_H

#include <vector>

#include "BaseTypes.h"
#include "Case.h"
#include "SUMlib.h"

#include "ISerializer.h"

using std::vector;

namespace SUMlib {

/// @class ParameterBounds defines the parameter bounds (low and high values).
/// For a categorical parameter the following convention holds:
/// 0 <= low <= high <= number of categorical values - 1.
///
/// Generated copy constructor and copy assignment are OK.
class INTERFACE_SUMLIB ParameterBounds : public ISerializable
{
public:

   /// Default constructor
   ParameterBounds();

   /// Constructor;
   /// for a categorical parameter the following convention must hold:
   /// 0 <= low <= high <= number of categorical values - 1
   /// @param [in] low         case containing low values
   /// @param [in] high        case containing high values
   /// @param [in] catValues   available values for each categorical parameter, ordered from low to high
   ParameterBounds( Case const& low, Case const& high, vector<IndexList> const& catValues );

   /// Constructor;
   /// for a categorical parameter the following convention holds:
   /// low >= 0 is the lowest categorical value occuring in caseSet,
   /// high <= number of categorical values - 1 is the highest categorical value in caseSet.
   /// @param [in] caseSet     Case set to deduce bounds from
   explicit ParameterBounds( vector<Case> const& caseSet );

   /// Destructor
   virtual ~ParameterBounds();

   /// initialisation from supplied bounds
   /// @param [in] low         case containing low values
   /// @param [in] high        case containing high values
   /// @param [in] catValues   available values for each categorical parameter, ordered from low to high
   void initialise( Case const& low, Case const& high, vector<IndexList> const& catValues );

   /// Returns whether the bounds of a specific parameter are equal
   /// @param [in] idx  parameter index
   /// @returns whether the bounds of a specific parameter are equal
   bool areEqual( unsigned int idx ) const { return ( m_high.isEqualTo( idx, m_low ) ); }

   /// Getter for the total number of parameters
   /// @returns the total number of parameters (actually the size of m_low)
   unsigned int size() const { return m_low.size(); }

   /// Getter for the number of continuous parameters
   /// @returns the number of continuous parameters
   unsigned int sizeCon() const { return m_low.sizeCon(); }

   /// Getter for the number of discrete parameters
   /// @returns the number of discrete parameters
   unsigned int sizeDis() const { return m_low.sizeDis(); }

   /// Get number of ordinal parameters (i.e. combined continuous and discrete parameters)
   /// @returns the number of ordinal parameters
   unsigned int sizeOrd() const { return m_low.sizeOrd(); }

   /// Getter for the number of categorical parameters
   /// @returns the number of categorical parameters
   unsigned int sizeCat() const { return m_low.sizeCat(); }

   /// Check whether specific bounds are continuous
   /// @param [in]   i        parameter index
   /// @returns true if bounds of par(i) are continuous, otherwise returns false
   bool isContinuous( unsigned int i ) const { return ( i < sizeCon() ); }

   /// Check whether specific bounds are discrete
   /// @param [in]   i        parameter index
   /// @returns true if bounds of par(i) are discrete, otherwise returns false
   bool isDiscrete( unsigned int i ) const { return ( ( i >= sizeCon() ) && ( i < sizeOrd() ) ); }

   /// Check whether specific bounds are categorical
   /// @param [in]   i        parameter index
   /// @returns true if bounds of par(i) are categorical, otherwise returns false
   bool isCategorical( unsigned int i ) const { return ( i >= sizeOrd() ); }

   /// Getter for the lower case
   /// @returns the lower case
   const Case& low() const { return m_low; }

   /// Getter for the higher case
   /// @returns the higher case
   const Case& high() const { return m_high; }

   /// Getter for the range of a specific ordinal parameter
   /// @param [in]   i        ordinal parameter index
   /// @returns the range of the specified ordinal parameter
   double rangeOrd( unsigned int i ) const { return high().ordinalPar( i ) - low().ordinalPar( i ); }

   /// Getter for the available values of a specific categorical parameter
   /// @param [in]   i        categorical parameter index
   /// @returns the available values of a specific categorical parameter
   const IndexList& catValues( unsigned int i ) const { return m_catValues[i]; }

   /// Getter for the available values of each categorical parameter
   /// @returns the available values of each categorical parameter
   const vector<IndexList>& catValues() const { return m_catValues; }

   // made deliberately private so that calling load/save directly on this class is more difficult
   // the preferred way is to call save/load on the ISerializer.
private:
   // ISerializable
   virtual bool load( IDeserializer*, unsigned int version );
   virtual bool save( ISerializer*, unsigned int version ) const;


private:
   /// Check dimensions and bounds
   void checkLowHigh() const;

   /// Check whether supplied categorical parameter values are valid
   /// @param [in]   catValues   supplied values for each categorical parameter
   bool validCatValues( vector<IndexList> const& catValues ) const;

   /// Set default values for each categorical parameter (limited by the bounds)
   void setDefaultCatValues();

   ///  Lower (inclusive) bounds
   Case m_low;

   ///  Upper (inclusive) bounds
   Case m_high;

   /// For each categorical parameter, a vector is used to store the available
   /// categorical values. For example, suppose that categorical parameter i can
   /// contain 3 values (i.e. 0, 1 and 2), but value 1 happens to be unavailable,
   /// then m_catValues[i] is an ordered vector (from low to high) of size 2:
   /// m_catValues[i][0] = 0 and m_catValues[i][1] = 2.
   /// Note that a categorical value can be regarded as a simple index.
   vector<IndexList> m_catValues;
};

} // namespace SUMlib

#endif // SUMLIB_PARAMETERBOUNDS_H
