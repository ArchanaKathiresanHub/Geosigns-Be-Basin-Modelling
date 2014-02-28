#ifndef INVERSION_PROPERTY_H
#define INVERSION_PROPERTY_H

class Scenario;

/// A Property is a range of values for a specific (set of) field(s) in a Cauldron project file.
/// To let the caller be agnostic of the types involved, an iteration interface (reset, 
/// nextValue, isPastEnd) is published. 'createParameter' should be called to
/// make a Parameter ( = an instantiation ) of the Property for the specific
/// value in the range.
class Property
{
public:
   virtual ~Property() {}

   /// Resets the iteration to the start of the sequence
   virtual void reset() = 0;

   /// Adds a new Parameter setting to an existing scenario.
   virtual void createParameter( Scenario & scenario ) const = 0;

   /// Go to the next value of the sequence
   virtual void nextValue() = 0;

   /// for DoE we also need often the last value of the interval
   virtual void lastValue() = 0;

   /// Returns 'true' when there are no more values in the sequence.
   virtual bool isPastEnd() const = 0;
};


#endif;
