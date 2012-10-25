#ifndef MAPLISTVIEWITEM_H
#define MAPLISTVIEWITEM_H
#include <qlistview.h>

#include <BitField.h>

namespace DataAccess
{
   namespace Interface
   {
      class InputValue;
      class PropertyValue;
   }
}

const unsigned int MapSelectedForUpload = 1 << 0;
const unsigned int MapUploaded = 1 << 1;
const unsigned int MapSelectedForRemoval = 1 << 2;

template <class ValueType> class MapListViewItem : public QListViewItem, public BitField
{

public:
   MapListViewItem ( QListView * parent );
   MapListViewItem ( QListViewItem * parent );

   inline void setAttribute (const ValueType * attr);
   inline const ValueType * getAttribute (void) const;
   inline unsigned int getIndex (void) const;

protected:
   unsigned int m_index;

   unsigned int m_flags;

private:
   static unsigned int s_lastIndex;
   const ValueType * m_attribute;
};

template <class ValueType>
void MapListViewItem<ValueType>::setAttribute (const ValueType * attr)
{
   m_attribute = attr;
}

template <class ValueType>
const ValueType * MapListViewItem<ValueType>::getAttribute (void) const
{
   return m_attribute;
}

template <class ValueType>
unsigned int MapListViewItem<ValueType>::getIndex (void) const
{
   return m_index;
}

class InputMapListViewItem : public MapListViewItem < DataAccess::Interface::InputValue >
{
   public:
   enum Column { AgeColumn = 0, PropertyColumn, SurfaceColumn, FormationColumn, ReservoirColumn };

   InputMapListViewItem ( QListView * parent );
   InputMapListViewItem ( QListViewItem * parent );

   virtual int compare (QListViewItem * item, int column, bool ascending) const;
};

class OutputMapListViewItem : public MapListViewItem < DataAccess::Interface::PropertyValue >
{
   public:
   enum Column { AgeColumn = 0, PropertyColumn, SurfaceColumn, FormationColumn, ReservoirColumn, ServerColumn };

   OutputMapListViewItem ( QListView * parent );
   OutputMapListViewItem ( QListViewItem * parent );

   virtual int compare (QListViewItem * item, int column, bool ascending) const;

   bool selectedForUpload (void);
   bool selectedForRemoval (void);
   bool uploaded (void);

   void setSelectForUpload (bool mode);
   void setSelectForRemoval (bool mode);
   void setUploaded (bool mode);

   void updateServerText ();
};


#endif // MAPLISTVIEWITEM_H
