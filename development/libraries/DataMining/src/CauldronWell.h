#ifndef _DATAACCESS__CAULDRON_WELL_H_
#define _DATAACCESS__CAULDRON_WELL_H_

#include <string>

#include "Point.h"

namespace DataAccess {

   namespace Mining {

      /// \brief Class defining a well.
      class CauldronWell {

      public :

         /// Units are metres.
         static const double DefaultSamplingResolution;


         /// Constructor.
         CauldronWell ( const std::string& name );

         /// Destructor.
         virtual ~CauldronWell () {}


         /// \name Location and trajectory.
         //@{

         /// \brief Get the top-most position of the well.
         ///
         /// The start point for the well extraction.
         virtual Numerics::Point getTop () const = 0;

         /// \brief Get the last point of the well.
         virtual Numerics::Point getBottom () const = 0;

         /// \brief Get the length of the well.
         virtual double getLength () const = 0;

         /// \brief Get the position (x,y,z) for the point along the well.
         virtual Numerics::Point getLocation ( const double depthAlongHole ) const = 0;

         //@}

         /// \name General information.
         //@{

         /// \brief Get the name of the well.
         const std::string& getName () const;

         /// \brief Set the null-value.
         void setNullValue ( const double nullValue );

         /// \brief Return the null-value.
         double getNullValue () const;

         /// \brief Set the elevation.
         void setElevation ( const double elevation );

         /// \brief Return the elevation.
         double getElevation () const;

         /// \brief Set the water depth.
         void setWaterDepth ( const double waterDepth );

         /// \brief Return the water depth.
         double getWaterDepth () const;

         /// \brief Set the Kelly bushing depth.
         void setKellyBushingDepth ( const double kellyBushingDepth );

         /// \brief Return the Kelly bushing depth.
         double getKellyBushingDepth () const;

         /// \brief Set the resolution at which any sampling of results should done.
         void setSamplingResolution ( const double resolution );

         /// \brief Return the resolution at which any sampling of results should done.
         double getSamplingResolution () const;

         //@}

      private :

         /// \brief The name of the well.
         const std::string m_name;

         /// The null-value.
         double m_nullValue;

         /// The elevation.
         double m_elevation;

         /// The depth of the water.
         double m_waterDepth;

         /// The depth of the Kelly bushing.
         double m_kellyBushingDepth;

         /// The sampling resolution
         double m_samplingResolution;

      };

   }

}

// Inline functions.

inline const std::string& DataAccess::Mining::CauldronWell::getName () const {
   return m_name;
}

inline double DataAccess::Mining::CauldronWell::getNullValue () const {
   return m_nullValue;
}

inline double DataAccess::Mining::CauldronWell::getElevation () const {
   return m_elevation;
}

inline double DataAccess::Mining::CauldronWell::getWaterDepth () const {
   return m_waterDepth;
}

inline double DataAccess::Mining::CauldronWell::getKellyBushingDepth () const {
   return m_kellyBushingDepth;
}

inline double DataAccess::Mining::CauldronWell::getSamplingResolution () const {
   return m_samplingResolution;
}


#endif // _DATAACCESS__CAULDRON_WELL_H_
