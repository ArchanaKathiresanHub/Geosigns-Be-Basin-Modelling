#ifndef _INTERFACE_INPUTVALUE_H_
#define _INTERFACE_INPUTVALUE_H_

#include "DAObject.h"
#include "Interface.h"

namespace database
{
   class Database;
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
         InputValue (ProjectHandle& projectHandle, database::Record * record);
         ~InputValue (void);

				 /// Return the type of the InputValue
				 virtual InputValueType getType (void) const;

				 /// Return the format identifier of the file
				 virtual const std::string & getMapType (void) const;

				 /// Return the name of the file from which the InputValue was read
				 virtual const std::string & getFileName (void) const;

				 virtual const std::string & getReferringTableName (void) const;
				 virtual const std::string & getMapName (void) const;

				 /// Return the GridMap that contains the values of this InputValue
				 virtual const GridMap * getGridMap (void) const;

				 /// Get the age at which the InputValue started to make an impact
				 virtual double getEventAge (void) const;

				 /// Get the name of the property associated with this InputValue
				 virtual const std::string & getPropertyName (void) const;

				 /// Get the name of the Surface associated with this InputValue
				 virtual const std::string & getSurfaceName (void) const;

				 /// Get the name of the Formation associated with this InputValue
				 virtual const std::string & getFormationName (void) const;

				 /// Get the name of the Reservoir associated with this InputValue
				 virtual const std::string & getReservoirName (void) const;

				 virtual void printOn (std::ostream &) const;

				 /// return a std::string description of this PropertyValue
				 virtual void asString (std::string &) const;

         /// Get the sequence number of the map.
         virtual int getMapSequenceNumber () const;

				 /// Get information about the hdf filename and datasetname for this inputmap when called from rank 0 only
				 void getHDFinfoSingleCore(std::string& fileName, std::string& dataSetName) const;

				 /// Get information about the hdf filename and datasetname for this inputmap
				 void getHDFinfo(std::string& fileName, std::string& dataSetName) const;

			 inline void setIndex(unsigned int index);
				 inline unsigned int getIndex (void);

				 unsigned int applyIndex (unsigned int index);

      private:
          static const unsigned int ValueMap = 0;

				 /// event attributes
				 mutable double m_eventAge;
				 mutable std::string m_propertyName;
				 mutable std::string m_surfaceName;
				 mutable std::string m_formationName;
				 mutable std::string m_reservoirName;

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
