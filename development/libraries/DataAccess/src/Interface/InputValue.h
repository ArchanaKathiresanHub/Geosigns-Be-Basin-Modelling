#ifndef _INTERFACE_INPUTVALUE_H_
#define _INTERFACE_INPUTVALUE_H_

#include "Interface/DAObject.h"
#include "Interface/Interface.h"

namespace database
{
   class Database;
   class Transaction;
   class Table;
   class Record;
}

namespace DataAccess
{
   namespace Interface
   {

      /// An InputValue object models an input value represented by a map
      class InputValue : public DAObject
      {
	 public:
	    InputValue (ProjectHandle * projectHandle, database::Record * record);
	    ~InputValue (void);

	    /// Return the type of the InputValue
	    virtual InputValueType getType (void) const;

	    /// Return the format identifier of the file
	    virtual const string & getMapType (void) const;

	    /// Return the name of the file from which the InputValue was read
	    virtual const string & getFileName (void) const;

	    virtual const string & getReferringTableName (void) const;
	    virtual const string & getMapName (void) const;

	    /// Return the GridMap that contains the values of this InputValue
	    virtual const GridMap * getGridMap (void) const;

	    /// Get the age at which the InputValue started to make an impact
	    virtual double getEventAge (void) const;

	    /// Get the name of the property associated with this InputValue
	    virtual const string & getPropertyName (void) const;

	    /// Get the name of the Surface associated with this InputValue
	    virtual const string & getSurfaceName (void) const;

	    /// Get the name of the Formation associated with this InputValue
	    virtual const string & getFormationName (void) const;

	    /// Get the name of the Reservoir associated with this InputValue
	    virtual const string & getReservoirName (void) const;

	    virtual void printOn (ostream &) const;

	    /// return a string description of this PropertyValue
	    virtual void asString (string &) const;


	    inline void setIndex (unsigned int index);
	    inline unsigned int getIndex (void);

	    unsigned int computeIndex ();
	    unsigned int applyIndex (unsigned int index);

	    void computeChecksum (const string & directory, database::Transaction * transaction);
	    string saveToDirectory (const string & directory);
	    void convertToBPA (database::Transaction * transaction);

	 private:
            static const unsigned int ValueMap = 0;

	    /// event attributes
	    mutable double m_eventAge;
	    mutable string m_propertyName;
	    mutable string m_surfaceName;
	    mutable string m_formationName;
	    mutable string m_reservoirName;

	    bool fillEventAttributes (void) const;

	    /// number to be used for the creation of a HDF5 input map file
	    unsigned int m_index;

	    GridMap * loadGridMap (void) const;
      };

      void InputValue::setIndex (unsigned int index)
      {
	 m_index = index;
      }

      unsigned int InputValue::getIndex (void)
      {
	 return m_index;
      }
   }
}

#endif // _INTERFACE_INPUTVALUE_H_
