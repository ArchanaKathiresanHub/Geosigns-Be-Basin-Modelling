#ifndef FASTPROPERTIES_OUTPUT_PROPERTY_VALUE__H
#define FASTPROPERTIES_OUTPUT_PROPERTY_VALUE__H

#include "Interface/Interface.h"
#include "AbstractProperty.h"
#include "AbstractSurface.h"

class OutputPropertyValue {

public :

   OutputPropertyValue ( const DataModel::AbstractProperty* property );

   virtual ~OutputPropertyValue () {}

   virtual double getValue ( const double i, const double j, const double k ) const = 0;

   virtual unsigned int getDepth () const = 0;

   virtual double getUndefinedValue () const;
   
   virtual const string & getName() const = 0;

   virtual bool hasMap() const = 0;

   virtual const DataModel::AbstractSurface* getSurface() const { return 0; }

   virtual const DataModel::AbstractProperty* getProperty () const;

private :

   const DataModel::AbstractProperty* m_property;

};

typedef boost::shared_ptr<OutputPropertyValue> OutputPropertyValuePtr;

inline OutputPropertyValue::OutputPropertyValue ( const DataModel::AbstractProperty* property ) : m_property ( property ) {}

inline const DataModel::AbstractProperty* OutputPropertyValue::getProperty () const {
   return m_property;
}


inline double OutputPropertyValue::getUndefinedValue () const {
   return DataAccess::Interface::DefaultUndefinedMapValue;
}

#endif // FASTPROPERTIES_OUTPUT_PROPERTY_VALUE__H
