//
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by PDS BV.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//
#ifndef GEOPHYSICS__MULTI_COMPOUND_PROPERTY__H
#define GEOPHYSICS__MULTI_COMPOUND_PROPERTY__H

#include "ArrayDefinitions.h"
#include "FormattingException.h"
#include "GeoPhysicalConstants.h"

namespace GeoPhysics {

   /// \brief Contains multiple compound lithology values.
   ///
   ///
   class MultiCompoundProperty {

   public :

      /// \brief Default constructor.
      MultiCompoundProperty ();

      /// \brief Construct the with the number of lithologies used in the compound lithology and the number of data values required.
      ///
      /// \param [in] lithoCount The number of litholigies.
      /// \param [in] dataCount  The number of values.
      /// \pre lithoCount must be the same as the number of lithologies used in the
      MultiCompoundProperty ( const unsigned int lithoCount,
                              const unsigned int dataCount );

      ~MultiCompoundProperty ();

      /// \brief Resize the multi-component property object.
      ///
      /// If the dataCount matches the previous value then no reallocation will occur.
      /// \param [in] lithoCount The number of litholigies.
      /// \param [in] dataCount  The number of values.
      /// \pre lithoCount must be the same as the number of lithologies used in the
      void resize ( const unsigned int lithoCount,
                    const unsigned int dataCount );

      /// \brief Get the number of lithologies assigned to this object.
      unsigned int getNumberOfLithologies () const;

      /// \brief Get the number of data values assigned to this object.
      unsigned int getNumberOfValues () const;

      /// \brief Get the size of the allocated array.
      ///
      /// This may differ from the number of data values due to
      /// memory alignment requirements.
      /// This value is calculated:
      ///
      ///      if getNumberOfValues () mod ARRAY_ALIGMNENT == 0
      ///         n = getNumberOfValues ()
      ///      else
      ///         n = getNumberOfValues () + ARRAY_ALIGMNENT - getNumberOfValues () mod ARRAY_ALIGMNENT
      ///      end if
      ///
      unsigned int getLeadingDimension () const;


      /// \brief Get the data value for the simple lithology and data position.
      ///
      /// \param [in] lithology Get the simple data for this lithology index.
      /// \param [in] value     Get the simple data value for this position.
      /// \pre 0 <= lithology < getNumberOfLithologies ()
      /// \pre 0 <= value     < getNumberOfValues ()
      double getSimpleData ( const unsigned int lithology,
                             const unsigned int value ) const;

      /// \brief Get the data value for the simple lithology and data position.
      ///
      /// \param [in] lithology Get the simple data for this lithology index.
      /// \param [in] value     Get the simple data value for this position.
      /// \pre 0 <= lithology < getNumberOfLithologies ()
      /// \pre 0 <= value     < getNumberOfValues ()
      double& getSimpleData ( const unsigned int lithology,
                              const unsigned int value );


      /// \brief Get the data value array for the simple lithology.
      ///
      /// \param [in] lithology Get the simple data for this lithology index.
      /// \pre 0 <= lithology < getNumberOfLithologies ()
      ArrayDefs::ConstReal_ptr getSimpleData ( const unsigned int lithology ) const;

      /// \brief Get the data value array for the simple lithology.
      ///
      /// \param [in] lithology Get the simple data for this lithology index.
      /// \pre 0 <= lithology < getNumberOfLithologies ()
      ArrayDefs::Real_ptr getSimpleData ( const unsigned int lithology );


      /// \brief Get the data value for the mixed lithology and data position.
      double getMixedData ( const unsigned int value ) const;

      /// \brief Get the data value for the mixed lithology and data position.
      double& getMixedData ( const unsigned int value );


      /// \brief Get the data value array for the mixed lithology.
      ArrayDefs::ConstReal_ptr getMixedData () const;

      /// \brief Get the data value array for the mixed lithology.
      ArrayDefs::Real_ptr getMixedData ();


   private :

      /// \brief Allocate the data arrays.
      void allocate ( const unsigned int lithoCount,
                      const unsigned int dataCount );

      /// \brief Deallocates the data arrays and sets them to null.
      void deallocate ();

      /// \brief The current number of lithologies.
      unsigned int m_numberOfLithologies;

      /// \brief The number of values for each array.
      unsigned int m_dataCount;

      /// \brief The number of values that have been allocated for each array.
      ///
      /// This may differ so that the start of each array aligns on the correct memory address.
      unsigned int m_alignedArraySize;

      /// \brief Array containing all the data values
      ArrayDefs::Real_ptr m_allValues;

      /// \brief Array of arrays containing the property values for each lithology.
      ArrayDefs::Real_ptr m_simpleValues [ MaximumNumberOfLithologies ];

      /// \brief Array containing the property values for the mixed lithology.
      ArrayDefs::Real_ptr m_mixedValues;

   };

} // end namespace GeoPhysics

//------------------------------------------------------------//
// Inline functions
//------------------------------------------------------------//

inline unsigned int GeoPhysics::MultiCompoundProperty::getNumberOfLithologies () const {
   return m_numberOfLithologies;
}

inline unsigned int GeoPhysics::MultiCompoundProperty::getNumberOfValues () const {
   return m_dataCount;
}

inline unsigned int GeoPhysics::MultiCompoundProperty::getLeadingDimension () const {
   return m_alignedArraySize;
}

inline double& GeoPhysics::MultiCompoundProperty::getSimpleData ( const unsigned int lithology,
                                                                  const unsigned int value ) {

   if ( lithology >= m_numberOfLithologies or value >= m_dataCount ) {
      throw formattingexception::GeneralException () << " lithology or value out of bounds";
   }

   return m_simpleValues [ lithology ][ value ];
}

inline double GeoPhysics::MultiCompoundProperty::getSimpleData ( const unsigned int lithology,
                                                                 const unsigned int value ) const {

   if ( lithology >= m_numberOfLithologies or value >= m_dataCount ) {
      throw formattingexception::GeneralException () << " lithology or value out of bounds";
   }

   return m_simpleValues [ lithology ][ value ];
}

inline ArrayDefs::Real_ptr GeoPhysics::MultiCompoundProperty::getSimpleData ( const unsigned int lithology ) {

#if 0
   if ( lithology >= m_numberOfLithologies ) {
      throw formattingexception::GeneralException ();
   }
#endif

   return m_simpleValues [ lithology ];
}

inline ArrayDefs::ConstReal_ptr GeoPhysics::MultiCompoundProperty::getSimpleData ( const unsigned int lithology ) const {

#if 0
   if ( lithology >= m_numberOfLithologies ) {
      throw formattingexception::GeneralException ();
   }
#endif

   return m_simpleValues [ lithology ];
}

inline double& GeoPhysics::MultiCompoundProperty::getMixedData ( const unsigned int value ) {

   if ( value >= m_dataCount ) {
      throw formattingexception::GeneralException ();
   }

   return m_mixedValues [ value ];
}

inline double GeoPhysics::MultiCompoundProperty::getMixedData ( const unsigned int value ) const {

   if ( value >= m_dataCount ) {
      throw formattingexception::GeneralException ();
   }

   return m_mixedValues [ value ];
}

inline ArrayDefs::Real_ptr GeoPhysics::MultiCompoundProperty::getMixedData () {
   return m_mixedValues;
}


inline ArrayDefs::ConstReal_ptr GeoPhysics::MultiCompoundProperty::getMixedData () const {
   return m_mixedValues;
}


#endif // GEOPHYSICS__MULTI_COMPOUND_PROPERTY__H
