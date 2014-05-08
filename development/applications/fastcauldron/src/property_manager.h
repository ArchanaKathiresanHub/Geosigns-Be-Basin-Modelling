//------------------------------------------------------------//

#ifndef __Property_Manager_HH__
#define __Property_Manager_HH__

//------------------------------------------------------------//

#include <string>
#include <petsc.h>
#include "PetscVectors.h"

//------------------------------------------------------------//

#include "FiniteElementTypes.h"
#include "globaldefs.h"

#include "Mesh3DIndex.h"
#include "Nodal3DIndexArray.h"

using namespace FiniteElementMethod;

//------------------------------------------------------------//

///
/// Forward definition of AppCtx
///
class AppCtx;

namespace Basin_Modelling {


  enum Fundamental_Property { 
        //
        //
        // Should these properties be in here?
        //
        Depth, Real_Thickness, Solid_Thickness,
        //
        //
        // Even though the type name is 'Fundamental_Property', we will 
        // still store ALL of the pressures ( Ph, Pl, Po, Pp = Ph + Po )
        //
        Hydrostatic_Pressure, Lithostatic_Pressure, Overpressure, Pore_Pressure, 
        //
        //
        // \phi_{cc} the time-ves-temperature-dependant porosity term
        //
        Chemical_Compaction,
        //
        //
        // Again VES is stored ( VES = Pl - Pp = Pl - ( Ph + Po ))
        //
        // VES_FP = VES_Fundamental_Property, just to disambiguate it from 
        // VES in the timeFilter::PropertyList.
        // Something else may have to be done in the near future
        // namespace for the timeFilter, _FP on all properties here, ...)
        //
        VES_FP, Max_VES,
        //
        //
        //
        //
        Temperature,
        //
        //
        // Add all other properties BEFORE this
        //
        No_Property };


  const std::string& fundamentalPropertyImage ( const Fundamental_Property property );

   /// Returns the name used of the cauldron property associated with the funcamental-property.
   const std::string& fundamentalPropertyName ( const Fundamental_Property property );

  Fundamental_Property operator++ ( Fundamental_Property& Property );
  Fundamental_Property operator++ ( Fundamental_Property& Property, const int Dummy );


  const int Number_Of_Fundamental_Properties = int ( No_Property );

  ///
  /// Since we are using linear hexahedra elements, there are 8 nodes per element.
  ///
  const int Number_Of_Nodes = 8;


  //------------------------------------------------------------//


  typedef Vec* Vec_Ptr;

  typedef DM*  DA_Ptr;

  typedef DM const*  DA_Const_Ptr;

  typedef PETSC_3D_Array* PETSC_3D_Array_Ptr;

  typedef PETSC_3D_Array const* PETSC_3D_Array_Const_Ptr;


  //------------------------------------------------------------//


  class Property_Array {

  public :

    PetscScalar operator ()( const Fundamental_Property Property ) const {

//        #ifdef __Constraint_Check__
//        if ( Property == No_Property ) {
//          throw Constraint_Error ( " Some message", __LINE__, __FUNCT__, __FILE__ );
//        } // end if
//        #endif

      return Properties [ Property ];
    } // end Property_Array::operator ()

    PetscScalar& operator ()( const Fundamental_Property Property ) {

//        #ifdef __Constraint_Check__
//        if ( Property == No_Property ) {
//          throw Constraint_Error ( " Some message", __LINE__, __FUNCT__, __FILE__ );
//        } // end if
//        #endif

      return Properties [ Property ];
    } // end Property_Array::operator ()


    void Fill ( const PetscScalar With_The_Value ); // is this needed?


  private :

    PetscScalar Properties [ Number_Of_Fundamental_Properties ];

  }; // end class Property_Array


  //------------------------------------------------------------//


  struct Property_Position {
    int X_Position;
    int Y_Position;
    int Z_Position;
  }; // end struct Property_Position


  class Element_Positions {

  public :

    void Set_Node_Position ( const int                Node_Number,
                             const Property_Position& Entry );

