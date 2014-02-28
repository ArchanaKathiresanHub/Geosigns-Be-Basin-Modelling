// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_CASE_H
#define SUMLIB_CASE_H

#include <vector>

#include "SUMlib.h"

#include "ISerializer.h"

using std::vector;

namespace SUMlib {

/// @class Case defines a case, i.e. a combination of parameter values
/// of different types in the order: CONtinuous, DIScrete, CATegorical.
///
/// Generated copy constructor and copy assignment are OK.
class INTERFACE_SUMLIB Case : public ISerializable
{
public:

   /// Default constructor
   Case();

   /// Constructor for continuous parameters only
   /// @param [in] vCon         vector with continuous parameter values
   explicit Case( vector<double> const& vCon );

   /// Constructor
   /// @param [in] vCon         vector with continuous parameter values
   /// @param [in] vDis         vector with discrete parameter values
   /// @param [in] vCat         vector with categorical parameter values
   Case( vector<double> const& vCon,
         vector<int> const& vDis, vector<unsigned int> const& vCat );

   /// Constructor
   /// @param [in] nCon         number of continuous parameters
   /// @param [in] nDis         number of discrete parameters
   /// @param [in] nCat         number of categorical parameters
   /// @param [in] conVal       continuous parameter value
   /// @param [in] disVal       discrete parameter value
   /// @param [in] catVal       categorical parameter value
   explicit Case( unsigned int nCon, unsigned int nDis = 0, unsigned int nCat = 0,
         double conVal = 0.0, int disVal = 0, unsigned int catVal = 0 );

   /// Destructor
   virtual ~Case();

   /// Getter for the number of continuous parameters
   /// @returns the number of continuous parameters
   unsigned int sizeCon() const { return m_continuous.size(); }

   /// Getter for the number of discrete parameters
   /// @returns the number of discrete parameters
   unsigned int sizeDis() const { return m_discrete.size(); }

   /// Get number of ordinal parameters (i.e. combined continuous and discrete parameters)
   /// @returns the number of ordinal parameters
   unsigned int sizeOrd() const { return ( m_continuous.size() + m_discrete.size() ); }

   /// Getter for the number of categorical parameters
   /// @returns the number of categorical parameters
   unsigned int sizeCat() const { return m_categorical.size(); }

   /// Getter for the total number of parameters
   /// @returns the total number of parameters
   unsigned int size() const { return ( sizeOrd() + m_categorical.size() ); }

   /// Getter for the vector of continuous parameters
   /// @returns the vector of continuous parameter values
   vector<double> const& continuousPart() const { return m_continuous; }

   /// Getter for a specific continuous parameter
   /// @param [in]   i        continuous parameter index
   /// @returns value of continuous parameter nr. i
   double continuousPar( unsigned int i ) const { return m_continuous[i]; }

   /// Getter for the vector of discrete parameters
   /// @returns the vector of discrete parameter values
   vector<int> const& discretePart() const { return m_discrete; }

   /// Getter for a specific discrete parameter
   /// @param [in]   i        discrete parameter index
   /// @returns value of discrete parameter nr. i
   int discretePar( unsigned int i ) const { return m_discrete[i]; }

   /// Getter for the vector of categorical parameters
   /// @returns the vector of categorical parameter values
   vector<unsigned int> const& categoricalPart() const { return m_categorical; }

   /// Getter for a specific categorical parameter
   /// @param [in]   i        categorical parameter index
   /// @returns value of categorical parameter nr. i
   unsigned int categoricalPar( unsigned int i ) const { return m_categorical[i]; }

   /// Get vector of ordinal parameters by concatenating vectors of continuous and discrete parameters
   /// @returns the vector of ordinal parameter values
   vector<double> ordinalPart() const;

   /// Getter for a specific ordinal parameter; enumeration order: 1.continuous, 2.discrete
   /// @param [in]   i        ordinal parameter index
   /// @returns value of ordinal parameter nr. i
   double ordinalPar( unsigned int i ) const;

