//                                                                      
// Copyright (C) 2012-2014 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
// 

/// @file PrmPermeabilityModel.h
/// @brief This file keeps API declaration for lithology permeability model parameters handling. 

#ifndef CASA_API_PARAMETER_PERMEABITY_MODEL_H
#define CASA_API_PARAMETER_PERMEABITY_MODEL_H

#include "Parameter.h"
#include "LithologyManager.h"

// CMB API
#include <UndefinedValues.h>
#include <cmbAPI.h>
#include <cassert>

namespace mbapi
{
   class Model;
}

/// @page CASA_PermeabilityModelPage Lithology permeability model
/// 
/// This parameter defines the type of permeability model and it parameters for a lithology
/// Here is the list of supported permeability model types:
/// -# None
/// -# Impermeable
/// -# Sandstone
/// -# Mudstone
/// -# Multipoint
///
/// To define lithology permeability model user should specify 
/// - Lithology name
/// - Permeability model type
/// - Permeability model dependent parameters
///
/// @anchor perm_vs_por
/// @image html PermeabilityModels.jpg "Figure 1: Temis and Cauldron porosity permeability trends for default clastic lithologies"
///
/// @anchor stand_lith_perm
/// Table 1. Cauldron Default Lithologies. 
/// |   Lithology      | Perm.      Model | Depo Perm [mD] | Perm Coef. | Porosity [%]       | log10(Perm) [mD]              | Perm Anis [@f$ k_h/k_v @f$] | Perm Recovery Ccoef. |
/// |:----------------:|:----------------:|:--------------:|:----------:|:------------------:|:-----------------------------:|:---------------------------:|:--------------------:|
/// | Std. Shale       | Mudstone         | 0.01           | 1.5        |                    |                               | 1.0                         | 0.01                 |
/// | Std. Sandstone   | Sandstone        | 6000           | 1.5        |                    |                               | 1.0                         |                      |
/// | SM.Mudst.40%Clay | Multipoint       |                |            | [5.0,60.0]         | [-6.0, -0.5]                  | 1.0                         |                      |
/// | SM.Mudst.50%Clay | Multipoint       |                |            | [5.0,60.0]         | [-6.1, -1.7]                  | 1.0                         |                      |
/// | SM.Mudst.60%Clay | Multipoint       |                |            | [5,20,30,40,50,60] | [-5.93,5.23,-4.67,-3.9,-3,-2] | 1.0                         |                      |
/// | SM.Sandstone     | Multipoint       |                |            | [5.0,60.0]         | [0.3,8.6]                     | 1.0                         |                      |
///
///
/// @section PermeabilityModelPrmInpermeableSec None/Impermeable models means tiny constant permeability
/// <b>None/Impermeable</b> permeability models define a constant tiny permeability and have no any parameters.
/// Permeability value is set to @f$ 1.0^{-9} @f$ mD
///
/// @section PermeabilityModelPrmSandstoneSec Permeability model for sandstone
/// This model is used for sandstones, however in general sandstone permeabilities have little importance in basin modeling as water 
/// flow and hydrocarbon capillary sealing are dominated by mud-rock permeabilities which are many orders of magnitude lower than 
/// sandstone permeabilities. The sandstone permeability @f$ (k_{sand}) @f$ is defined as a function of porosity @f$ (\phi) @f$, 
/// depositional porosity @f$ (\phi) @f$, depositional permeability @f$ (k_0) @f$ and a coefficient @f$ (C)@f$. In the sandstone 
/// model this permeability coefficient is the clay % of the sand.
///
/// @f[ k_{sand}=k_{0}\cdot10^{(\phi-\phi_{0})\cdot(0.12+0.02\cdot C)} @f]
///
/// The porosity-permeability trend for the default Cauldron sandstone (Std.Sandstone) is shown on Figure @ref perm_vs_por
/// Cauldron has a maximum permeability of 1000 mDarcy.
///
/// The <b>Sandstone</b> permeability model parameters are following:
///  -# @f$ k_{0} @f$ - depositional permeability [mD]
///  -# permeability anisotropy [kh/kv] which is used to scale lateral permeability
///  -# @f$ C @f$ clay percentage of the sand [%]
///
/// @section PermeabilityModelPrmMudstoneSec Permeability model for mudstone
/// The @b Mudstone permeability model in Cauldron is rather an oddity as it relates permeability to stress rather than porosity. 
/// @f[ k_{shale}=k_{0}\cdot\left(\frac{VES+\sigma_{ref}}{\sigma_{ref}}\right)^{-C_{sensitivity}} @f]
/// The default shale permeability for the Std.Shale is shown in Figure \ref perm_vs_por. As can be seen from the equation above 
/// the permeability is actually a function of VES. However, it is plotted in Figure perm_vs_por against porosity so that it can 
/// be compared with permeability trends from other lithologies. This permeability model can be extended to allow increasing 
/// permeability during episodes of reducing stress: 
/// @f[ k_{shale}=k_{0}\cdot\left(\left[\frac{VES+\sigma_{ref}}{\sigma_{ref}}\right]^{-C_{sensitivity}}\cdot
///       \left[\frac{VES+\sigma_{ref}}{MaxVES+\sigma_{ref}}\right]^{-C_{recovery}}\right)
/// @f]
/// Here, the permeability change on unloading, set by the permeability recovery coefficient, is much lower than the permeability 
/// change during loading which is set by the permeability sensitivity coefficient (Table 1).
///
/// The model has 4 parameters
///  -# @f$ k_{0} @f$ - depositional permeability [mD]
///  -# permeability anisotropy [kh/kv] which is used to scale lateral permeability
///  -# @f$ C_{sensitivity} @f$ permeability sensitivity coefficient []
///  -# @f$ C_{recovery} @f$ permeability recovery coefficient []
///
/// @section MultiPointPermModelSec Permeability model is described by a Permeability vs Porosity profile
/// in <b>Multipoint</b> permeability model is a simple table of porosity and @f$ (log10) @f$ permeability values. This allows the user to 
/// input any porosity-permeability relationship. Cauldron contains default permeability trends for the 40, 50 and 60 [%] clay fraction shales.
/// These are shown in Figure @ref perm_vs_por
///
/// @image html MultipointPerm.png "Figure 2. Example of tabulated representation of permeability/porosity relation"
///
/// The model has 3 parameters
///  -# permeability anisotropy [kh/kv] which is used to scale lateral permeability
///  -# number of Porosity [%] / Permeability [log10(mD)] points in table 
///  -# set of Porosity [%] /Permeability [log10(mD)] values as an array of pairs 

