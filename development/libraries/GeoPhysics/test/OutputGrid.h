#ifndef GEOPHYSICS_TEST_OUTPUTGRID_H
#define GEOPHYSICS_TEST_OUTPUTGRID_H

#include "../src/VitriniteReflectance.h"

#include <vector>

/** The class OutputGrid is declared which will be used to add functionality
    to methods that are needed in the InputGrid and OutputGrid nested classes to
	calculate VitriniteReflectance                                               */
class OutputGrid : public GeoPhysics::VitriniteReflectance::OutputGrid
{
public:
   OutputGrid( int theSize );

   virtual double * getVRe();
   virtual int getSize() const;
   double printVR() const;

private:
   int m_theSize;
   std::vector<double> m_storageArray;
};


#endif
