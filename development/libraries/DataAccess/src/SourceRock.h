#ifndef _INTERFACE_SOURCEROCK_H_
#define _INTERFACE_SOURCEROCK_H_

using namespace std;

#include "DAObject.h"
#include "Interface.h"

namespace DataAccess
{
   namespace Interface
   {
      /// A SourceRock object contains information on a specific source rock
      class SourceRock : public DAObject
      {
   public:
      SourceRock (ProjectHandle& projectHandle, database::Record * record);
      virtual ~SourceRock (void);


			/// Return the layer name of this SourceRock
			virtual const string & getLayerName (void) const;
						/// Set a layer name.
						void setLayerName ( const std::string& name );
			/// Return the type of this SourceRock
			virtual const string & getType (void) const;
						/// Return the H/C of this SourceRock
						virtual const double & getHcVRe05(void) const;
						/// Return the S/C of this SourceRock
						virtual const double & getScVRe05(void) const;
						/// Return the activation energy of Preasphaltene
						virtual const double & getPreAsphaltStartAct(void) const;
						/// Return the VRE of this SourceRock
						/// Return the diffusion energy of Asphaltene
						virtual const double & getAsphalteneDiffusionEnergy(void) const;
						/// Return the diffusion energy of Resin
						virtual const double & getResinDiffusionEnergy(void) const;
						/// Return the diffusion energy of C15+ Aromatics
						virtual const double & getC15AroDiffusionEnergy(void) const;
						/// Return the diffusion energy of C15+ Sat
						virtual const double & getC15SatDiffusionEnergy(void) const;
						///Return true if the VREoptimization flag has been set to Yes
						virtual bool isVREoptimEnabled(void) const;
						///Return the value of the VRE threshold
						virtual const double & getVREthreshold(void) const;
						/// get predefined SourceRock Type, this SoureRock is derived from
						virtual const string & getBaseSourceRockType (void) const;


      ///Return true if the maximum VES value is enabled
            virtual bool isVESMaxEnabled(void) const;
	    ///Return the value of the maximum admissible VES value
	    virtual const double & getVESMax(void) const;
         
	    // shale gas related functions
	    /// Whether to perform adsorption
	    virtual bool doApplyAdsorption (void) const;
	    /// Whether to use a TOC-dependent adsorption capacity function
	    virtual bool adsorptionIsTOCDependent (void) const;
	    /// Whether to (also) perform OTGC during adsorption
	    virtual bool doComputeOTGC (void) const;
	    /// get the name of the adsorption function
	    virtual const string & getAdsorptionCapacityFunctionName (void) const;
	    /// get the adsorption simulator
	    virtual const string & getAdsorptionSimulatorName (void) const;

	    /// Return the (GridMap) value of one of this SourceRock's attributes
		virtual const GridMap * getMap (SourceRockMapAttributeId attributeId) const;

	    /// load a map
		GridMap * loadMap (SourceRockMapAttributeId attributeId) const;

	    /// Print the attributes of this SourceRock
            // May not work if user application is compiled under IRIX with CC -lang:std
        void printOn (ostream &) const;

      /// Print the attributes of this SourceRock
            // May not work if user application is compiled under IRIX with CC -lang:std
        void asString (string &) const;

	 private:
			static const string s_MapAttributeNames[];

						std::string m_layerName;

      };
   }
}


#endif // _INTERFACE_SOURCEROCK_H_
