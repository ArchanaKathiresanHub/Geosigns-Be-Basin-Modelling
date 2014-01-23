#include <vtkStructuredGridAlgorithm.h>
#include <vtkFloatArray.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkStructuredGrid.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>

#include <iostream>

#include "ParaViewReader.h"

vtkStandardNewMacro(ParaViewReader);

ParaViewReader
  :: ParaViewReader()
   : FileName(0)
{
  std::cout << "ParaViewReader object created" << std::endl;
  this->SetNumberOfInputPorts(0);
  this->SetNumberOfOutputPorts(1);
}

void ParaViewReader::PrintSelf(std::ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
   
    os << indent << "File Name: "
             << (this->FileName ? this->FileName : "(none)") << "\n";
}

int
ParaViewReader
  :: RequestInformation( vtkInformation * request, 
                          vtkInformationVector ** inputVector,
                          vtkInformationVector * outputVector)
  {
     std::cout << "Entered RequestInformation" << std::endl;
     
     request->PrintSelf(std::cout, vtkIndent(0));
     std::cout << std::endl;

     vtkInformation* outputInfo = outputVector->GetInformationObject(0);
     int extent[6] = { 0, 12, 0, 10, 0, 10};
 
 
     outputInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), extent, 6);

     std::cout << "Leaving RequestInformation" << std::endl;
     return 1;
  }

int
ParaViewReader
  :: RequestUpdateExtent( vtkInformation * request, 
                           vtkInformationVector ** inputVector,
                           vtkInformationVector * outputVector)
  {
     std::cout << "Entered RequestUpdateExtent" << std::endl;

     request->PrintSelf(std::cout, vtkIndent(0));
     std::cout << std::endl;


     std::cout << "Leaving RequestUpdateExtent" << std::endl;
     return 1;
  }


int
ParaViewReader
  :: RequestData( vtkInformation * request, 
                   vtkInformationVector ** inputVector,
                   vtkInformationVector * outputVector)
  {
     std::cout << "Entered RequestData" << std::endl;
     vtkInformation* outputInfo = outputVector->GetInformationObject(0);
     request->PrintSelf(std::cout, vtkIndent(0));
     std::cout << std::endl;

     std::cout << "Output Vector information" << std::endl;
     outputInfo->PrintSelf(std::cout, vtkIndent(0));
     std::cout << std::endl;

     vtkStructuredGrid* sgrid = vtkStructuredGrid::SafeDownCast
       (outputInfo->Get(vtkDataObject::DATA_OBJECT()));
    
     static int dims[3]={13,11,11};

     int extent[6] = { 0, -1, 0, -1, 0, -1};
     outputInfo->Get
       (vtkStreamingDemandDrivenPipeline::UPDATE_EXTENT(), extent);
    
     std::cout << "Setting extent" << std::endl;
     sgrid->SetExtent(extent);

      // We also create the points and vectors. The points
  // form a hemi-cylinder of data.
     int volume = (extent[1] - extent[0]+1)*(extent[3]-extent[2]+1)*(extent[5]-extent[4]+1);
     vtkFloatArray *vectors = vtkFloatArray::New();
       vectors->SetNumberOfComponents(3);
       vectors->SetNumberOfTuples(volume);
     vtkPoints *points = vtkPoints::New();
       points->Allocate(volume);

     float x[3], v[3], rMin=0.5, rMax=1.0;
     float deltaZ = 2.0 / (dims[2]-1);
     float deltaRad = (rMax-rMin) / (dims[1]-1);
     v[2]=0.0;
     for ( int k=extent[4]; k<=extent[5]; k++)
       {
         std::cout << "Computing k: " << k << std::endl;
       x[2] = -1.0 + k*deltaZ;
       int kOffset = k * (extent[1]-extent[0]+1)*(extent[3]-extent[2]+1);
       for (int j=extent[2]; j<=extent[3]; j++)
         {
         float radius = rMin + j*deltaRad;
         int jOffset = j * (extent[1]-extent[0]+1);
         for (int i=extent[0]; i<=extent[1]; i++)
           {
           float theta = i * vtkMath::RadiansFromDegrees(15.0);
           x[0] = radius * cos(theta);
           x[1] = radius * sin(theta);
           v[0] = -x[1];
           v[1] = x[0];
           int offset = i + jOffset + kOffset;
           points->InsertPoint(offset,x);
           vectors->InsertTuple(offset,v);
           }
         }
       }

     std::cout << "Setting points" << std::endl;
     sgrid->SetPoints(points);
     points->Delete();
     std::cout << "Setting vectors" << std::endl;
     sgrid->GetPointData()->SetVectors(vectors);
     vectors->Delete();

     std::cout << "Leaving RequestData" << std::endl;
     return 1;
  }


