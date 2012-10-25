#include "SCmathlib.h"
#ifndef M_PI
#define M_PI	3.143
#endif
#include <iomanip>

namespace Genex5
{

SCPoint::SCPoint(int dim){
  dimension = dim;
  data = new double[dimension];

  for(int i=0;i<dimension;i++)
    data[i] = 0.0;
}


SCPoint::SCPoint(const SCPoint &v){
  dimension = v.Dimension();
  data = new double[dimension];

  for(int i=0;i<dimension;i++)
    data[i] = v.data[i];
}


SCPoint::~SCPoint(){
  dimension = 0;
  delete[] data;
  data = NULL;
}


int SCPoint::Dimension() const{
  return(dimension);
}


double SCPoint::operator()(const int i) const{
  if(i>=0 && i<dimension)
    return data[i];

  cerr << "SCPoint::Invalid index " << i << " for SCPoint of dimension " << dimension << endl;
  return(0);
}



double& SCPoint::operator()(const int i){
  if(i>=0 && i<dimension)
    return data[i];

  cerr << "SCPoint::Invalid index " << i << " for SCPoint of dimension " << dimension << endl;
  return(data[0]);
}


SCPoint& SCPoint::operator=(const SCPoint &v) {
  dimension = v.Dimension();
  for(int i=0;i<dimension;i++)
    data[i] = v.data[i];
  return *this;
};

void SCPoint::Print() const
{
  cout << endl;
  cout << "[ ";
  if(dimension>0)
    cout << data[0];
  for(int i=1;i<dimension;i++)
    cout << "; " << data[i];
  cout << " ]" << endl;
}

SCVector::SCVector(){
  dimension = 0;
  data = NULL;
}


SCVector::SCVector(int dim){
  dimension = dim;
  data = new double[dimension];

  for(int i=0;i<dimension;i++)
    data[i] = 0.0;
}


SCVector::SCVector(const SCVector &v){
  dimension = v.Dimension();
  data = new double[dimension];

  for(int i=0;i<dimension;i++)
    data[i] = v.data[i];
}


SCVector::SCVector(int col, const SCMatrix &A){
  dimension = A.Rows();

  data = new double[dimension];
  
  for(int i=0;i<A.Rows();i++)
    data[i] = A(i,col);

}


SCVector::~SCVector(){
  dimension = 0;
  delete[] data;
  data = NULL;
}


void SCVector::Initialize(int dim){
  if(dimension!=0)
    delete[] data;

  dimension = dim;
  data = new double[dimension];
  
  for(int i=0;i<dimension;i++)
    data[i] = 0.0;
}


int SCVector::Dimension() const{
  return(dimension);
}


double SCVector::operator()(const int i) const{
  if(i>=0 && i<dimension)
    return data[i];

  cerr << "SCVector::Invalid index " << i << " for SCVector of dimension " << dimension << endl;
  return(0);
}



double& SCVector::operator()(const int i){
  if(i>=0 && i<dimension)
    return data[i];

  cerr << "SCVector::Invalid index " << i << " for SCVector of dimension " << dimension << endl;
  return(data[0]);
}


SCVector& SCVector::operator=(const SCVector &v) {
  dimension = v.Dimension();
  for(int i=0;i<dimension;i++)
    data[i] = v.data[i];
  return *this;
};

void SCVector::Print() const{
   
  cout << endl;
  cout << "[ ";
  if(dimension>0)
    cout << data[0];
  for(int i=1;i<dimension;i++)
    cout << "; " << data[i];
  cout << " ]" << endl;

}


double SCVector::Norm_l1(){
  double sum = 0.0;
  for(int i=0;i<dimension;i++)
    sum += fabs(data[i]);
  return(sum);
}


double SCVector::Norm_l2(){
  double sum = 0.0;
  for(int i=0;i<dimension;i++)
    sum += data[i]*data[i];
  return(sqrt(sum));
}

void SCVector::Normalize(){
  double tmp = 1.0/Norm_l2();
  for(int i=0;i<dimension;i++)
    data[i] = data[i]*tmp;
}


double SCVector::Norm_linf(){
  double maxval = 0.0,tmp;
  
  for(int i=0;i<dimension;i++){
    tmp = fabs(data[i]);
    maxval = (maxval > tmp)?maxval:tmp;
  }
  return(maxval);
}

double SCVector::MaxMod(){
  double maxm = -1.0e+10;

  for(int i=0; i<dimension; i++)
    maxm = (maxm > fabs(data[i]))?maxm:fabs(data[i]);
  
  return maxm;
}

double SCVector::ElementofMaxMod(){
  return(data[MaxModindex()]);
}


int SCVector::MaxModindex(){
  double maxm = -1.0e+10;
  int maxmindex = 0;

  for(int i=0; i<dimension; i++){
    if(maxm<fabs(data[i])){
      maxm = fabs(data[i]);
      maxmindex = i;
    }
  }
  
  return maxmindex;
}

void SCVector::Initialize(double a){
  for(int i=0; i<dimension; i++)
    data[i] = a;
}

void SCVector::Initialize(double *v){
  for(int i=0; i<dimension; i++)
    data[i] = v[i];
}

SCMatrix::SCMatrix(int dim){
  rows = dim;
  columns = dim;
  data = new double*[rows];
  for(int i=0;i<rows;i++){
    data[i] = new double[columns];
    for(int j=0;j<columns;j++)
      data[i][j] = 0.0;
  }
}

  
SCMatrix::SCMatrix(int rows1, int columns1){
  rows = rows1;
  columns = columns1;

  data = new double*[rows];
  for(int i=0;i<rows;i++){
    data[i] = new double[columns];
    for(int j=0;j<columns;j++)
      data[i][j] = 0.0;
  }
}

SCMatrix::SCMatrix(const SCMatrix& m){
  rows = m.rows;
  columns = m.columns;

  data = new double*[rows];

  for(int i=0;i<rows;i++){
    data[i] = new double[columns];
    for(int j=0; j<columns; j++)
      data[i][j] = m.data[i][j];
  }
}

SCMatrix::SCMatrix(int num_SCVectors, const SCVector * q){
  rows = q[0].Dimension();
  columns = num_SCVectors;

  data = new double*[rows];

  for(int i=0;i<rows;i++){
    data[i] = new double[columns];
    for(int j=0; j<columns; j++)
      data[i][j] = q[j](i);
  }
}

SCMatrix::SCMatrix(int rows1, int columns1, double **rowptrs){
  rows = rows1;
  columns = columns1;

  data = new double*[rows];

  for(int i=0;i<rows;i++)
    data[i] = rowptrs[i];
}


SCMatrix::~SCMatrix(){
  for(int i=0;i<rows;i++)
    delete[] data[i];

  rows = 0;
  columns = 0;
  delete[] data;
}

int SCMatrix::Rows() const{
  return(rows);
}  

int SCMatrix::Columns() const{
  return(columns);
}  


double **SCMatrix::GetPointer(){
  return(data);
}

void SCMatrix::GetColumn(int col, SCVector &x){
  x.Initialize(0.0);
  for(int i=0;i<rows;i++)
    x(i) = data[i][col];
}

void SCMatrix::GetColumn(int col, SCVector &x, int rowoffset){
  x.Initialize(0.0);
  for(int i=0;i<rows-rowoffset;i++)
    x(i) = data[i+rowoffset][col];
}

void SCMatrix::PutColumn(int col, const SCVector &x){
  for(int i=0;i<rows;i++)
    data[i][col] = x(i);
}


double SCMatrix::Norm_linf(){
  double maxval = 0.0,sum;
  
  for(int i=0;i<rows;i++){
    sum = 0.0;
    for(int j=0;j<columns;j++)
      sum += fabs(data[i][j]);
    maxval = (maxval > sum)?maxval:sum;
  }
  return(maxval);
}


double SCMatrix::Norm_l1(){
  double maxval = 0.0,sum;

  for(int j=0;j<columns;j++){
    sum = 0.0;
    for(int i=0;i<rows;i++)
      sum += fabs(data[i][j]);
    maxval = (maxval > sum)?maxval:sum;
  }
  return(maxval);
}



SCMatrix& SCMatrix::operator=(const SCMatrix &m){
  if( (rows == m.rows) && (columns == m.columns)){
    for(int i=0; i<rows; i++)
      for(int j=0;j<columns;j++){
	data[i][j] = m.data[i][j];
      }
  }
  else
    cerr << "SCMatrix Error: Cannot equate matrices of different sizes\n";
  return *this;
}

  
double SCMatrix::operator()(const int i, const int j) const {
  if( (i>=0) && (j>=0) && (i<rows) && (j<columns))
    return(data[i][j]);  
  else
    cerr << "SCMatrix Error: Invalid SCMatrix indices (" << i << "," << j << 
      "), for SCMatrix of size " << rows << " X " << columns << endl;
  return((double)0);
}
  

double& SCMatrix::operator()(const int i, const int j) {
  if( (i>=0) && (j>=0) && (i<rows) && (j<columns))
    return(data[i][j]);  
  else
    cerr << "SCMatrix Error: Invalid SCMatrix indices (" << i << "," << j << 
      "), for SCMatrix of size " << rows << " X " << columns << endl;;
  return(data[0][0]);
}


void SCMatrix::Print() const{
  cout << endl;


  cout << "[ ";
  for(int i=0;i<rows;i++){
    cout << data[i][0];
    for(int j=1;j<columns;j++)
      cout << " " << data[i][j];
    if(i!=(rows-1))
      cout << ";\n";
  }
  cout << " ]" << endl;
}


double SCMatrix::MaxModInRow(int row){
  double maxv = -1.0e+10;
  for(int i=0;i<columns;i++)
    maxv = (fabs(data[row][i])>maxv)?fabs(data[row][i]):maxv;

  return maxv;
}

double SCMatrix::MaxModInRow(int row, int starting_column){
  double maxv = -1.0e+10;
  for(int i=starting_column;i<columns;i++)
    maxv = (fabs(data[row][i])>maxv)?fabs(data[row][i]):maxv;

  return maxv;
}

int SCMatrix::MaxModInRowindex(int row){
  int maxvindex = 0;
  double maxv = -1.0e+10;
  
  for(int i=0;i<columns;i++){
    if(maxv < fabs(data[row][i])){
      maxv = fabs(data[row][i]);
      maxvindex = i;
    }
  }

  return maxvindex;
}

int SCMatrix::MaxModInRowindex(int row, int starting_column){
  int maxvindex = 0;
  double maxv = -1.0e+10;

  for(int i=starting_column;i<columns;i++){
    if(maxv < fabs(data[row][i])){
      maxv = fabs(data[row][i]);
      maxvindex = i;
    }
  }
  
  return maxvindex;
}

double SCMatrix::MaxModInColumn(int column){
  double maxv = -1.0e+10;
  for(int i=0;i<rows;i++)
    maxv = (fabs(data[i][column])>maxv)?fabs(data[i][column]):maxv;

  return maxv;
}

double SCMatrix::MaxModInColumn(int column, int starting_row){
  double maxv = -1.0e+10;
  for(int i=starting_row;i<rows;i++)
    maxv = (fabs(data[i][column])>maxv)?fabs(data[i][column]):maxv;

  return maxv;
}

int SCMatrix::MaxModInColumnindex(int column){
  int maxvindex = 0;
  double maxv = -1.0e+10;
  
  for(int i=0;i<rows;i++){
    if(maxv < fabs(data[i][column])){
      maxv = fabs(data[i][column]);
      maxvindex = i;
    }
  }

  return maxvindex;
}

int SCMatrix::MaxModInColumnindex(int column, int starting_column){
  int maxvindex = 0;
  double maxv = -1.0e+10;

  for(int i=starting_column;i<rows;i++){
    if(maxv < fabs(data[i][column])){
      maxv = fabs(data[i][column]);
      maxvindex = i;
    }
  }
  
  return maxvindex;
}

void SCMatrix::RowSwap(int row1, int row2){
  double * tmp = data[row1];
  data[row1] = data[row2];
  data[row2] = tmp;
}



/****************************************************************/
/*                 Operator Definitions                         */
/****************************************************************/


SCVector operator-(const SCVector& v)
{
   SCVector x(v.Dimension());
   for(int i=0;i<v.Dimension();i++)
   {
      x(i) = -v(i);
   }
   return x;
}
SCVector operator+(const SCVector& v1, const SCVector& v2)
{
   int min_dim = min_dimension(v1,v2);
   SCVector x(min_dim);
   for(int i=0;i<min_dim;i++)
   {
      x(i) = v1(i) + v2(i);
   }
   return x;
}
SCVector operator-(const SCVector& v1, const SCVector& v2)
{
   int min_dim = min_dimension(v1,v2);
   SCVector x(min_dim);
   for(int i=0;i<min_dim;i++)
   {
      x(i) = v1(i) - v2(i);
   }
   return x;
}
SCVector operator/(const SCVector& v, const double s) 
{
   SCVector x(v.Dimension());
   for(int i=0;i<v.Dimension();i++)
   {
      x(i) = v(i)/s;
   }
   return x;
}
SCVector operator*(const double s, const SCVector &v) 
{
   SCVector x(v.Dimension());
   for(int i=0;i<v.Dimension();i++)
   {
      x(i) = s*v(i);
   } 
   return x;
}
SCVector operator*(const SCVector& v, const double s) 
{
   SCVector x(v.Dimension());
   for(int i=0;i<v.Dimension();i++)
   {
      x(i) = s*v(i);
   }   
return x;
}

SCVector operator*(const SCMatrix& A, const SCVector& x)
{
   int rows = A.Rows(), columns = A.Columns();
   int dim = x.Dimension();
   SCVector b(dim);

   if(columns != dim)
   {
      cerr << "Invalid dimensions given in matrix-vector multiply" << endl;
      return(b);
   }
   for(int i=0;i<rows;i++)
   {
      b(i) = 0.0;
      for(int j=0;j<columns;j++)
      {
         b(i) += A(i,j)*x(j);
      }
   }
   return b;
}


/****************************************************************/
/*                 Function Definitions                         */
/****************************************************************/

int min_dimension(const SCVector& v1, const SCVector& v2)
{
   int min_dim = (v1.Dimension()<v2.Dimension())?v1.Dimension():v2.Dimension();
   return(min_dim);
}


double dot(const SCVector& u, const SCVector& v)
{
   double sum = 0.0;
   int min_dim;
   min_dim = min_dimension(u,v);

   for(int i=0;i<min_dim;i++)
   {
      sum += u(i)*v(i);
   } 
   return sum; 
}


double dot(int N, const SCVector& u, const SCVector& v)
{
   double sum = 0.0;
   for(int i=0;i<N;i++)
   {
      sum += u(i)*v(i);
   }
   return sum;
}


double dot(int N, double *a, double *b)
{
   double sum = 0.0;
   for(int i=0;i<N;i++)
   {
      sum += a[i]*b[i];
   }
   return sum;
}


/*******************************/
/*   Log base 2 of a number    */
/*******************************/

double log2(double x)
{
   return(log(x)/log(2.0));
}

void Swap(double &a, double &b)
{
   double tmp = a;
   a = b;
   b = tmp;
}
double Sign(double x)
{
   double xs;
   xs = (x>=0.0)?1.0:-1.0;
   return xs;
}

//GammaF function valid for x integer, or x (integer+0.5)
double GammaF(double x)
{
   double gamma = 1.0;

   if (x == -0.5)
   { 
      gamma = -2.0*sqrt(M_PI);
   }
   else if (!x) 
   {
      return gamma;
   }
   else if ((x-(int)x) == 0.5)
   { 
      int n = (int) x;
      double tmp = x;
      gamma = sqrt(M_PI);
      while(n--)
      {
         tmp   -= 1.0;
         gamma *= tmp;
      }
   }
   else if ((x-(int)x) == 0.0)
   {
      int n = (int) x;
      double tmp = x;
      while(--n)
      {
         tmp   -= 1.0;
         gamma *= tmp;
      }
   }  
  return gamma;
}
int Factorial(int n)
{
   int value=1;
   for(int i=n;i>0;i--)
   {
      value = value*i;
   }
   return value;
}

double ** CreateMatrix(int m, int n)
{
   double ** mat;
   mat = new double*[m];
   for(int i=0;i<m;i++)
   {
      mat[i] = new double[n];
      for(int j=0;j<m;j++)
      {
         mat[i][j] = 0.0;
      }
   }
   return mat;
}

int ** ICreateMatrix(int m, int n)
{
   int ** mat;
   mat = new int*[m];
   for(int i=0;i<m;i++)
   {
      mat[i] = new int[n];
      for(int j=0;j<m;j++)
      {
         mat[i][j] = 0;
      }
   }
   return mat;
}
 /*
void DestroyMatrix(double ** mat, int m, int n)
{
   for(int i=0;i<m;i++)
   {
      delete[] mat[i];
   }
   delete[] mat;
}
*/
void DestroyMatrix(double ** mat, int rows)
{
   for(int i=0;i<rows;i++)
   {
      delete[] mat[i];
   }
   delete[] mat;
}
 /*
void IDestroyMatrix(int ** mat, int m, int n)
{
   for(int i=0;i<m;i++)
   {
      delete[] mat[i];
   }
  delete[] mat;
}
    */
void IDestroyMatrix(int ** mat, int rows)
{
   for(int i=0;i<rows;i++)
   {
      delete[] mat[i];
   }
  delete[] mat;
}

void ModifiedArnoldi(int m, const SCMatrix &A, SCMatrix &H, SCMatrix &V)
{
   SCVector v(A.Rows()),w(A.Rows());
   v.Initialize(0.0);
   v(0) = 1.0;

   V.PutColumn(0,v);

   for(int j=0;j<m;j++)
   {
      w = A*v;
      for(int i=0;i<=j;i++)
      {
         V.GetColumn(i,v);
         H(i,j) = dot(w,v);
         w = w - H(i,j)*v;
      }
      H(j+1,j) = w.Norm_l2();
      v = w/H(j+1,j);
      V.PutColumn(j+1,v);
   }
}
void ModifiedArnoldi(int m, const SCVector &x, const SCMatrix &A, SCMatrix &H,SCMatrix &V)
{
   SCVector v(A.Rows()),w(A.Rows());
   v = x;
   V.PutColumn(0,v);
   for(int j=0;j<m;j++)
   {
      w = A*v;
      for(int i=0;i<=j;i++)
      {
         V.GetColumn(i,v);
         H(i,j) = dot(w,v);
         w = w - H(i,j)*v;
      }
      H(j+1,j) = w.Norm_l2();
      v = w/H(j+1,j);
      V.PutColumn(j+1,v);
   }
}
void GMRES(int m, const SCMatrix &A, const SCVector &b, SCVector &x){
  int i,j,k,ll,nr;
  int N = A.Rows();
  SCMatrix H(m+1,m),V(N,m+1);
  SCVector w(N),r(N),y(m+1),z(N);
  double * c = new double[m+1];
  double * s = new double[m+1];
  const int maxit = 10000;
  const double tol = 1.0e-7;
  double delta,rho,tmp;

  x.Initialize(0.0);

  r = b - A*x;

  for(j=0;j<maxit;j++){
    y.Initialize(0.0);
    y(0) = r.Norm_l2();
    r.Normalize();

    ModifiedArnoldi(m,r,A,H,V);

    /* Givens Rotation to accomplish QR factorization */
    for(i=0;i<m;i++){
      for(k=1;k<=i;k++){
	tmp = H(k-1,i);
	H(k-1,i) = c[k-1]*H(k-1,i) + s[k-1]*H(k,i);
	H(k,i) = -s[k-1]*tmp + c[k-1]*H(k,i);
      }

      delta = sqrt(H(i,i)*H(i,i)+H(i+1,i)*H(i+1,i));     
      c[i] = H(i,i)/delta;
      s[i] = H(i+1,i)/delta;

      H(i,i) = c[i]*H(i,i) + s[i]*H(i+1,i);
      
      for(k=i+1;k<m+1;k++)
	H(k,i) = 0.0;

      y(i+1) = -s[i]*y(i);
      y(i)   =  c[i]*y(i);
      rho = fabs(y(i+1));
      if(rho < tol){
	nr = i;
	break;
      }
    }

    /* Backsolve to obtain coefficients */
    z.Initialize(0.0);
    if(i>=(m-1)){ 
      nr = m; 
      z(nr-1) = y(nr-1)/H(nr-1,nr-1);
    }

    for(k=nr-2;k>=0;k--){
      z(k) = y(k);
      for(ll=k+1;ll<nr;ll++)
	z(k) -= H(k,ll)*z(ll);
      z(k) = z(k)/H(k,k);
    }
    
    /* Linear combination of basis vectors of the Krylov space */
    for(i=0;i<nr;i++){
      V.GetColumn(i,r);
      x = x + z(i)*r;
    }

    //    if(rho<tol)
    //  break;

    r = b - A*x;
  }
  delete[] c;
  delete[] s;
}

//int N, // number of unknowns 
//void gauss(int N, // number of unknowns
//float A [20] [21], // coefficients and constants
//float result[20],   bool& err)
// Solve system of N linear equations with N unknowns
// using Gaussian elimination with scaled partial pivoting
// First N rows and N+1 columns of A contain the system
// with right-hand sides of equations in column N+1
// err returns true if process fails; false if it is successful
// original contents of A are destroyed
// solution appears in column N
void GEPP(const SCMatrix &in_A, const SCVector &in_b, SCVector &x)
{
   int N=in_A.Rows();
   double **A=CreateMatrix(N, N+1);
   //double *result= new double[N];

   int input_rows,input_columns;
   for( input_rows=0;input_rows<N; input_rows++)
   {
         for( input_columns=0;input_columns<N; input_columns++)
         {
            A[input_rows][input_columns] =in_A(input_rows, input_columns);
         }
   }
   for( input_rows=0;input_rows<N; input_rows++)
   {
         A[input_rows][N] =in_b(input_rows);
   }

   //int indx[20];
   int *indx=new int[N];
   //double scale[20];
   double *scale=new double[N];
   //float maxRatio;
   double maxRatio;
   int maxIndx;
   int tmpIndx;
   //float ratio;
   double ratio;
   //float sum;
   double sum;

   const double ZERO = 1.0e-7;

   for (int i = 0; i < N; i++) indx[i] = i;	// index array initialization

   // determine scale factors
   for (int row = 0; row < N; row++)
   {
      scale[row] = fabs(A[row][0]);
      for (int col = 1; col < N; col++)
      {
         if (fabs(A[row][col]) > scale[row]) scale[row] = fabs(A[row][col]);
      }
   }

// forward elimination
   int k=0;
   for (k = 0; k < N; k++)
   {
      // determine index of pivot row
      //----maxRatio = abs(A[indx[k]] [k])/scale[indx[k]];
      maxRatio = fabs(A[indx[k]] [k])/scale[indx[k]];
      maxIndx = k;
      int i=0;
      for (i = k+1; i < N; i++)
      {
         //if (abs(A[indx[i]] [k])/scale[indx[i]] > maxRatio)
         if (fabs(A[indx[i]] [k])/scale[indx[i]] > maxRatio)
         {
            //maxRatio = abs(A[indx[i]] [k])/scale[indx[i]];
            maxRatio = fabs(A[indx[i]] [k])/scale[indx[i]];
            maxIndx = i;
         }
      }
      //if (maxRatio == 0) // no pivot available
      if (fabs(maxRatio) < ZERO) // no pivot available
      {
        //err = true;
        return;
      }
      tmpIndx = indx[k]; indx[k] = indx[maxIndx]; indx[maxIndx] = tmpIndx;

      // use pivot row to eliminate kth variable in "lower" rows
      for (i = k+1; i < N; i++)
      {
         ratio = -A[indx[i]] [k]/A[indx[k]] [k];
         for (int col = k; col <= N; col++)
         {
            A[indx[i]] [col] += ratio*A[indx[k]] [col];
         }
      }
   }

// back substitution

   for (k = N-1; k >= 0; k--)
   {
      sum = 0;
      for (int col = k+1; col < N; col++)
      {
         sum += A[indx[k]] [col] * A[indx[col]] [N];
      }
      A[indx[k]] [N] = (A[indx[k]] [N] - sum)/A[indx[k]] [k];
    }
    
    //PostProcessing
    //for (int k = 0; k < N; k++) result[k] = A[indx[k]] [N];

    for (k = 0; k < N; k++) x(k) = A[indx[k]] [N];

    DestroyMatrix(A, N);
     //int indx[20];
   delete [] indx;
   //double scale[20];
   delete [] scale;
}
void GaussElimination(SCMatrix &A, SCVector &b, int pivotflag){
  int k,pivot;
  int N = A.Rows();

  /* NOTE: The values contained in both the matrix A and
     the vector b are modified in this routine.  Upon 
     returning, A contains the upper triangular matrix
     obtained from its LU decomposition, and b contains
     the solution of the system Ax=b*/

  // Steps (1) and (2) (decomposition and solution of Ly = b)
  switch(pivotflag){
  case 1: // Case in which pivoting is employed

    for(k=0; k < N-1; k++){
      pivot = A.MaxModInColumnindex(k,k);
      A.RowSwap(pivot,k);
      Swap(b(pivot),b(k)); 
      for(int i=k+1;i<N;i++){
	double l_ik = A(i,k)/A(k,k);
	for(int j=k;j<N;j++)
	  A(i,j) = A(i,j) - l_ik*A(k,j);
	b(i) = b(i) - l_ik*b(k); 
      }
    }
    break;

  case 0:  // Case 0/default in which no pivoting is used
  default:

    for(k=0; k < N-1; k++){
      for(int i=k+1;i<N;i++){
	double l_ik = A(i,k)/A(k,k);
	for(int j=k;j<N;j++)
	  A(i,j) = A(i,j) - l_ik*A(k,j);
	b(i) = b(i) - l_ik*b(k); 
      }
    }
  }

  // Step (3) (backsolveing to solve Ux=y)  
  b(N-1) = b(N-1)/A(N-1,N-1);
  for(k=N-2;k>=0;k--){
    for(int j=k+1;j<N;j++)
      b(k) -= A(k,j)*b(j);
    b(k) = b(k)/A(k,k);
  }
}
 
}







