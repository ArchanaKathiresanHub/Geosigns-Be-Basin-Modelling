// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_BASETYPES_H
#define SUMLIB_BASETYPES_H

#include <map>
#include <vector>
#include <list>

namespace SUMlib {

/// @typedef RealVector represents a vector of real numbers
typedef std::vector<double> RealVector;

/// @typedef RealMatrix represents a matrix of real numbers
/// The implicit assumption is that all vectors in the matrix are of equal size
typedef std::vector<RealVector> RealMatrix;

/// @typedef Index represents an index (non-negative natural number)
typedef unsigned int Index;

/// @typedef IndexList is a vector of indices
typedef std::vector<Index> IndexList;

/// @typedef IndexSubset is a list of indices (no random access!)
typedef std::list<Index> IndexSubset;

/// @typedef DataTuple represents an set of values ordered according to the
/// order of the parameter factors.
typedef RealVector DataTuple;

/// @typedef Parameter represents a list of values for each of the
/// parameter entries. The ordering of the values is according to the ordering
/// of the parameters.
/// Alternative name: Case
typedef DataTuple Parameter; //to be removed

/// @typedef ParameterData represents a list of representative values other
/// than a Parameter (i.e. boundary values, mean, stddev for each of the
/// parameter entries). The ordering of the values is according to the ordering
/// of the parameters.
typedef DataTuple ParameterData; //to be removed

/// @typedef Parameterset represents a collection of Parameters, associated
/// with a series of runs each using a Parameter.
typedef std::vector<Parameter> ParameterSet; //to be removed

/// @typedef TargetSet represents a target value for each case in a
/// corresponding ParameterSet.
typedef RealVector TargetSet;

/// @typedef TargetCollection represents a collection of TargetSets.
/// If there are multiple targets in which the user is interested,
/// e.g. cumoil@field@end-of-life and pavg_datum@field@2008/01/01
/// then this yields two independent target values for each case.
/// Proxy estimation needs to be done for each target independently
typedef std::vector<TargetSet> TargetCollection;

/// @typedef CaseList   identifies which cases are active.
/// A list of boolean values, identifying which cases are active.
/// Used for filtering both TargetSet and ParameterSet
typedef std::vector<bool> CaseList;

/// @typedef Partition   partitioning of parameter space in 2 disjunct subsets.
/// A list of boolean values that correspond to a partition.
/// Used for partitioning the parameter space in 2 disjunct subsets.
typedef std::vector<bool> Partition;

/// @typedef VarList  represents a list of indexes into an array
/// of proxy variables
typedef IndexList VarList;

/// @typedef ResponseSet reprents a proxy function call return value for
/// each of the Parameters in the corresponding ParameterSet.
typedef RealVector ResponseSet;

/// @typedef ProxyData is a matrix of proxy coefficients
typedef RealMatrix ProxyData;


/// @typedef The proxy response value
typedef double                      ProxyValue;

/// @typedef A list of proxy response values, one for each target set
/// in a target collection
typedef std::vector<ProxyValue>     ProxyValueList;


/// @typedef A combination of parameter element indices
typedef IndexList                         MonomialKey;

/// @typedef A list of monomial keys
typedef std::vector< IndexList >          MonomialKeyList;

/// @typedef A map of monomial keys and their coefficients
typedef std::map< MonomialKey, double >   MonomialCoefficientsMap;

/// @typedef CategoricalCombination is a specific combination of values of all
/// categorical parameter variables
typedef std::vector<unsigned int> CategoricalCombination;

/// @typedef CategoricalParameter is a parameter with categorical parameter variables
typedef std::pair<CategoricalCombination,Parameter> CategoricalParameter;

/// @typedef CategoricalParameterSet is set of CategoricalParameters
typedef std::vector<CategoricalParameter> CategoricalParameterSet;

/// @typedef CategoricalParameter is a target for a specific categorical combination
typedef std::pair<CategoricalCombination,double> CategoricalTarget;

/// @typedef CategoricalTargetSet is set of CategoricalTargets
typedef std::vector<CategoricalTarget> CategoricalTargetSet;
} // namespace SUMlib

#endif // SUMLIB_BASETYPES_H
