// Copyright 2012, Shell Global Solutions International B.V.
// All rights reserved. This document and the data and information contained herein is CONFIDENTIAL.
// Neither the whole nor any part of this document may be copied, modified or distributed in any
// form without the prior written consent of the copyright owner.

#ifndef SUMLIB_SUMLIB_H
#define SUMLIB_SUMLIB_H

// dll import/export only for windows
#if defined(WIN32) && !defined(SUMLIB_STATIC)

// Make sure the interface definition doesn't mess up anything already defined.
#ifdef INTERFACE_SUMLIB

#error "DLL interface definition requires redefinition of SUMLIB_DLL_INTERFACE"

#else // INTERFACE_SUMLIB

// Define the direction of the interface specification
#ifdef SUMLIB_EXPORT
#define INTERFACE_SUMLIB __declspec( dllexport )
#else
#define INTERFACE_SUMLIB __declspec( dllimport )
#endif // SUMLIB_EXPORT
#endif //INTERFACE_SUMLIB


#define INTERFACE_SUMLIB_DEBUG INTERFACE_SUMLIB

// Uncomment to avoid exporting names used only in the unit tests
//#ifdef NDEBUG
//#define INTERFACE_SUMLIB_DEBUG
//#endif //_DEBUG

#else // WIN32

// Empty defines for all other situations
#define INTERFACE_SUMLIB
#define INTERFACE_SUMLIB_DEBUG

#endif // WIN32

namespace SUMlib {

static const char * const __sumlib_version__ = "SUMlib (svn revision: $Rev$ $Date$)";

} // namespace SUMlib

// This documents the SUMlib API
/**
\mainpage SUMlib API Documentation

\section Sec_Intro Introduction

\warning This description is under construction

%SUMlib is a class library for performing stochastic uncertainty modeling. All
classes are in namespace SUMlib. This document describes how to use the
classes in the library for the five primary purposes of the library:

\li \ref Sec_ExpDesign Generating cases for a parameter domain using an experimental design algorithm
\li \ref Sec_ProxyCreation Creating a proxy for observables based on cases and associated observable output
\li \ref Sec_ParameterSensitivity Evaluating parameter sensitivities based on a proxy
\li \ref Sec_MCMC Running an MCMC process on a proxy
\li \ref Sec_Clustering Clustering of a generated (MCMC) data set

\section Sec_BaseTypes Base types

The library uses a series of base types implemented as typedefs of STL
containers to describe the arguments and return types of class methods. The
base types are defined in header BaseTypes.h. Most typedefs are confusing/
outdated by now and should be removed (e.g. Parameter, DataTuple, TargetSet).

\section Sec_ExpDesign Experimental Design

The library contains a class hierarchy of experimental designs, which generate
a set of cases according to a defined algorithm.

\li SUMlib::ExpDesign Abstract base class defining shared functions
\li SUMlib::ExpDesignBase Base class
\li SUMlib::BoxBehnken Design excluding extreme parameter combinations
\li SUMlib::ScreenDesign Plackett-Burman design
\li SUMlib::FactDesign Full factorial design
\li SUMlib::OptimisedLHD Latin Hypercube design
\li SUMlib::HybridMC Augmentable space-filling design
\li SUMlib::Tornado Design where parameters are varied one by one

\todo Combine ExpDesign and ExpDesignBase?

\section Sec_ProxyCreation Proxy Creation

A proxy is a mathematical model (a polynomial up to 3rd order with or without Kriging) describing the relation between the parameters and an observable.

\li SUMlib::CompoundProxyCollection External class holding a proxy for each user-defined observable and the parameter space in which the proxies "live".
\li SUMlib::Proxy The abstract base class defining the interface for all internal proxy classes.

\li SUMlib::McmcProxy Abstract base class defining shared functions for proxies wrapped with reference data.
\li SUMlib::ReferenceProxy A proxy wrapper class that combines proxies with corresponding reference (measurement) data if available.

\todo Rename ProxyBuilder to CubicProxyBuilder?

\section Sec_ParameterSensitivity Parameter Sensitivity Analysis

Pareto sensitivities can be based on multiple observables whereas Tornado sensitivities are always based on a single observable.

\li SUMlib::Pareto
\li SUMlib::TornadoSensitivities

\section Sec_MCMC Mcmc Process Execution

All SUMlib classes that derive from McmcBase make use of a (wrapped) proxy response to do the sampling in a scaled parameter space.
Contrary to MC, MCMC and MCSolver are algorithms constrained by reference data.

\li SUMlib::McmcBase Base class for a sampling algorithm.
\li SUMlib::MC Monte Carlo algorithm that creates a sample of cases based on prior PDF's.
\li SUMlib::MCMC Markov Chain Monte Carlo algorithm that creates a sample of cases approximating the posterior PDF.
\li SUMlib::MCSolver Monte Carlo based solver searching for the cases that best honor the supplied reference data.

\li SUMlib::McmcStatistics Calculates and stores statistics of the mcmc process.

\section Sec_Clustering

The SUMlib clustering assumes data sets are carefully prepared (scaled and converted to doubles) before they can be analysed.
Note that also the cluster output is still scaled and converted to doubles.

\li SUMlib::Cluster Abstract base class defining shared functions for an individual cluster.
\li SUMlib::ChainClusterAlg Algorithm that identifies different clusters in a data set (of MCMC cases).

\section Sec_Utility Utility classes

\li SUMlib::RandomGenerator

\li SUMlib::Case A case holding continuous, discrete as well as categorical parameter values.
\li SUMlib::ParameterBounds Holds parameter bounds and the used values (indices) of the categorical parameters (if present).
\li SUMlib::ParameterSpace Parameter domain with functions to prepare (scale and strip fixed parameters) and unprepare cases.
\li SUMlib::ParameterPdf Holds parameter bounds, most likely values, (co)variances, weights, and categorical parameter values.

\li SUMlib::ProbDistr Abstract base class defining shared functions for calculating prior probabilities.
\li SUMlib::MVNormalProbDistr Describes a multivariate normal distribution.
\li SUMlib::MarginalProbDistr Describes marginal distributions (Uniform, Triangular, Normal, Log-normal, weighted).

**/

#endif // SUMLIB_SUMLIB_H
