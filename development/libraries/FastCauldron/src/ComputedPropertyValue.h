#ifndef _FASTCAULDRON_COMPUTED_PROPERTY_VALUE_H_
#define _FASTCAULDRON_COMPUTED_PROPERTY_VALUE_H_

class ComputedProperty : public Interface::Property {

public :

   ComputedProperty ();


   bool outputIsRequested () const;


private :

};


class ComputedPropertyValue : public Interface::PropertyValue {

public :

   ComputedPropertyValue ();

   /// save a 2D PropertyValue to file
   bool saveMapToFile (MapWriter & mapWriter);

   /// save a 3D PropertyValue to file
   bool saveVolumeToFile (MapWriter & mapWriter);


   const ComputedProperty& getProperty () const;

   bool outputIsRequested () const;


private :

   const ComputedProperty* property;

};


#endif // _FASTCAULDRON_COMPUTED_PROPERTY_VALUE_H_
