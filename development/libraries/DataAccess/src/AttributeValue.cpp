#include <assert.h>

#include <iostream>
#include <sstream>

#include "AttributeValue.h"

using namespace DataAccess;
using namespace Interface;


AttributeValue::~AttributeValue (void)
{
}

AttributeValue::AttributeValue (const Parent * owner, unsigned int childIndex) : Child (owner, childIndex), m_type (NoAttributeType)
{
   u_double = 0;
}

AttributeValue::AttributeValue (const Parent * owner, unsigned int childIndex, bool value) : Child (owner, childIndex), m_type (Bool)
{
   u_bool = value;
}

AttributeValue::AttributeValue (const Parent * owner, unsigned int childIndex, int value) : Child (owner, childIndex), m_type (Int)
{
   u_int = value;
}

AttributeValue::AttributeValue (const Parent * owner, unsigned int childIndex, long value) : Child (owner, childIndex), m_type (Long)
{
   u_long = value;
}

AttributeValue::AttributeValue (const Parent * owner, unsigned int childIndex, float value) : Child (owner, childIndex), m_type (Float)
{
   u_float = value;
}

AttributeValue::AttributeValue (const Parent * owner, unsigned int childIndex, double value) : Child (owner, childIndex), m_type (Double)
{
   u_double = value;
}

AttributeValue::AttributeValue (const Parent * owner, unsigned int childIndex, const std::string & value) : Child (owner, childIndex), m_type (String)
{
   u_string = & value;
}

AttributeType AttributeValue::getType (void) const
{
   return m_type;
}

bool AttributeValue::getBool (void) const
{
   assert (getType () == Bool);
   return u_bool;
}

int AttributeValue::getInt (void) const
{
   assert (getType () == Int);
   return u_int;
}

long AttributeValue::getLong (void) const
{
   assert (getType () == Long);
   return u_long;
}

float AttributeValue::getFloat (void) const
{
   assert (getType () == Float);
   return u_float;
}

double AttributeValue::getDouble (void) const
{
   assert (getType () == Double);
   return u_double;
}

const std::string & AttributeValue::getString (void) const
{
   assert (getType () == String);
   return * u_string;
}

void AttributeValue::release (void) const
{
   Child::release ();
}

void AttributeValue::printOn (std::ostream & ostr) const
{
   std::string str;
   asString (str);
   ostr << str << std::endl;
}

void AttributeValue::asString (std::string & str) const
{
   std::ostringstream buf;

   const std::string typeNames[] = { "Bool", "Int", "Long", "Float", "Double", "String" };
   buf << "AttributeValue:";
   buf << " type = ";
   if (getType () != -1)
   {
      buf << typeNames[getType ()];
   }
   else
   {
      buf << "Unknown";
   }
   buf << ", value = ";

   switch (getType ())
   {
      case Bool:
	 buf << getBool ();
	 break;
      case Int:
	 buf << getInt ();
	 break;
      case Long:
	 buf << getLong ();
	 break;
      case Float:
	 buf << getFloat ();
	 break;
      case Double:
	 buf << getDouble ();
	 break;
      case String:
	 buf << getString ();
	 break;
      case NoAttributeType:
      default:
	 buf << "No Value";
	 break;
   }
   buf << std::endl;

   str = buf.str ();

}
