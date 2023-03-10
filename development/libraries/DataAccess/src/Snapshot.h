//
// Copyright (C) 2018-2018 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef INTERFACE_SNAPSHOT_H
#define INTERFACE_SNAPSHOT_H

#include "AbstractSnapshot.h"
#include "DAObject.h"

namespace DataAccess
{
   namespace Interface
   {
      /// A Snapshot object contains a time for which PropertyValue objects were computed.
      /// These times have been system-generated or user-generated. System-generated times are
      /// all times of geological events captured by the basin model.
      class Snapshot : public DataModel::AbstractSnapshot, public DAObject
      {
         public:
            Snapshot (ProjectHandle& projectHandle, database::Record * record);
            Snapshot (ProjectHandle& projectHandle, const double time);
            ~Snapshot () final = default;

            /// \return the time of this Snapshot
            double getTime () const final;

            /// \return the name of the file in which 3D PropertyValues associated to this snapshot are
            ///    stored
            virtual const std::string & getFileName (bool setIfNecessary = false) const;

            /// \return the snapshot-kind:
            ///   - System Generated; or
            ///   - User Defined; or
            ///   - "" (a null std::string).
            virtual const std::string& getKind () const;

            virtual void asString (std::string &) const;

            /// \return the age of this snapshot in std::string form
            virtual const std::string & asString () const;

            virtual void printOn (std::ostream &) const;

            virtual int getType () const;

            virtual bool getUseInResQ () const;

            /// \brief Set whether the snapshot file should be appended or not.
            virtual void setAppendFile ( const bool append );

            /// \brief Indicates whether the snapshot file should be appended or not.
            virtual bool getAppendFile () const;

         private:
            mutable int m_type;
            double m_time;
            bool m_appendFile; ///< Indicates whether the snapshot file should be appended or not, the default if false.
      };

      bool operator== (const Snapshot & ss1, const Snapshot & ss2);
      bool operator<  (const Snapshot & ss1, const Snapshot & ss2);
      bool operator>  (const Snapshot & ss1, const Snapshot & ss2);

   }
}


#endif // INTERFACE_SNAPSHOT_H
