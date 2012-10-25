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
 
#include <stdlib.h>

//
// PUBLIC METHODS 
//

void AxisRange::init()
{
   setMin (0);
   setMax (0);
}

void AxisRange::setHeading( const char * heading )
{ 
   this->setCaption (heading);
}

// override virtual show function
void AxisRange::show()
{
   setMinText();
   setMaxText();

   QDialog::show ();
}


void AxisRange::setMin( double min )
{
   m_min = min;
}

void AxisRange::setMax( double max )
{
   m_max = max;
}

double AxisRange::getMin()
{
   return m_min;
}

double AxisRange::getMax()
{
   return m_max;
}

//
// PUBLIC SLOTS
//
void AxisRange::okaySlot()
{
   readMinText();
   readMaxText();

   cancelSlot();
}


void AxisRange::cancelSlot()
{
   close ();
}

//
// PRIVATE METHODS
//
void AxisRange::readMinText()
{
   m_min = atof (minEdit->text ());
}

void AxisRange::readMaxText()
{
   m_max = atof (maxEdit->text ());
}

void AxisRange::setMinText()
{
   char temp[50];
   sprintf(temp, "%g", m_min);
   minEdit->setText (temp);
}

void AxisRange::setMaxText()
{
   char temp[50];
   sprintf(temp, "%g", m_max);
   maxEdit->setText (temp);
}







