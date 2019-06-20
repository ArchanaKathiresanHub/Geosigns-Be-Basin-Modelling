#ifndef _DATA_ACCESS__POINT_ADSORPTION_HISTORY_H_
#define _DATA_ACCESS__POINT_ADSORPTION_HISTORY_H_

#include <string>

#include "DAObject.h"
#include "Interface.h"

namespace DataAccess {

   namespace Interface {

      /// \brief A point at which a history for all the properties (e.g. pressure, c1-retained, ...) should be kept.
      class PointAdsorptionHistory : public DAObject {

      public :

         /// \brief Constructor.
         PointAdsorptionHistory ( ProjectHandle*    projectHandle,
                                  database::Record* record );

         /// \brief Destructor.
         virtual ~PointAdsorptionHistory ();

         /// \brief Get the x-position of the point.
         virtual double getX () const;

         /// \brief Get the y-position of the point.
         virtual double getY () const;

         /// \brief Get the name of the history file.
         virtual const std::string& getFileName () const;

         /// \brief Get the formation name.
         virtual const std::string& getFormationName () const;

         /// \brief Get mangled (i.e. no 'strange' characters, spaces, ...) name of formation.
         virtual const std::string& getMangledFormationName () const;

         /// \brief Return a string representation of the object.
         virtual std::string image () const;

      private :

         /// \brief The x-position.
         double m_x;

         /// \brief The y-position.
         double m_y;

         /// \brief The formation name.
         std::string m_formationName;

         /// \brief The name of the history file.
         std::string m_fileName;

         /// \brief The mangled formation name.
         std::string m_mangledFormationName;

      };

   }

}

#endif // _DATA_ACCESS__POINT_ADSORPTION_HISTORY_H_
