#ifndef _INTERFACE_SNAPSHOT_H_
#define _INTERFACE_SNAPSHOT_H_

#include "AbstractSnapshot.h"
#include "Interface/DAObject.h"

namespace DataAccess
{
   namespace Interface
   {
      /// A Snapshot object contains a time for which PropertyValue objects were computed.
      /// These times have been system-generated or user-generated. System-generated times are
      /// all times of geological events captured by the basin model.
      class Snapshot : public DataModel::AbstractSnapshot, public DAObject
      {
	 public:
	    Snapshot (ProjectHandle * projectHandle, database::Record * record);
	    Snapshot (ProjectHandle * projectHandle, double time);
	    virtual ~Snapshot (void);
	    /// Return the time of this Snapshot
	    virtual double getTime (void) const;
	    /// return the name of the file in which 3D PropertyValues associated to this snapshot are
	    /// stored
	    virtual const string & getFileName (bool setIfNecessary = false) const;

       /// Return the snapshot-kind:
       ///   - System Generated; or
       ///   - User Defined; or
       ///   - "" (a null string).
       virtual const std::string& getKind () const;

	    virtual void asString (string &) const;
	    /// return the age of this snapshot in string form
	    virtual const string & asString (void) const;
	    virtual void printOn (ostream &) const;

	    virtual int getType (void) const;
	    virtual bool getUseInResQ (void) const;

       /// Set whether the snapshot file should be appended or not.
       virtual void setAppendFile ( const bool append );

       /// Indicates whether the snapshot file should be appended or not.
       virtual bool getAppendFile () const;

      private:
         mutable int m_type;
         double m_time;
         
         /// Indicates whether the snapshot file should be appended or not, the default if false.
         bool m_appendFile;
      };

      bool operator== (const Snapshot & ss1, const Snapshot & ss2);
      bool operator< (const Snapshot & ss1, const Snapshot & ss2);
      bool operator> (const Snapshot & ss1, const Snapshot & ss2);
      bool operator<= (const Snapshot & ss1, const Snapshot & ss2);
      bool operator>= (const Snapshot & ss1, const Snapshot & ss2);

      /// \brief Functor class for ordering snapshots.
      class SnapshotLessThan {

      public :

         bool operator ()( const Snapshot* s1,
                           const Snapshot* s2 ) const;

      };


   }
}


#endif // _INTERFACE_SNAPSHOT_H_
