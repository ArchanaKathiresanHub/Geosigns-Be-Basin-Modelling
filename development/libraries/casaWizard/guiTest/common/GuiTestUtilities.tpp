//
// Copyright (C) 2012 - 2023 Shell International Exploration & Production.
// All rights reserved.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <QWidget>
#include <QApplication>
#include <QTest>
#include <QDialog>
#include <QAbstractButton>

namespace guiUtilities
{

template <class Type> QList<Type> topLevelWidgets() {
   QList<Type> widgets;
   for (QWidget* widget : QApplication::topLevelWidgets())
   {
      if (widget->isVisible())
      {
         if (auto* tmp = qobject_cast<Type>(widget))
         {
            widgets << tmp;
         }
      }
   }
   return widgets;
}

template <class Type> Type topLevelWidget()
{
   auto widgets = topLevelWidgets<Type>();
   return (widgets.size() >= 1) ? widgets.first() : nullptr;
}

template <typename Type>
void pressButtonInDialog(const bool accept)
{
   if (auto* dialog = dynamic_cast<QDialog*>(topLevelWidget<Type*>()))
   {
      accept ?  QTest::keyClick(dialog, Qt::Key_Enter, Qt::NoModifier) :
                QTest::keyClick(dialog, Qt::Key_Escape, Qt::NoModifier);
   }
}

template <class Type> Type* getQChild(const QString& name, const QWidget* parent)
{
   QList<Type*> allChildren;
   for (auto* child : parent->children() )
   {
      if (Type* object = qobject_cast<Type*>(child) )
      {
         allChildren.push_back( object );
      }
   }

   //return first child with matching description (name)
   for (Type* child : allChildren )
   {
         if (qobject_cast<QWidget*>(child)->accessibleName() == name ||
             qobject_cast<QAbstractButton*>(child)->text() == name)
         {
            return child;
         }
   }
   return nullptr;
}

} // namespace guiUtilities
