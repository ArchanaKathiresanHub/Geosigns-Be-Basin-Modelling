#include <vector>

#include "DerivedProperty.h"

#include "Formation.h"
#include "Property.h"
#include "PropertyValue.h"

using namespace DataAccess;
using namespace Interface;

#include "voxetschemafuncs.h"
#include "VoxetProjectHandle.h"
#include "DerivedPropertyFormationFunction.h"
#include "VoxetDomainInterpolator.h"
#include "DerivedPropertyFunction.h"
#include "GridDescription.h"

extern bool debug;

// Detault value is in feet/sec
const float DerivedProperty::DefaultWaterVelocity = 1500.0 * 3.281;


DerivedProperty::DerivedProperty ( Interface::ProjectHandle* cauldronProjectHandle,
                                   VoxetProjectHandle*            voxetHandle,
                                   database::Record*              record ) : 

   m_cauldronProjectHandle ( cauldronProjectHandle ),
   m_voxetHandle ( voxetHandle ),
   m_record ( record )
{
}

const std::string& DerivedProperty::getName () const {
   return database::getDerivedPropertyName ( m_record );
}

const std::string& DerivedProperty::getUnits () const {
   return database::getDerivedPropertyUnits ( m_record );
}

void DerivedProperty::calculate ( VoxetCalculator&   interpolator,
                                  const Snapshot*    snapshot,
                                  VoxetPropertyGrid& values,
                                  const bool         verbose )
{
   if (verbose)
   {
      cout << " interpolating derived property   : " << getName () << "  " << flush;
   }

   DerivedPropertyFormationFunctionList *propertyFormationFunctions = m_voxetHandle->getDerivedPropertyFormationFunctions (this);
   DerivedPropertyFormationFunction* waterColumnFormationFunction = m_voxetHandle->getWaterColumnDerivedPropertyFormationFunction (this);
   DerivedPropertyFunction* waterColumnFunction = 0;

   // Water column functions have no parameters.
   std::vector<Parameter *> waterColumnParameters ( 1 );
   std::vector<float> waterColumnParameterValues ( 1, 0.0 );

   DerivedPropertyFormationFunctionList::iterator propertyFormationFunctionIter;

   VoxetDomainInterpolator & anyInterpolator = interpolator.getAnyVoxetDomainInterpolator ();

   /// Get the null value.
   const float NullValue = interpolator.getNullValue ();

   FormationList *cauldronFormations = m_cauldronProjectHandle->getFormations (snapshot);

   const GridDescription & gridDescription = m_voxetHandle->getGridDescription ();

   int *functionNumbers = new int[cauldronFormations->size ()];
   DerivedPropertyFunction **propertyFunctions = new DerivedPropertyFunction *[cauldronFormations->size ()];
   std::vector<Parameter *> * * propertyFunctionCallParametersArray = new std::vector<Parameter *> * [cauldronFormations->size ()];

   int i;
   int j;
   int k;
   int formationNumber;
   float z;

   // For outputting a . for every 5% completion, in verbose mode.
   int outputDotNumber = gridDescription.getVoxetNodeCount () / 20;
   int count;

   if ( waterColumnFormationFunction != 0 ) {
      waterColumnParameters [ 0 ] = 0;

      if ( waterColumnFormationFunction->getFunctionCallParameters ().size () > 1 or 
           ( waterColumnFormationFunction->getFunctionCallParameters ().size () == 1 and waterColumnFormationFunction->getFunctionCallParameters ()[ 0 ]->getProperty ()->getName () != "Depth" )) {
         cerr << endl << endl << "Error in formation function " << waterColumnFormationFunction->getFunctionName () << endl;
         cerr << "Any water-column function can only have no parameters or the property Depth as its sole parameter. " << endl;
         exit ( -1 );
      }

      waterColumnFunction = m_voxetHandle->getDerivedPropertyFunction ( waterColumnFormationFunction->getFunctionName ());
   }

   for (i = 0; i < cauldronFormations->size (); ++i)
   {
      functionNumbers[i] = -1;
      propertyFunctions[i] = 0;
   }

   // map the formation to the derived-property-function.
   for (i = 0; i < cauldronFormations->size (); ++i)
   {
      for (propertyFormationFunctionIter = propertyFormationFunctions->begin (); propertyFormationFunctionIter != propertyFormationFunctions->end (); ++propertyFormationFunctionIter)
      {
         if ((*cauldronFormations)[i]->getName () == (*propertyFormationFunctionIter)->getFormationName ())
         {
	    if (debug)
	    {
	       cerr << "Assigning property function " << (*propertyFormationFunctionIter)->getFunctionName () << " to formation " << (*propertyFormationFunctionIter)->
		  getFormationName () << " (" << i << ")" << endl;
	    }

            propertyFunctions[i] = m_voxetHandle->getDerivedPropertyFunction ((*propertyFormationFunctionIter)->getFunctionName ());
            assert (propertyFunctions[i] != 0);
	    propertyFunctionCallParametersArray[i] = & (*propertyFormationFunctionIter)->getFunctionCallParameters ();
         }
      }
   }

   // Compute the derived-property.
   for (i = 0, count = 0; i < gridDescription.getVoxetNodeCount (0); ++i)
   {

      for (j = 0; j < gridDescription.getVoxetNodeCount (1); ++j)
      {
         for (k = 0, z = gridDescription.getVoxetGridOrigin (2);
              k < gridDescription.getVoxetNodeCount (2); ++k, z += gridDescription.getVoxetGridDelta (2), ++count)
         {
            // Could have equally used the bulk-density interpolator.
            formationNumber = anyInterpolator(i,j).findLayer (z);


            if (formationNumber == -1)
            {

               if ( anyInterpolator ( i, j ).isPartOfWaterColumn ( z ) and waterColumnFunction != 0 ) {
                  waterColumnParameterValues [ 0 ] = z;
                  values (i, j, k) = waterColumnFunction->calculate ( waterColumnParameters, waterColumnParameterValues );
               } else {
                  values (i, j, k) = NullValue;
               }

            }
            else
            {
	       if (debug) cout << formationNumber << ":  ";

               DerivedPropertyFunction *propertyFunctionFound = propertyFunctions[formationNumber];
               assert (propertyFunctionFound);
	       
	       std::vector<Parameter *> & functionCallParameters = * propertyFunctionCallParametersArray[formationNumber];
	       std::vector<Parameter *>::iterator parameterIterator;

	       std::vector<float> parameterValues;
	       for (parameterIterator = functionCallParameters.begin (); parameterIterator != functionCallParameters.end (); ++parameterIterator)
	       {
		  if ((*parameterIterator)->isConstant ())
		  {
		     parameterValues.push_back ((*parameterIterator)->getConstant ());
		  }
		  else
		  {
		     VoxetDomainInterpolator & voxetDomainInterpolator = interpolator.getVoxetDomainInterpolator ((* parameterIterator)->getProperty ());
		     const DepthInterpolator & depthInterpolator = voxetDomainInterpolator (i, j);
		     parameterValues.push_back (depthInterpolator (z));
		  }
	       }

               values (i, j, k) = propertyFunctionFound->calculate (functionCallParameters, parameterValues);
            }

            if (verbose and count == outputDotNumber)
            {
               cout << "." << flush;
               count = 0;
            }
         }
      }
   }

   if (verbose)
   {
      cout << " done." << endl;
   }

   delete[]propertyFunctions;
   delete[]propertyFunctionCallParametersArray;
   delete[]functionNumbers;
   delete cauldronFormations;
   delete propertyFormationFunctions;
}