namespace casa
{
   class VarPrmPermeabilityModel;

   /// @brief Source rock initial organic content parameter
   class PrmPermeabilityModel : public Parameter
   {
   public:
      /// @brief Permeability model type for lithology
      typedef enum
      {
         Sandstone   = mbapi::LithologyManager::PermSandstone,   ///< permeability model for sandstones
         Mudstone    = mbapi::LithologyManager::PermMudstone,    ///< permeability model for shales
         None        = mbapi::LithologyManager::PermNone,        ///< non permeable layer, layer with tiny permeability ~1e-9
         Impermeable = mbapi::LithologyManager::PermImpermeable, ///< mostly the same as None
         Multipoint  = mbapi::LithologyManager::PermMultipoint,  ///< permeability depends on porosity as 1D function
         Unknown     = mbapi::LithologyManager::PermUnknown      ///< Not any model was defined
      } PermeabilityModelType;
      
      /// @brief Defines order of parameters in parameters array for all permeability models
      typedef enum
      {
         AnisotropicCoeff   = 0,  ///< Sandstone, Mudstone, Multipoint models parameter

         DepositionalPerm   = 1,  ///< Sandstone, Mudstone mdoels parameter
         MPProfileNumPoints = 1,  ///< Multipoint model parameter

         ClayPercentage     = 2,  ///< Snadstone model parameter
         SensitivityCoeff   = 2,  ///< Mudstone model parameter

         RecoverCoeff       = 3   ///< Mudstone model parameter
      } PermeabilityModelParametersOrder;
 

      /// @brief Constructor. Create parameter by reading parameter value from the given model
      /// @param mdl Cauldron model interface object to get Permeability model and it parameters value 
      ///            for the given lithology. 
      /// @param lithoName lithology name
      PrmPermeabilityModel( mbapi::Model & mdl, const char * lithoName );

      /// @brief Constructor. Create parameter from values set     
      PrmPermeabilityModel( const VarPrmPermeabilityModel * parent    ///< pointer to a variable parameter which created this one
                          , const char                    * lithoName ///< lithoName lithology name
                          , PermeabilityModelType           mdlType   ///< type of permeability model
                          , const std::vector<double>     & mdlPrms   ///< list of model parameters, depends on the model type
                          );

