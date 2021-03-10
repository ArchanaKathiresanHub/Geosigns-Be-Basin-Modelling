//                                                                      
// Copyright (C) 2015-2016 Shell International Exploration & Production.
// All rights reserved.
// 
// Developed under license for Shell by PDS BV.
// 
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#ifndef _FASTCAULDRON__HISTORY_H_
#define _FASTCAULDRON__HISTORY_H_

#include "propinterface.h"
#include "ConstantsFastcauldron.h"

#include <sstream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <map>
#include <string>
using namespace std;


typedef set< double, less<double> > Double_Container;

typedef vector< double > Double_Vector;

typedef map<string, Double_Vector*, less<string> > HistoryProperties;

struct Node_Info {
   ~Node_Info();
   double X_Coord;
   double Y_Coord;
   int    I_Index;
   int    I_Step;
   int    J_Index;
   int    J_Step;
   int    K_Index;
   double Fractions [4];
   HistoryProperties Properties;
};

typedef vector< Node_Info* > Node_Container;

struct Surface_Info {

   ~Surface_Info();
   string           Name;
   int              Layer_Index;
   Node_Container   Nodes;
 
};

typedef map<double, Surface_Info*, less<double> > SurfaceManager;

class History
{
public:

   History( AppCtx *appl );
   ~History();

   Double_Container Time_Stamps;

   bool IsDefined ();
   void Add_Time ( const double time );
   void Read_Spec_File ();
   void Record_Properties ( Surface_Info* surface );
   void Output_Properties ();

   void clearProperties ();

   void Locate_Point ( const double X_Coord, const double Y_Coord, Node_Info * &node );
   void Save_Property ( const string&      property_name,
                        const enum::PropertyIdentifier propertyId,
                        LayerProps*        layer,
                        const int          K,
                        Node_Container&    nodes );
  
   void Save_Property_Value_At_Node ( Node_Info *    node,
                                      double         Property_Value,
                                      const string & Property_Name );

   double Calculate_Interpolated_Value( Node_Info *     node, 
                                        PETSC_3D_Array& property );

   void Save_Depositional_Property ( const string&      property_name,
                                     const enum::PropertyIdentifier propertyId,
                                     LayerProps*        layer,
                                     Node_Container&    nodes );

   double Interpolate_Depositional_Property( LayerProps * layer,
                                             Node_Info * node );

   double Calculate_Surface_Property( LayerProps *     layer,
                                      const int        I, 
                                      const int        J );

   void Generate_Log_File_Header ();

private:

   void createLogFileHeader ( ofstream& historyDataFile ) const;

   SurfaceManager::iterator findSurface ( const double age,
                                          const double relativeTolerance = DefaultAgeTolerance );

   Double_Container::iterator findTimeStamp ( const double age,
                                              const double relativeTolerance = DefaultAgeTolerance );


   AppCtx * appctx;

   double           current_time;
   static const int width = 20;

   bool             Has_Nodes;
   SurfaceManager   Surfaces;
   string           Log_File_Header;

   PropListVec Property_List;

   // Remove all the iterators.
   PropListVec::iterator property_it;

   Double_Container::iterator dble_it;
   Double_Container::reverse_iterator r_dble_it;

   HistoryProperties::iterator prop_vec_it;
   HistoryProperties::reverse_iterator r_prop_vec_it;

   Node_Container::iterator node_it;

   SurfaceManager::iterator surface_it;
   SurfaceManager::reverse_iterator r_surface_it;

   std::string calculationModeFileNameExtension;

};

#endif /* _FASTCAULDRON__HISTORY_H_ */
