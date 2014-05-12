#ifndef FASTCAULDRON_VITRINITEREFLECTANCE_H
#define FASTCAULDRON_VITRINITEREFLECTANCE_H

#include <string>
#include <iostream>

namespace GeoPhysics
{

/// Computes the vitrinite reflectance on the basis of temperature history
class VitriniteReflectance
{
public:

   class InputGrid
   { 
   public:
      virtual ~InputGrid() {}

      /// The temperature in Celsius
      virtual const double * getTemperature() const = 0;
   
      /// get the number of nodes in the grid 
      virtual int getSize() const = 0;

      /// get the indicices of the active nodes. 
      virtual const int * getActiveNodes() const = 0;

      /// get the number of active nodes
      virtual int getNumberOfActiveNodes() const = 0;

      /// time in Ma
      virtual double getTime() const = 0;
   };

   class OutputGrid 
   { 
   public:
      virtual ~OutputGrid() {}

      /// Get the array that stores the VR values
      virtual double * getVRe() = 0;

      /// get the number of nodes in the grid 
      virtual int getSize() const = 0;
   };

   virtual ~VitriniteReflectance() {}

   /// Reset the computation to the beginning of the basin.
   virtual void reset() = 0;

   /// Do to the computation for the next time step. 
   virtual void doTimestep( const InputGrid & previousGrid, const InputGrid & currentGrid ) = 0;

   /// Get the results.
   virtual void getResults( OutputGrid & ) = 0;

   /// Create a new VitriniteReflectance algorithm
   static VitriniteReflectance * create(const std::string & algorithmName);
};

}



#endif
