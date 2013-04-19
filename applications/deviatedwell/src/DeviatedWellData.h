#ifndef _DEVIATED_WELL__DEVIATED_WELL_DATA__H_
#define _DEVIATED_WELL__DEVIATED_WELL_DATA__H_

#include <string>
#include <vector>

#include "GlobalDefs.h"
#include "WellTrajectoryLocation.h"
#include "PieceWiseInterpolator1D.h"

typedef std::vector<WellTrajectoryLocation> WellTrajectoryLocationArray;

/// Class containing internal representation of input data file contents.
class DeviatedWellData {

public :

   /// Indicates a start of a comment.
   ///
   /// Must be at the start of a line.
   static const char CommentCharacter = '#';

   /// Name of well-name field.
   static const std::string WellName;

   /// Name of unit field.
   static const std::string Unit;

   /// Name of elevation field.
   static const std::string Elevation;

   /// Name of Kelly Bushing depth field.
   static const std::string KellyBushingDepth;

   /// Alternative name of Kelly Bushing depth field.
   static const std::string KellyBushing;

   /// Alternative name of Kelly Bushing depth field.
   static const std::string KB;

   /// Name of water-depth field.
   static const std::string WaterDepth;

   /// Name of delta field.
   static const std::string Delta;

   /// Alternative name of delta field.
   static const std::string DeltaS;

   /// Name of null-value field.
   static const std::string NullValue;

   /// Alternative name of delta field.
   static const std::string Resolution;

   /// Name of interpolation field.
   static const std::string Interpolation;

   /// Alternative name of interpolation field.
   static const std::string Interp;

   static const double DefaultDelta;


   /// Constructor, requires valid file-name.
   DeviatedWellData ( const std::string& fileName );

   /// Get the name of the well.
   const std::string& getName () const;

   /// Get the unit in which input and output is to be done.
   DistanceUnit getUnit () const;

   /// \brief Get the elevation.
   ///
   /// May differ from the water-depth and Kelly bushing depth.
   double getElevation () const;

   /// Get the water-depth.
   ///
   /// Value may be the null-value.
   double getWaterDepth () const;

   /// Get the Kelly Bushing depth.
   ///
   /// Value may be the null-value.
   double getKellyBushingDepth () const;

   /// Get the output step size.
   double getDeltaS () const;

   /// Get the null-value.
   double getNullValue () const;

   /// \brief Get the interpolation-kind.
   Numerics::PieceWiseInterpolator1D::InterpolationKind getInterpolationKind () const;

   /// Get constant reference to well-trajectory points.
   const WellTrajectoryLocationArray& getTrajectory () const;

   /// Return the string representation of the object.
   std::string image () const;

private : 

   /// Set a field with a value.
   ///
   /// The field-name should be in lower-case and should
   /// be one of the class field names.
   /// It should be possible to convert the field-value 
   /// string to the type required for the field.
   void setField ( const std::string& fieldName,
                   const std::string& fieldValue );

   /// Add a point to the well-trajectory.
   void addPoint ( const std::string& str );

   /// \brief After input is complete then all fields must be normalised.
   ///
   /// This means, for example, if the null-value has chaged, any field that
   /// was not chaged and was initialised with default null value must be changed
   /// to the new null-value. Also, the default deltaS is in metres, if the unit
   /// is now in feet this must be changed.
   void normaliseFields ();


   /// The name of the well.
   std::string m_name;

   /// The unit in which input and output is to be done.
   DistanceUnit m_unit;

   /// The elevation at the well location.
   double m_elevation;

   /// The depth of the water at the well location.
   double m_waterDepth;

   /// The depth of the Kelly Bushing.
   double m_kellyBushingDepth;

   /// The step size for output.
   double m_deltaS;

   /// The null-value.
   double m_nullValue;

   Numerics::PieceWiseInterpolator1D::InterpolationKind m_interpolationKind;

   /// Array containing all points in the well-trajectory.
   WellTrajectoryLocationArray m_trajectory;

};

// Inline functions.

inline const std::string& DeviatedWellData::getName () const {
   return m_name;
}

inline DistanceUnit DeviatedWellData::getUnit () const {
   return m_unit;
}

inline double DeviatedWellData::getElevation () const {
   return m_elevation;
}

inline double DeviatedWellData::getWaterDepth () const {
   return m_waterDepth;
}

inline double DeviatedWellData::getKellyBushingDepth () const {
   return m_kellyBushingDepth;
}

inline double DeviatedWellData::getDeltaS () const {
   return m_deltaS;
}

inline double DeviatedWellData::getNullValue () const {
   return m_nullValue;
}

inline Numerics::PieceWiseInterpolator1D::InterpolationKind DeviatedWellData::getInterpolationKind () const {
   return m_interpolationKind;
}

inline const WellTrajectoryLocationArray& DeviatedWellData::getTrajectory () const {
   return m_trajectory;
}


#endif // _DEVIATED_WELL__DEVIATED_WELL_DATA__H_