   /// Getter for a specific parameter; enumeration order: 1.continuous, 2.discrete, 3.categorical
   /// @param [in]   i        parameter index
   /// @returns parameter i value, converted to a double
   double par( unsigned int i ) const;

   /// Setter for the vector of continuous parameters
   /// @param [in]   vCon     vector of continuous parameter values
   void setContinuousPart( vector<double> const& vCon ) { m_continuous = vCon; }

   /// Setter for a specific continuous parameter
   /// @param [in]   i        continuous parameter index
   /// @param [in]   conVal   continuous parameter value
   void setContinuousPar( unsigned int i, double conVal ) { m_continuous[i] = conVal; }

   /// Setter for the vector of discrete parameters
   /// @param [in]   vDis     vector of discrete parameter values
   void setDiscretePart( vector<int> const& vDis ) { m_discrete = vDis; }

   /// Setter for a specific discrete parameter
   /// @param [in]   i        discrete parameter index
   /// @param [in]   disVal   discrete parameter value
   void setDiscretePar( unsigned int i, int disVal ) { m_discrete[i] = disVal; }

   /// Setter for the vector of categorical parameters
   /// @param [in]   vCat     vector of categorical parameter values
   void setCategoricalPart( vector<unsigned int> const& vCat ) { m_categorical = vCat; }

   /// Setter for a specific categorical parameter
   /// @param [in]   i        categorical parameter index
   /// @param [in]   catVal   categorical parameter value
   void setCategoricalPar( unsigned int i, unsigned int catVal ) { m_categorical[i] = catVal; }

   /// Check whether a specific parameter is continuous
   /// @param [in]   i        parameter index
   /// @returns true if par(i) is continuous, otherwise returns false
   bool isContinuous( unsigned int i ) const { return ( i < sizeCon() ); }

   /// Check whether a specific parameter is discrete
   /// @param [in]   i        parameter index
   /// @returns true if par(i) is discrete, otherwise returns false
   bool isDiscrete( unsigned int i ) const { return ( ( i >= sizeCon() ) && ( i < sizeOrd() ) ); }

   /// Check whether a specific parameter is categorical
   /// @param [in]   i        parameter index
   /// @returns true if par(i) is categorical, otherwise returns false
   bool isCategorical( unsigned int i ) const { return ( i >= sizeOrd() ); }

   /// Check whether another case is comparable, i.e. contains the same parameters
   /// @param [in]   c        another case
   /// @returns whether case c is comparable
   bool isComparableTo( Case const& c ) const;

   /// Copy from specific parameter value of another comparable case
   /// @param [in]   i        parameter index
   /// @param [in]   c        case to copy from
   void copyFrom( unsigned int i, Case const& c );

   /// Check whether a specific parameter value is equal to the corresponding one of another case
   /// @param [in]   i        parameter index
   /// @param [in]   c        another case
   /// @returns whether the value of parameter i is equal to the corresponding one of case c
   bool isEqualTo( unsigned int i, Case const& c ) const;

   /// Check whether another case is equal, i.e. contains the same parameter values
   /// @param [in]   c        another case
   /// @returns whether all parameter values are equal to the ones of case c
   bool isEqualTo( Case const& c ) const;

   /// Check whether a specific parameter value is smaller than the corresponding one of another case
   /// @param [in]   i        parameter index
   /// @param [in]   c        another case
   /// @returns whether the value of parameter i is smaller than the corresponding one of case c
   bool isSmallerThan( unsigned int i, Case const& c ) const;

   // made deliberately private so that calling load/save directly on this class is more difficult
   // the preferred way is to call save/load on the ISerializer.
private:
   // ISerializable
   virtual bool load( IDeserializer*, unsigned int version );
   virtual bool save( ISerializer*, unsigned int version ) const;

private:
   /// Continuous parameter values
   vector<double> m_continuous;

   /// Discrete parameter values
   vector<int> m_discrete;

   /// Categorical parameter values
   vector<unsigned int> m_categorical;
};

} // namespace SUMlib

#endif // SUMLIB_CASE_H
