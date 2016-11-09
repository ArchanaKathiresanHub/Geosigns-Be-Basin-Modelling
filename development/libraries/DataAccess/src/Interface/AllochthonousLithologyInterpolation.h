#ifndef _INTERFACE_ALLOCHTHONOUS_LITHOLOGY_INTERPOLATION_H_
#define _INTERFACE_ALLOCHTHONOUS_LITHOLOGY_INTERPOLATION_H_

#include <iostream>
using namespace std;

#include <string>
using namespace std;

#include "Interface/Interface.h"
#include "Interface/Formation.h"
#include "Interface/Snapshot.h"
#include "Interface/DAObject.h"

namespace DataAccess
{
   namespace Interface
   {

     class Formation;
     class Snapshot;

      /// Provides access to the Allochthonous modelling table and results.
      ///
      /// Each object corresponds to a row in the AllochthonLithoInterpIoTbl
      /// providing the:
      ///    - Formation name;
      ///    - File name in which the results are stored;
      ///    - The Number of data points the interpolant uses;
      ///    - The polynomial degree used in the interpolant;
      ///    - The name of the HDF5 'group' the results are in;
      ///    - The names of the data sets containing the results:
      ///       -# ScalingDataSetName;
      ///       -# TranslationDataSetName;
      ///       -# PointsDataSetName;
      ///       -# CoefficientsDataSetName;
      ///       -# RHSDataSetName;
      ///
     class AllochthonousLithologyInterpolation : public DAObject
     {
	 public:


            /// \name Interpolation results data set names
            /// @{

            /// \var ScalingDataSetName
            /// Dataset name containing the scaling vector.
            static const std::string ScalingDataSetName;

            /// \var TranslationDataSetName
            /// Dataset name containing the translation vector.
            static const std::string TranslationDataSetName;

            /// \var PointsDataSetName
            /// Dataset name containing the interpolation point-set.
            static const std::string PointsDataSetName;

            /// \var CoefficientsDataSetName
            /// Dataset name containing the interpolation coefficients.
            static const std::string CoefficientsDataSetName;

            /// \var RHSDataSetName
            /// Dataset name containing the interpolation rhs vector.
            static const std::string RHSDataSetName;

            /// @}


            AllochthonousLithologyInterpolation (ProjectHandle * projectHandle, database::Record * record);
            ~AllochthonousLithologyInterpolation  (void);

	    /// Return the name of the formation of this AllochthonousLithologyInterpolation.
	    const string& getFormationName (void) const;

	    /// Return the the Snapshot of this AllochthonousLithologyInterpolation
	    const Snapshot * getStartSnapshot (void) const;

	    /// Return the the Snapshot of this AllochthonousLithologyInterpolation
	    const Snapshot * getEndSnapshot (void) const;

	    /// Return the Formation of this AllochthonousLithologyInterpolation
	    const Formation * getFormation (void) const;

	    /// Return the number of points used in the interpolation.
	    int getNumberOfPoints () const;

	    /// Return the degree of the polynomial used.
	    int getPolynomialDegree () const;

	    /// Return the file in which the results can be found.
	    const std::string& getInterpFileName () const;

	    /// Return the name of the group in which the points, rhs and coefficients can be found.
	    const std::string& getInterpGroupName () const;

	    /// Print the attributes of this AllochthonousLithologyInterpolation
            // May not work if user application is compiled under IRIX with CC -lang:std
	    void printOn (ostream & ostr) const;

	 protected:

            const Formation* m_formation;
            const Snapshot*  m_startSnapshot;
            const Snapshot*  m_endSnapshot;

      };
   }
}

#endif // _INTERFACE_ALLOCHTHONOUS_LITHOLOGY_INTERPOLATION_H_
