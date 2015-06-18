#ifndef TRACK1D__OUTPUT_PROPERTY_VALUE__H
#define TRACK1D__OUTPUT_PROPERTY_VALUE__H

#include "Interface/Interface.h"

class OutputPropertyValue {

public :

   OutputPropertyValue () {}

   virtual ~OutputPropertyValue () {}

   virtual double getValue ( const double i, const double j, const double k ) const = 0;

   virtual unsigned int getDepth () const = 0;

   virtual double getUndefinedValue () const;


};

typedef boost::shared_ptr<OutputPropertyValue> OutputPropertyValuePtr;


inline double OutputPropertyValue::getUndefinedValue () const {
   return DataAccess::Interface::DefaultUndefinedMapValue;
}

#endif // TRACK1D__OUTPUT_PROPERTY_VALUE__H