    void Set_Node_Position ( const int                nodeNumber,
                             const int                newZPosition,
                             const int                newYPosition,
                             const int                newXPosition );

    Property_Position operator ()( const int Node_Number ) const {
      return Entries [ Node_Number ];
    }

    int X_Position ( const int Node_Number ) const {
      return Entries [ Node_Number ].X_Position;
    }

    int Y_Position ( const int Node_Number ) const {
      return Entries [ Node_Number ].Y_Position;
    }

    int Z_Position ( const int Node_Number ) const {
      return Entries [ Node_Number ].Z_Position;
    }

  private :

    Property_Position Entries [ Number_Of_Nodes ];

  }; // end class Element_Positions


  //------------------------------------------------------------//


  class Fundamental_Property_Manager {

  public :

    Fundamental_Property_Manager ();
//      Fundamental_Property_Manager ( const DA* Layer_Array );
    ~Fundamental_Property_Manager ();

    void Set_Layer_DA ( const DM* Layer_Array );

    void Set_Property_Vector ( const Fundamental_Property Property_Value, Vec* Property_Vector );


    //----------------------------//


    void Create_Properties ();

    void Create_Property ( const Fundamental_Property Property );


    void Activate_Property   ( const Fundamental_Property Property,
                               const InsertMode           Mode = INSERT_VALUES,
                               const bool                 Include_Ghost_Values = false );

    void Activate_Properties ( const InsertMode           Mode = INSERT_VALUES,
                               const bool                 Include_Ghost_Values = false );


//      void Activate_Properties ( const int                   Number_Of_Z_Nodes );

//      void Activate_Property   ( const int                   Number_Of_Z_Nodes,
//                                 const Fundamental_Property  Property );

    bool propertyIsActivated ( const Fundamental_Property Property ) const;

    void Restore_Property    ( const Fundamental_Property  Property );

    void Restore_Properties ();


    //----------------------------//


    void Extract_Property    ( const Fundamental_Property  Property, 
                               const Property_Position&    Position, 
                                     PetscScalar&          Property_Value  ) const;

    void Extract_Property    ( const Fundamental_Property  Property, 
                               const int                   Z_Position, 
                               const int                   Y_Position, 
                               const int                   X_Position, 
                                     PetscScalar&          Property_Value  ) const;

    void Extract_Property    ( const Fundamental_Property  Property, 
                               const Element_Positions&    Positions, 
                                     ElementVector&       Property_Values ) const;

    void Extract_Property    ( const Fundamental_Property  property, 
                               const Nodal3DIndexArray&    indices, 
                                     ElementVector&        propertyValues ) const;



    void Extract_Properties  ( const Property_Position&    Position, 
                                     Property_Array&       Property_Values ) const;

    void Extract_Properties  ( const int                   Z_Position, 
                               const int                   Y_Position, 
                               const int                   X_Position, 
                                     Property_Array&       Property_Values ) const;

//      void Extract_Properties  ( const Element_Positions&    Positions, 
//                                       Element_Properties&   Property_Values ) const;


    //----------------------------//

    void Set_Property        ( const Fundamental_Property  Property, 
                               const Property_Position&    Position, 
                               const PetscScalar&          Property_Value  );

    void Set_Property        ( const Fundamental_Property  Property, 
                               const int                   Z_Position, 
                               const int                   Y_Position, 
                               const int                   X_Position, 
                               const PetscScalar&          Property_Value  );

    //----------------------------//

     /// Return the value for the property at the position.
     ///
     /// Since the property-manager is designed for a layer then the index will use the local-k index.
     PetscScalar  operator () ( const Fundamental_Property  Property, 
                                const Mesh3DIndex&          index ) const;
     
     /// Return the value for the property at the position.
     PetscScalar  operator () ( const Fundamental_Property  Property,
                                const int                   Z_Position, 
                                const int                   Y_Position, 
                                const int                   X_Position ) const;


