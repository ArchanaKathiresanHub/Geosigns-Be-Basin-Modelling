//
// Copyright (C) 2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _MIGRATION_OVERBURDEN_H_
#define _MIGRATION_OVERBURDEN_H_


#include "MigrationFormation.h"

namespace migration
{
   namespace overburden
   {

      /// The following functions return the overburden formations of a particular formation.
      /// And because these formations are fixed, they don't rely on a Snapshot value or a 
      /// position.  They also don't involve MPI in any way.

      class OverburdenFormations
      {
      public:

         typedef vector<const MigrationFormation*> formations_type;

      private:

         vector<const MigrationFormation*> m_formations;
         bool m_upward;

      public:

         OverburdenFormations (const formations_type formations, bool upward) :
            m_formations (formations), m_upward (upward)
         {
         }

         ~OverburdenFormations ()
         {
         }

         void append (const formations_type& formations)
         {
            m_formations.insert (m_formations.end (), formations.begin (), formations.end ());
         }

         const formations_type& formations () const { return m_formations; }
         bool upward () const { return m_upward; }

         formations_type& getFormations () { return m_formations; }
         bool& getUpward () { return m_upward; }
      };

      OverburdenFormations getOverburdenFormations (const MigrationFormation* formation,
         bool upward = false);

      /// The following functions include a selection functor:

      template <typename PRED>
      vector<const MigrationFormation*> getOverburdenFormationsIf (const MigrationFormation* formation,
         PRED pred, bool upward = false);

   }
} // namespace migration::overburden

#endif
