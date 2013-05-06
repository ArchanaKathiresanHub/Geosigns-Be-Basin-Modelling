#ifndef INVERSION_PARAMETER_H
#define INVERSION_PARAMETER_H

class Project;

class Parameter
{
public:
   virtual ~Parameter() {}
   virtual void print()=0;
   virtual void changeParameter(Project & project)=0;
};

#endif

