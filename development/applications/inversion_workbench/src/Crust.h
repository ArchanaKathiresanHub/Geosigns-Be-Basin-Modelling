#ifndef INVERSION_CRUST
#define INVERSION_CRUST

#include <utility>
#include <vector>
#include <map>

class Crust
{
public:
   typedef double Time;
   typedef double Thickness;
   typedef std::pair< Time, Thickness > ThicknessAtTime;
   typedef std::vector< ThicknessAtTime > ThicknessHistory;

   explicit Crust(double initialThickness);

   ThicknessHistory getThicknessHistory() const;

   // Adds a thinning events. It is an error when the event overlaps with an other.
   // Note: Duration and ratio must be positive (> 0) numbers.
   void addThinningEvent(double startTime, double duration, double ratio);

private:
   typedef double Duration;
   typedef double Ratio;
   typedef std::map<Time, std::pair< Duration, Ratio> > ThinningEvents ;

   static const double MinimumEventSeparation; // Ma

   // answers whether there is already an event that overlaps with the given time interval
   bool collides(double startTime, double duration) const;

   // answers the question whether two time intervals overlap. Note that t0 and t1 are in millions of years AGO.
   static bool overlap(double t0, double dt0, double t1, double dt1);

   double m_initialThickness;       // The initial thickness of the crust
   ThinningEvents m_thinningEvents; // a sorted-list of non-overlapping thinning events
};

#endif