      /// @brief Destructor
      virtual ~PrmPermeabilityModel() { ; }
     
      /// @brief Get name of the parameter
      /// @return parameter name
      virtual const char * name() const { return m_name.c_str(); }

      /// @brief Get variable parameter which was used to create this parameter
      /// @return Pointer to the variable parameter
      virtual const VarParameter * parent() const { return m_parent; }

      /// @brief Get the level of influence to cauldron applications pipeline for this parametr
      /// @return number which indicates which solver influence this parameter
      virtual AppPipelineLevel appSolverDependencyLevel() const  { return PTSolver; }                

      /// @brief Set this parameter value in Cauldron model
      /// @param caldModel reference to Cauldron model
      /// @param caseID unique RunCase ID, in some parameters it is used in new map file name generation
      /// @return ErrorHandler::NoError in success, or error code otherwise     
      virtual ErrorHandler::ReturnCode setInModel( mbapi::Model & caldModel, size_t caseID );

      /// @brief Validate Permeability model parameter values
      /// @param caldModel reference to Cauldron model
      /// @return empty string on success or error message with current parameter value
      virtual std::string validate( mbapi::Model & caldModel );

      // The following methods are used for converting between CASA RunCase and SUMLib::Case objects
      
      /// @brief Get parameter value as an array of doubles
      /// @return parameter value represented as set of doubles
      virtual std::vector<double> asDoubleArray() const;

      /// @brief Get parameter value as integer
      /// @return parameter value represented as integer
      virtual int asInteger() const { return static_cast<int>( m_modelType ); }

      /// @brief Are two parameters equal?
      /// @param prm Parameter object to compare with
      /// @return true if parameters are the same, false otherwise
      virtual bool operator == ( const Parameter & prm ) const;

      /// @brief Define the current Porosity/Permeability profile of Multipoint model position in variation between min/max profiles
      /// Here is some trick is used to avoid passing the whole curve to SUMlib for DoE generation
      /// @param val variation value
      void setVariationPosition( double val ) { m_mpProfilePos = val; }

      /// @brief For Multipoint model, get porosity values set
      /// @return array of porosity values
      const std::vector<double> & multipointPorosity()     const { return m_mpPorosity; }

      /// @brief For Multipoint model, get permeability values set
      /// @return array of permeability values
      const std::vector<double> & multipointPermeability() const { return m_mpPermeab; }

      /// @{
      /// @brief Defines version of serialized object representation. Must be updated on each change in save()
      /// @return Actual version of serialized object representation
      virtual unsigned int version() const { return 0; }

      /// @brief Save all object data to the given stream, that object could be later reconstructed from saved data
      /// @param sz Serializer stream
      /// @param  version stream version
      /// @return true if it succeeds, false if it fails.
      virtual bool save( CasaSerializer & sz, unsigned int version ) const;

      /// @brief Get type name of the serialaizable object, used in deserialization to create object with correct type
      /// @return object class name
      virtual const char * typeName() const { return "PrmPermeabilityModel"; }

      /// @brief Create a new parameter instance by deserializing it from the given stream
      /// @param dz input stream
      /// @param objVer version of object representation in stream
      PrmPermeabilityModel( CasaDeserializer & dz, unsigned int objVer );
      /// @}

   protected:
      const VarParameter  * m_parent;        ///< variable parameter which was used to create this one

      std::string           m_name;          ///< name of the parameter
      
      PermeabilityModelType m_modelType;     ///< type of the permeability model
      std::string           m_lithoName;     ///< lithology name
      
      // All model parameters
      double                m_anisotCoeff;   ///< anisotropy coefficient

      // Sandstone/Mudstsone models parameters
      double                m_depoPerm;      ///< depositional permeability

      // Sandstone model parameters
      double                m_clayPercentage;///< clay fraction which unique defines surface permeability and compaction coefficient
   
      // Mustone model parameters
      double                m_sensitCoeff;   ///< sensitivity coefficient
      double                m_recoveryCoeff; ///< recovery coefficient

      // Multipoint model parameters
      double               m_mpProfilePos;   ///< value in range [0:1], which is used to variate 1D profile
      std::vector<double>  m_mpPorosity;     ///< list of values for porosity
      std::vector<double>  m_mpPermeab;      ///< list of vector for permeability
   };
}

#endif // CASA_API_PARAMETER_PERMEABILITY_MODEL_H
