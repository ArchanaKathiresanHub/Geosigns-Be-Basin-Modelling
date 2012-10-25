#ifndef _GENEXSIMULATION_LOCALGRIDINTERPOLATOR_H_
#define _GENEXSIMULATION_LOCALGRIDINTERPOLATOR_H_

namespace DataAccess
{
   namespace Interface
   {
      class Snapshot;
      class GridMap;
   }
}


namespace GenexSimulation
{

class LocalGridInterpolator
{
public:

   ///constructor
   LocalGridInterpolator();
   
   ///destructor
   virtual ~LocalGridInterpolator();
   
   void compute(const DataAccess::Interface::Snapshot  *start, 
                const DataAccess::Interface::GridMap   *dataAtStart, 
                const DataAccess::Interface::Snapshot  *end, 
                const DataAccess::Interface::GridMap   *dataAtEnd);

   double evaluateProperty(const int &I, const int &J, const double &in_timeInstance) const;

protected:
   enum BufferType{A, B};

   void setValueFromGridMap(const double &value, const int &GridI,const int &GridJ, BufferType theType);
   double getValueForGrid(const int &GridI, const int &GridJ, BufferType theType) const ;
   void initialize(const DataAccess::Interface::GridMap *theGrid);
   void clear();
   double **allocate2DArray(const int &rows, const int &columns);
   void delete2DArray(double **array, const int &rows);

private:
   double **m_A;
   double **m_B;
   int m_firstI;
   int m_lastI;
   int m_firstJ;
   int m_lastJ;
   int m_rows;
   int m_columns;

   LocalGridInterpolator(const LocalGridInterpolator &);
   LocalGridInterpolator &operator=(const LocalGridInterpolator &);
   
  
};

}

#endif

