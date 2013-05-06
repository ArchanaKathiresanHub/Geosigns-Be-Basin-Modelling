#ifndef INVERSION_PROJECT_H
#define INVERSION_PROJECT_H

#include <string>
#include <vector>
#include <map>

namespace DataAccess { namespace Interface {
   class Formation;
   class ProjectHandle;
   class Surface;
} } 

class Project
{
public:
   Project(const std::string & input, const std::string & output );
   ~Project();

   void close();

   void discard();

   double setBasementProperty(const std::string & parameter, double newValue);

   typedef double Time; // in mA (million of years ago)
   typedef double Thickness; // in meters.

   typedef std::pair<Time, Thickness> ThicknessAtTime;

   void setCrustThickness( const std::vector< ThicknessAtTime> & series);

   void clearSnapshotTable();

   // Changes the thermal conductivity for a specific lithotype.
   // Lithotypes that are always used:
   //   - "Crust"
   //   - "Litho. Mantle"
   //   - "Astheno. Mantle"
   //   
   // Beside those, you can have many others, e.g.
   //   - "Std. Sandstone"
   //   - "Std. Shale"
   //   - ...
   //
   // The resulting thermal conductivity is the product of correctionFactor with
   // the previous thermal conductivity number
   void adjustThermalConductivity( const std::string & lithotype, double correctionFactor);

   void setLithotypeProperty( const std::string & property, const std::string & lithotype, double increment, double multiplicationFactor = 0.0);

   struct Formation : public std::string
   {
      double m_minThickness, m_maxThickness;
      double m_minAge, m_maxAge;
      bool m_constant;

      Formation( const DataAccess::Interface::Formation * formation);
   };

   // Returns which formations erode which other formations
   std::map< Formation, std::vector< Formation > > getErosionFormations() const;

   void setErosionThickness(const std::string & formationName, double thickness, double t0, double t1, double t2 );

   void addErosion( double thickness, double t0, double duration );

private:
   Project( const Project & ); // prohibit copying
   Project & operator=(const Project &); // prohibit assignment

   enum SurfaceOrder { UNDETERMINED, ABOVE, UNDER, INTERSECTING, EQUAL };

   // Compares the order of two surfaces
   static SurfaceOrder compare( const DataAccess::Interface::Surface * a, const DataAccess::Interface::Surface * b );

   std::string m_inputFileName, m_outputFileName;
   DataAccess::Interface::ProjectHandle * m_projectHandle;
};



#endif
