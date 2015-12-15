#ifndef _INTERFACE_ATTRIBUTEVALUE_H
#define _INTERFACE_ATTRIBUTEVALUE_H

#include <string>
using namespace std;

#include "Interface/Child.h"
#include "Interface/Interface.h"

namespace DataAccess
{
   namespace Interface
   {
      class Parent;
      /// An AttributeValue contains the value of the Attribute for which it was requested.
      /// See for instance LithoType::getAttributeValue ().
      class AttributeValue : public Child
      {
	 public:
	    /// constructors
	    AttributeValue (const Parent * owner, unsigned int childIndex);
	    AttributeValue (const Parent * owner, unsigned int childIndex, bool value);
	    AttributeValue (const Parent * owner, unsigned int childIndex, int value);
	    AttributeValue (const Parent * owner, unsigned int childIndex, long value);
	    AttributeValue (const Parent * owner, unsigned int childIndex, float value);
	    AttributeValue (const Parent * owner, unsigned int childIndex, double value);
	    AttributeValue (const Parent * owner, unsigned int childIndex, const string & value);

	    virtual ~AttributeValue (void);

	    /// Return the AttributeType of this AttributeValue
	    virtual AttributeType getType (void) const;

	    /// Return the bool value of this AttributeValue.
	    /// Its behavior is undefined if the INTERFACE::AttributeType of this AttributeValue is not INTERFACE::Bool
	    virtual bool getBool (void) const;
	    /// Return the int value of this AttributeValue.
	    /// Its behavior is undefined if the INTERFACE::AttributeType of this AttributeValue is not INTERFACE::Int
	    virtual int getInt (void) const;
	    /// Return the long value of this AttributeValue.
	    /// Its behavior is undefined if the INTERFACE::AttributeType of this AttributeValue is not INTERFACE::Long
	    virtual long getLong (void) const;
	    /// Return the float value of this AttributeValue.
	    /// Its behavior is undefined if the INTERFACE::AttributeType of this AttributeValue is not INTERFACE::Float
	    virtual float getFloat (void) const;
	    /// Return the double value of this AttributeValue.
	    /// Its behavior is undefined if the INTERFACE::AttributeType of this AttributeValue is not INTERFACE::Double
	    virtual double getDouble (void) const;
	    /// Return the string value of this AttributeValue.
	    /// Its behavior is undefined if the INTERFACE::AttributeType of this AttributeValue is not INTERFACE::String
	    virtual const string & getString (void) const;

	    /// cease usage of this object
	    /// Informs the Parent object that the user application has stopped using this AtttributeValue so that
	    /// it may be deleted.
	    virtual void release (void) const;

	    virtual void printOn (ostream &) const;

	    /// Return a string description of the object
	    virtual void asString (string &) const;

	 private:
	    AttributeType m_type;

	    union {
	       bool u_bool;
	       int u_int;
	       long u_long;
	       float u_float;
	       double u_double;
	       const string * u_string;
	    };
      };
   }
}

#endif // _INTERFACE_ATTRIBUTEVALUE_H