     /// Return the value for the property at the position.
     ///
     /// Since the property-manager is designed for a layer then the index will use the local-k index.
     PetscScalar& operator () ( const Fundamental_Property  Property, 
                                const Mesh3DIndex&          index );

     /// Return the value for the property at the position.
     PetscScalar& operator () ( const Fundamental_Property  Property,
                                const int                   Z_Position, 
                                const int                   Y_Position, 
                                const int                   X_Position );

    //----------------------------//

//      PETSC_3D_Array_Const_Ptr operator ()( const Fundamental_Property Property ) const {
//        return &Properties [ Property ];
//      } // end Fundamental_Property_Manager::operator ()


    Vec operator ()( const Fundamental_Property Property ) const {
      return *Vector_Properties [ Property ];
    } // end Fundamental_Property_Manager::operator ()

    Vec operator ()( const Fundamental_Property Property ) {
      return *Vector_Properties [ Property ];
    } // end Fundamental_Property_Manager::operator ()

    //----------------------------//

    void Copy ( const Fundamental_Property_Manager& Manager,
                const Boolean2DArray&               Valid_Needle );

    //----------------------------//

  private :

    void Copy_Vector ( const Boolean2DArray& Valid_Needle,
                       const int             X_Start,
                       const int             X_End,
                       const int             Y_Start,
                       const int             Y_End,
                       const int             Z_Start,
                       const int             Z_End,
                       const DM              Layer_DA,
                       const Vec             Current_Property,
                             Vec&            Previous_Property ) const;


    PETSC_3D_Array Properties        [ Number_Of_Fundamental_Properties ];
    Vec_Ptr        Vector_Properties [ Number_Of_Fundamental_Properties ];
    bool           Property_Active   [ Number_Of_Fundamental_Properties ];

    DA_Const_Ptr   Layer_DA;
//      DA_Ptr         Layer_DA;

  }; // end class Fundamental_Property_Manager


  //------------------------------------------------------------//


//    class Element_Nodal_Properties {

//    public :

//      Element_Nodal_Properties ();


//      PetscScalar operator ()( const Fundamental_Property Property,
//                               const int                  Node ) const {

//        return Properties [ Property ][ Node ];

//      } // end operator ()

//      //----------------------------//

//      PetscScalar& operator ()( const Fundamental_Property Property,
//                                const int                  Node ) {

//        return Properties [ Property ][ Node ];

//      } // end operator ()

//      //----------------------------//

//      PetscScalar Evaluate_Property ( const Fundamental_Property Property,
//                                      const ElementVector&      Coefficients ) const;


//      void Evaluate_Properties ( const ElementVector& Coefficients, 
//                                       Property_Array& Property_Values ) const;


//    private :

//      PetscScalar  All_Properties [ Number_Of_Nodes * Number_Of_Fundamental_Properties ];
//      PetscScalar* Properties     [ Number_Of_Fundamental_Properties ];

//    }; // end class Element_Nodal_Properties


  ///------------------------------------------------------------//
  ///
  /// Functions to compute derived properties
  ///


  ///
  /// Compute the bulk density and fill layer vectors (BulkDensity)
  ///
  void computeBulkDensityVectors ( AppCtx* basinModel );

  ///
  /// Delete the bulk density vectors stored in the layers
  ///
  void deleteBulkDensityVectors ( AppCtx* basinModel );

  ///
  /// Compute the velocity and fill layer vectors 
  ///
  void computeVelocityVectors ( AppCtx* basinModel );

  ///
  /// Delete the velocity vectors stored in the layers
  ///
  void deleteVelocityVectors ( AppCtx* basinModel );

  ///
  /// Compute the sonic and fill layer vectors 
  ///
  void computeSonicVectors ( AppCtx* basinModel );

  ///
  /// Delete the Sonic vectors stored in the layers
  ///
  void deleteSonicVectors ( AppCtx* basinModel );

  ///
  /// Compute the Reflectivity and fill layer vectors 
  ///
  void computeReflectivityVectors ( AppCtx* basinModel );

