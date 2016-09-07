//
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

/// @file DoEGenerator.h
/// @brief This file keeps API declaration for creating Design of Experiments

#ifndef CASA_API_DOE_GENERATOR_H
#define CASA_API_DOE_GENERATOR_H

/// @page CASA_DoEGeneratorPage Design of Experiments generator
///
/// @section CASA_DoEGeneratorSection Variety of DoE algorithms available in API
///
/// @link casa::DoEGenerator Design of Experiments (DoE) @endlink - an algorithm which creates the set of cases with parameters variations in optimal
/// way for response surface reconstruction. It allows to reduce the costs of experimentation by allowing statistical
/// model to be estimated with fewer experimental runs.
///
/// The following set of DoE generation algorithms are implemented in API:
///
/// @subsection TornadoSection Tornado design
/// The Tornado design is used for parameter screening by systematically varying the parameters one
/// at a time. This means that each variable resembles the base case value, except for one parameter that is set
/// to either its lower or its upper bound.
/// Figure
/// @image html TornadoDesign.png "Graphical representation of a Tornado design for three parameters"
/// gives a graphical representation of a Tornado design for three parameters.The number of simulation runs is, for
/// this particular case, seven: one for each face center and one for the center point.Each blue dot represents one
/// simulation run and its coordinates are the values of the parameter combination. Generally speaking, the number
/// of cases is twice the number of parameters plus one.
///
/// @subsection PlackettBurmanSection Plackett-Burman design
/// The Plackett-Burman design is mainly used for parameter screening, because the main effects are
/// heavily confounded with two-factor interactions. The number of cases of a Placket-Burman design equals the number
/// of parameters and therefore, this design is half as expensive, compared with the Tornado design. A characteristic
/// of a Placket-Burman design is that it consists of cases on the extreme corner points of the parameter space.
/// If the simulator has problems in running at these points, it is better to submit a Tornado design.
///
/// Figure
/// @image html PlackettBurman.png "Placket-Burman design for three parameters"
/// and
/// @image html PlackettBurmanMirror.png "Placket-Burman plus mirror design for three parameters"
/// give a graphical representation of a Plackett-Burman design for three parameters.
///
/// @subsection BoxBehnkenSection Box-Behnken design
/// The cases in the Box Behnken design correspond to points at a hyper sphere, such that extreme
/// parameter combinations are excluded. The design is suitable for uncertainty analysis where extreme solutions
/// (near the vertices of the hyper cube) are unlikely. The dimension of the parameter space must be 2 or larger.
/// The number of cases in the design is quadratic proportional to the number of parameters.
///
/// Figure
/// @image html BoxBehnkenDesign.png "Box-Behnken design for three parameters"
/// gives a graphical representation of a Box-Behnken design for three parameters. In this particular case,
/// the number of simulation runs is 13. Each blue dot represents one simulation run and their coordinates
/// represent the values of the corresponding parameter combination. (Note that in higher dimensions the cases
/// are not always on the mid points of the edges.This varies per dimension.)
///
/// @subsection FullFactorialSection Full Factorial design
/// The cases in the Full Factorial design correspond to the vertices of the hyper cube that is the
/// extremes of the bounded parameter space. The design is suitable for uncertainty analysis or for optimization.
/// Note that Full Factorial design is one of the most expensive Experimental Designs, since the number of cases
/// goes exponential with the number of parameters in the model.
///
/// Figure
/// @image html FullFactorialDesign.png "Full Factorial design for three parameters"
/// gives a graphical representation of a Full Factorial design for three parameters.
///
/// @subsection SpaceFillingSection Space filling design
/// Space filling design is usually used to generate cases to test the validity of the generated
/// proxy. This Space Filling design has three important advantages:
///
/// -# User has the possibility to augment the a specified number of new cases, which can be very useful.
///    The cases are generated design with a specified number of automatically in a quasi random way, such that they do not cluster
///    around earlier generated cases in the design.That way, a global search in the parameter space can be performed.
/// -# The design provides the means to gradually improve the accuracy of proxies over the whole parameter space as
///    the proxies will adapt to the new space filling cases.
/// -# The quasi-random cases serve as adequate blind test during Proxy QC after which the proxies can be forced to 
///    adapt to these new cases.
///
/// Figure
/// @image html SpaceFillingDesign.png "Space filling design for three parameters"
/// gives a graphical representation of a Space filling design for three parameters.
///
/// @subsection LatinHypercubeSection  Basic/Optimized Latin Hypercube.
/// Variant of Space filling design. LH design is non collapsing, i.e. none of the design points collapse when they are
/// orthogonally projected onto a lower dimensional space. This avoids that any two design points can be considered as the same
/// point if one or more parameters turn out to be insensitive.The design is also space-filling, but not augmentable.


// CASA
#include "CasaSerializer.h"
#include "ErrorHandler.h"

// STL
#include <vector>

namespace casa
{
   class RunCaseSet;
   class VarSpace;

   /// @class DoEGenerator DoEGenerator.h "DoEGenerator.h"
   /// @brief Class DoEGenerator allows to generate various types of "Design of Experiments"
   class DoEGenerator : public ErrorHandler, public CasaSerializable
   {
   public:

      /// @brief List of implemented DoE algorithms
      enum DoEAlgorithm
      {
         TheFirstDoEAlgo,              ///< The first DoE Algorithm (used in loops and for validation)
         BoxBehnken = TheFirstDoEAlgo, ///< Box-Behnken design
         Tornado,                      ///< Tornado design. The default algorithm
         PlackettBurman,               ///< Placket-Buurman design
         PlackettBurmanMirror,         ///< Placket-Buurman with mirror design
         FullFactorial,                ///< Full factorial design
         LatinHypercube,               ///< Optimized latin hypercube design
         SpaceFilling,                 ///< Space filling design
         TheLastDoEAlgo = SpaceFilling ///< The last DoE algorithm (used in loops and for validation)
      };

      /// @brief Destructor
      virtual ~DoEGenerator() {;}

      /// @brief Generate set of cases for DoE
      /// @return ErrorHandler::NoError on success, error code otherwise
      virtual ErrorHandler::ReturnCode generateDoE( const VarSpace & varPrmsSet   ///< [in] list of variable parameters
                                                  , RunCaseSet     & rcSet        ///< [out] container to keep the set of cases for DoE
                                                  , size_t           runsNum = 0  ///< [in] number of runs for DoE algorithm (if it support it)
                                                  , std::string      doeName = "" ///< [in] optional user given name for further reference
                                                  ) = 0;

      /// @brief Convert algorithm enumeration to the string with DoE name
      /// @param algo DoE type
      /// @return DoE type name as string
      static std::string DoEName( DoEAlgorithm algo );

      /// @brief Get DoE type
      /// @return DoE algorithm type
      virtual DoEAlgorithm algorithm() = 0;

   protected:
      DoEGenerator( ) {;}
   };
}
#endif // CASA_API_DOE_GENERATOR_H
