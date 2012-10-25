#include "Interface/PropertyValue.h"
#include "Interface/InputValue.h"

using namespace DataAccess;
using namespace Interface;

#include "maplistviewitem.h"

template <class ValueType>
unsigned int MapListViewItem<ValueType>::s_lastIndex = 0;

template <class ValueType>
MapListViewItem<ValueType>::MapListViewItem ( QListView * parent ) : QListViewItem (parent)
{
   m_index = ++s_lastIndex;
}

template <class ValueType>
MapListViewItem<ValueType>::MapListViewItem ( QListViewItem * parent ) : QListViewItem (parent)
{
   m_index = ++s_lastIndex;
}

InputMapListViewItem::InputMapListViewItem ( QListView * parent ) : MapListViewItem<InputValue> (parent)
{
}

InputMapListViewItem::InputMapListViewItem ( QListViewItem * parent ) : MapListViewItem<InputValue> (parent)
{
}

int InputMapListViewItem::compare (QListViewItem * item, int column, bool ascending) const
{
   InputMapListViewItem * ivi = (InputMapListViewItem *) item;

   if (column == 2 || column == 3 || column == 4)
   {
      if (!text (column).isEmpty () && ivi->text (column).isEmpty ())
	 return -1;
      else if (text (column).isEmpty () && !ivi->text (column).isEmpty ())
	 return 1;
      else
	 return (getIndex () < ivi->getIndex () ? -1 : 1);
   }
   else
   {
      return MapListViewItem<InputValue>::compare (ivi, column, ascending);
   }
}

OutputMapListViewItem::OutputMapListViewItem ( QListView * parent ) : MapListViewItem<PropertyValue> (parent)
{
}

OutputMapListViewItem::OutputMapListViewItem ( QListViewItem * parent ) : MapListViewItem<PropertyValue> (parent)
{
}

int OutputMapListViewItem::compare (QListViewItem * item, int column, bool ascending) const
{
   OutputMapListViewItem * ovi = (OutputMapListViewItem *) item;

   if (column == 0)
   {
      // return MapListViewItem<PropertyValue>::compare (ovi, column, ascending);
      return getAttribute ()->compareByAge (ovi->getAttribute ());
   }
   else if (column == 1)
   {
      return MapListViewItem<PropertyValue>::compare (ovi, column, ascending);
   }
   else if (column == 2 || column == 3 || column == 4)
   {
      if (!text (column).isEmpty () && ovi->text (column).isEmpty ())
	 return -1;
      else if (text (column).isEmpty () && !ovi->text (column).isEmpty ())
	 return 1;
      else
	 return getAttribute ()->compareByDepoAge (ovi->getAttribute ());
   }
   else
   {
      return MapListViewItem<PropertyValue>::compare (ovi, column, ascending);
   }
}

bool OutputMapListViewItem::selectedForUpload (void)
{
   return BitValueIsTrue (MapSelectedForUpload);
}

bool OutputMapListViewItem::selectedForRemoval (void)
{
   return BitValueIsTrue (MapSelectedForRemoval);
}

bool OutputMapListViewItem::uploaded (void)
{
   return BitValueIsTrue (MapUploaded);
}

void OutputMapListViewItem::setSelectForUpload (bool mode)
{
   SetBitValue (MapSelectedForUpload, mode);
   updateServerText ();
}

void OutputMapListViewItem::setSelectForRemoval (bool mode)
{
   SetBitValue (MapSelectedForRemoval, mode);
   updateServerText ();
}

void OutputMapListViewItem::setUploaded (bool mode)
{
   SetBitValue (MapUploaded, mode);
   updateServerText ();
}

void OutputMapListViewItem::updateServerText (void)
{
   QString text;

   if (uploaded ())
   {
      text.append ("On Server");
   }

   if (selectedForRemoval ())
   {
      text.append (", Remove");
   }

   if (selectedForUpload ())
   {
      text.append ("Upload");
   }

   setText (ServerColumn, text);
}
