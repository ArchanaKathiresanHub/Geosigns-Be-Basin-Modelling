// #include <iostream>      
// #include <vector>        
// #include <fstream>       
// #include <sstream>       
// #include <iomanip>       
// #include <boost/math/distributions/chi_squared.hpp>
// using namespace std;     
// using boost::math::chi_squared; 
// using boost::math::quantile;

#include <boost/algorithm/string/join.hpp>
#include <vector>
#include <iostream>

int main(int, char **)
{
    std::vector<std::string> list;
    list.push_back("Hello");
    list.push_back("World!");

    std::string joined = boost::algorithm::join(list, ", ");
    std::cout << joined << std::endl;
}
