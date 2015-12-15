#ifndef GEOPHYSICS_TEST_INPUTGRID_H
#define GEOPHYSICS_TEST_INPUTGRID_H

#include "../src/VitriniteReflectance.h"
#include <vector>


/** The class InputGrid is declared which will be used to add functionality
    to methods that are needed in the InputGrid and OutputGrid nested classes to 
	calculate VitriniteReflectance                                               */
class InputGrid : public GeoPhysics::VitriniteReflectance::InputGrid
{
public:
   InputGrid( double theTime, double theTemperature, int theSize );

   virtual const double * getTemperature() const;
   virtual int getSize() const;
   virtual const int * getActiveNodes() const;
   virtual int getNumberOfActiveNodes() const;
   virtual double getTime() const;

private:
   int m_theSize;
   double m_theTime;
   std::vector<double> m_theTemperature;
   std::vector<int>  m_theActiveNodes;
};

#endif
