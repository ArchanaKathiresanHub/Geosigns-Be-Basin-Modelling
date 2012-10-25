/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/

#include "includestreams.h"
#include "componentgraphform.h"

const char* ComponentGraphOptionsFrame::componentType ()
{ 
   return compTypeChoice->currentText ();
}

const char* ComponentGraphOptionsFrame::phaseType ()
{
   return phaseChoice->currentText ();
}

const char* ComponentGraphOptionsFrame::conditionType ()
{
   return valueTypeChoice->currentText ();
}

void ComponentGraphOptionsFrame::updateGraphButtonSlot()
{
   dynamic_cast<ComponentGraphForm*>(parent())->updateGraph ();
}