  ///
  /// Delete the Reflectivity vectors stored in the layers
  ///
  void deleteReflectivityVectors ( AppCtx* basinModel );

  ///
  /// Compute the ThermalConductivity and fill layer vectors 
  ///
  void computeThermalConductivityVectors ( AppCtx* basinModel );

  ///
  /// Delete the ThermalConductivity vectors stored in the layers
  ///
  void deleteThermalConductivityVectors ( AppCtx* basinModel );

  ///
  /// Compute the Diffusivity and fill layer vectors 
  ///
  void computeDiffusivityVectors ( AppCtx* basinModel );

  ///
  /// Delete the Diffusivity vectors stored in the layers
  ///
  void deleteDiffusivityVectors ( AppCtx* basinModel );

  ///
  /// Compute the Thickness and fill layer vectors 
  ///
  void computeThicknessVectors ( AppCtx* basinModel );

  ///
  /// Delete the Thickness vectors stored in the layers
  ///
  void deleteThicknessVectors ( AppCtx* basinModel );

  ///
  /// Compute the Permeability and fill layer vectors 
  ///
  void computePermeabilityVectors ( AppCtx* basinModel );


  ///
  /// Compute the maps for each layer of which elements contains a fault lithology
  ///
  void computeFaultElementMaps ( AppCtx* basinModel );

  ///
  ///
  void deleteFaultElementMaps ( AppCtx* basinModel );

  /// Compute the maps for each layer holding the ratio of current thickness to deposition thickness.
  ///
  /// The thicknesses used will be:
  ///   - Solid thickness for geometric loop; and
  ///   - Real thickness for non-geometric loop.
  void computeErosionFactorMaps ( AppCtx* basinModel,
                                  const double    age );

  ///
  void deleteErosionFactorMaps ( AppCtx* basinModel );

  /// Compute the maps for each layer of which elements contains an allochthonous lithology
  void computeAllochthonousLithologyMaps ( AppCtx* basinModel );

  void deleteAllochthonousLithologyMaps ( AppCtx* basinModel );

  void collectAndSaveIsoValues(const double Current_Time, AppCtx *basinModel );
  void updateSedimentBottomSurfaceCurves(const double Current_Time, AppCtx *basinModel);


  void computeBasementLithostaticPressure ( AppCtx* basinModel,
					    const double    age );

  void computeBasementLithostaticPressureForCurrentTimeStep ( AppCtx* basinModel,
                                                              const double    age );



} // end namespace Basin_Modelling


//------------------------------------------------------------//
// Inline functions.
//------------------------------------------------------------//

inline PetscScalar Basin_Modelling::Fundamental_Property_Manager::operator ()( const Fundamental_Property  Property, 
                                                                               const Mesh3DIndex&          index ) const {
   return Properties [ Property ]( index.getLocalK (), index.getJ (), index.getI ());
}

//------------------------------------------------------------//

inline PetscScalar Basin_Modelling::Fundamental_Property_Manager::operator () ( const Fundamental_Property  Property,
                                                                                const int                   Z_Position, 
                                                                                const int                   Y_Position, 
                                                                                const int                   X_Position ) const {
  return Properties [ Property ]( Z_Position, Y_Position, X_Position );
}

//------------------------------------------------------------//

inline PetscScalar& Basin_Modelling::Fundamental_Property_Manager::operator ()( const Fundamental_Property  Property, 
                                                                                const Mesh3DIndex&          index ) {
   return Properties [ Property ]( index.getLocalK (), index.getJ (), index.getI ());
}

//------------------------------------------------------------//

inline PetscScalar& Basin_Modelling::Fundamental_Property_Manager::operator () ( const Fundamental_Property  Property,
                                                                                 const int                   Z_Position, 
                                                                                 const int                   Y_Position, 
                                                                                 const int                   X_Position ) {
  return Properties [ Property ]( Z_Position, Y_Position, X_Position );
}

//------------------------------------------------------------//

#endif // __Property_Manager_HH__

//------------------------------------------------------------//
