#include <iostream>
#include <iomanip>

int main(int argc, char** argv)
{
   using namespace std;
   
   float myFloat = 0.123456789987654321123456789987654321;

   cout.setf(ios::fixed,ios::floatfield);
   cout << setw(25) << setprecision(18);

   cout << "  myFloat " << myFloat << endl;

   double myDouble = myFloat;

   cout << " myDouble " << myDouble << endl;

   float myFloat2 = myFloat;
   
   double myDouble2 = myFloat2;

   cout << "myDouble2 " << myDouble2 << endl;
   
   float myFloat3 = 0.345 ;
   
   cout << "  myFloat3 " << myFloat3 << endl;

   double myDouble3 = myFloat3;

   cout << "myDouble3 " << myDouble3 << endl;
   
   return 0;
}
