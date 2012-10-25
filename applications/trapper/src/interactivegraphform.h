/// Class InteractiveGraphForm inherits from GraphForm
/// and it allows the user to enable/disable graph lines
/// and zoom into the plot

#ifndef __interactivegraphform__
#define __interactivegraphform__

#include "graphform.h"

class QLabel;

class InteractiveGraphForm : public GraphForm
{
public:
   // CTOR / DTOR
   InteractiveGraphForm (QWidget* parent = 0, const char* name = 0, 
                         WFlags fl = 0);
   virtual ~InteractiveGraphForm () {}
   
protected slots:
   // PROTECTED SLOTS
   virtual void toggleCurve(long curveKey);
   
protected:
   // PROTECTED METHODS
   virtual void zoom(bool zoomOn);
};

#endif
