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

   // Adds a thinning events if-and-only-if the event not overlap with an other
   // event. In that case it also returns true. When the event does overlap, it
   // doesn't add anything and returns false.
   // Note: Duration and ratio must be positive (> 0) numbers.
   bool addThinningEvent(double startTime, double duration, double ratio);

private:
   typedef double Duration;
   typedef double Ratio;
   typedef std::map<Time, std::pair< Duration, Ratio> > ThinningEvents ;

   static const double MinimumEventSeparation;

   static bool overlap(double t0, double dt0, double t1, double dt1);

   double m_initialThickness;       // The initial thickness of the crust
   ThinningEvents m_thinningEvents; // a sorted-list of non-overlapping thinning events
};

#endif
