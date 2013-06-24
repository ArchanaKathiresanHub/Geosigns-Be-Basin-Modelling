#include "Crust.h"
#include "formattingexception.h"
#include <cassert>
#include <limits>
#include <cmath>

struct CrustThinningException : formattingexception::BaseException< CrustThinningException > 
{ CrustThinningException() { *this << "Ignoring crustal thinning event: "; } };

Crust::Crust( double initialThickness)
   : m_initialThickness( initialThickness )
   , m_thinningEvents()
{}     

bool Crust :: overlap( double t0, double dt0, double t1, double dt1 )
{
   assert( dt0 > 0);
   assert( dt1 > 0);

   // Two cases
   
   // Case 1:
   //          (t0)|-----------| (t0+dt0)
   //   (t1) |------------| (d1+dt1)
   
   // Case 2:
   //   (t0) |------------|  (t0+dt0)
   //        (t1)  |-----------| (t1+dt1)

   if (t1 > t0)
      return t0 > t1 - dt1;
   else
      return t1 > t0 - dt0;
}

bool Crust :: addThinningEvent( double startTime, double duration, double ratio)
{
   assert (ratio > 0.0);
   assert (duration > 0.0);
   assert (startTime >= duration);

   ThinningEvents::iterator te;

   for (te = m_thinningEvents.begin (); te != m_thinningEvents.end(); ++te)
   {
      if (overlap(te->first, te->second.first, startTime, duration))
	 throw CrustThinningException() << "Crustal thinning event (" << startTime << ", (" << duration << ", " << ratio << ")) overlaps "
	    "with (" << te->first << ", (" << te->second.first << ", " << te->second.second << "))";

      if (startTime > te->first) break;
   }

   m_thinningEvents.insert (te, std::make_pair (startTime, std::make_pair (duration, ratio)));

   return true;

#if 0 // Wijnand's code
   // find the latest event that could collide
   ThinningEvents::iterator lowerBound = m_thinningEvents.lower_bound (startTime - duration);

   ThinningEvents::iterator e = lowerBound;
   while (e != m_thinningEvents.end () && e->first - e->second.first < startTime)
   {
      if (overlap (e->first, e->second.first, startTime, duration))
         return false;
   }

   if (e == m_thinningEvents.end ())
   {
      m_thinningEvents.insert (e, std::make_pair (startTime, std::make_pair (duration, ratio)));
      return true;
   }
   else
   {
      return false;
   }
#endif
}

const double Crust::MinimumEventSeparation = 1.0e-6; // Ma

Crust::ThicknessHistory Crust::getThicknessHistory() const
{
   ThicknessHistory result;
   double time = std::numeric_limits < double >::infinity ();

   if (m_thinningEvents.empty ())
   {
      result.push_back (ThicknessAtTime (0, m_initialThickness));
   }
   else
   {
      // Play back the events in order of time (recall that the unit of Time is negative: million of years ago)

      double thickness = m_initialThickness;
      typedef ThinningEvents::const_reverse_iterator EventIt;

      for (EventIt e = m_thinningEvents.rbegin (); e != m_thinningEvents.rend (); ++e)
      {
         assert (time >= e->first);

         if (std::fabs (e->first - time) < MinimumEventSeparation)
         {
            // previous event edges to the current event, so we can leave out a record for this.
            time = e->first;
         }
         else
         {                      // put in a new record at the start of the thinning event
            time = e->first;
            result.push_back (ThicknessAtTime (time, thickness));
         }

         double duration = e->second.first;
         double ratio = e->second.second;

         assert (duration > 0.0);
         assert (ratio > 0.0);

         time -= duration;
         thickness /= ratio;

         assert (time > 0.0);

         // put in a new record at the end of the thinning event
         result.push_back (ThicknessAtTime (time, thickness));
      }
   }

   return result;
}

