#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include "tokenize.hh"
#include <iterator>
using namespace std;





int main()
{
  std::string str;
  ifstream file_up( "wellinput.dat" );
  
while(!file_up.eof())
 	{
  std::vector<string> vecStr;
  getline(file_up, str);
  
  //tokenize(str, vecStr, "\t");
  tokenize(str, vecStr, " ");
   for(int i =1; i<vecStr.size(); ++i)
	 std::cout<<vecStr[i]<<std::endl;
  //std::cout<<"size: "<<vecStr.size()<<std::endl;
}
}





// struct Record
// {
//     string    name;
//     string    address;
//     string    phone;
// };

// int main()
// {
//     ifstream inFile( "tmp/test.dat" );

//     if ( !inFile )
//     {
//         cout << "Error opening file!";
//         return 1;
//     }

//     vector<string> lines;
//     string line;

//     // Read each line into a vector.
//     while ( getline( inFile, line ) )
//     {
// 	  std::cout<<"line size: "<<line.size()<<std::endl;
//         lines.push_back( line );
//     }

//     vector<Record> records;
//     string field;

//     // Parse each line and put the fields into a vector of records.
//     for ( vector<string>::iterator it = lines.begin(); it != lines.end(); ++it )
//     {
//         Record record;
//         size_t start = 0;
//         size_t end   = it->find( "\t" );

//         record.name = it->substr( start, end );
// 		std::cout<<"name_size: "<<record.name.size()<<std::endl;
// 		//if(record.name=="\t") std::cout<<"there is a tab";
//         start = end + 1;

//         end = it->find( "\t", start );
//         record.address = it->substr( start, (end - start) );
//         start = end + 1;
// 		std::cout<<"add_size: "<<record.address.size()<<std::endl;
//         record.phone = it->substr( start );
//         records.push_back( record );
//     }

//     // for ( vector<Record>::iterator it = records.begin(); it != records.end(); ++it )
// //     {
// //         cout << it->name    << endl
// //              << it->address << endl
// //              << it->phone   << endl
// //              << "--------------------------------------" << endl;
// //     }

//     return 0;
// }




// void tokenize(const string& str,
// 			  vector<string>& tokens,
// 			  const string& delimiters = " ");

// int main(int argc, char* argv[])
// {
//   //std::ifstream file_up("update.dat",ios::in);//file with update time details
//   //std::ifstream file_up("tmp/test.dat",ios::in);//file with update time details
//   //std::string str;


//  std::string token;
//  std::ifstream file("tmp/test.dat");
// while(std::getline(file, token))
//   {
// 	if(file.get() == '\t') {
// 	  std::cout << "tab found" << std::endl;
// 	}
//     std::istringstream line(token);
//     while(line >> token)
// 	  {
// 		std::cout << "Token :" << token << std::endl;
// 	  }
    
// }


 
  // while(!file_up.eof())
// 	{
// 	  std::vector<string> vecStr;
// 	  getline(file_up, str);
// 	  //std::cout<<"str: "<<str<<std::endl;
// 	  tokenize(str, vecStr, " ");
// 	  //std::cout<<"size: "<<vecStr.size()<<std::endl;

// 	   if(vecStr.size() > 0)
// 		{
// 		  if(vecStr[0]=="KEYWORD")
// 			{
// 			  std::string newstr;
// 			  for(int i =1; i<vecStr.size(); ++i)
// 				newstr+=vecStr[i]+" ";
// 			  std::cout<<newstr<<std::endl;
// 			}
// 		}
// 	}
	 
			
 //  std::string str="filename";
//   std::vector<string> vecStr;
//   tokenize(str, vecStr, ".");
//   std::cout<<vecStr[0]<<"  "<<vecStr[1]<<std::endl;
  
  

//   //=============== read the input file =============// 
//   int wellCOUNT=0;
//   int num;
//   string str;
//   std::vector<std::vector<double> > CTimeVec;
//   std::vector<std::vector<std::string> > propVec;
//   std::vector<double> vecTime;
//   std::vector<double> vecCTimeStatus;
//   std::vector<string>propVecStatus;
//   std::vector<string>wellNames;
//   std::ifstream file_op("test_file.txt",ios::in);
//   while(!file_op.eof())
// 	{
// 	  std::vector<string> vecStr;
// 	  getline(file_op, str);
// 	  //std::cout<<"str: "<<str<<std::endl;
// 	  tokenize(str, vecStr, " ");
// 	  //std::cout<<"size: "<<vecStr.size()<<std::endl;
// 	  if(vecStr.size() > 0)
// 		if(vecStr[0]=="TIME")
// 		  {
// 			double n;
// 			std::istringstream i(vecStr[2]); // 3rd string
// 			i >> n;
// 			//std::cout<<"TIME: "<<n<<std::endl;
// 			vecTime.push_back(n);
// 		}
// 		else if(vecStr[0]=="START")
// 		{
// 		  //std::cout<<"start well zone name: "<<vecStr[1]<<std::endl;
// 			//get the well name
// 			//getline(file_op, str);
// 			//std::cout<<"well name: "<<str<<std::endl;
// 		}
// 		else if(vecStr[0]=="WELL")
// 		{
// 		  //std::cout<<"well name: "<<vecStr[2]<<std::endl;
// 		  wellCOUNT++;
// 		  wellNames.push_back(vecStr[2]);
// 		  if(wellCOUNT > 1)
// 			{
// 			  CTimeVec.push_back(vecCTimeStatus);
// 			  propVec.push_back(propVecStatus);
// 			  // for(int i =0; i<vecCTimeStatus.size(); ++i)
// // 				std::cout<<"CTime: "<<vecCTimeStatus[i]<<std::endl;
// 			  vecCTimeStatus.clear();
// 			  propVecStatus.clear();
// 			}
// 		}
// 		else if(vecStr[0]=="PROPERTY")
// 		  {
// 			//std::cout<<"No of properties: "<<vecStr[1]<<" "<<vecStr[2]<<" "<<vecStr.size()-2<<std::endl;
// 			std::string i = vecStr[2];
// 			propVecStatus.push_back(i);
// 		  }
// 	  else if(vecStr[0]=="COUPLING")
// 		{
// 		  double n;
// 		  istringstream i(vecStr[3]); // 4th string
// 		  i >> n;
// 		  vecCTimeStatus.push_back(n);
// 		  // for(int i =0; i<vecCTimeStatus.size(); ++i)
// // 				std::cout<<"time: "<<vecCTimeStatus[i]<<std::endl;
// 		}
// 	  else if(vecStr[0]=="STOP")
// 		{
// 		  //std::cout<<"stop well zone name: "<<vecStr[1]<<std::endl;
// 		  CTimeVec.push_back(vecCTimeStatus);
// 		  propVec.push_back(propVecStatus);
// 			  // for(int i =0; i<vecCTimeStatus.size(); ++i)
// // 				std::cout<<"CTime: "<<vecCTimeStatus[i]<<std::endl;
// 			  vecCTimeStatus.clear();
// 			  propVecStatus.clear();
// 		}
// 	  else continue;
// 	}
//   file_op.close();
    
//   // for(int i =0; i<CTimeVec.size(); ++i)
//   //   for(int j =0; j<CTimeVec[i].size(); ++j)
//   // 	std::cout<<"CTime: "<<CTimeVec[i][j]<<std::endl;
//    // for(int i =0; i<propVec.size(); ++i)
// //      for(int j =0; j<propVec[i].size(); ++j)
// // 	   std::cout<<"prop: "<<propVec[i][j]<<std::endl;
  
//   //==============end of file read =========//


//   //write TIME to time_skin
  
//   std::ofstream SkinF("SKIN_FULL.dat");

//    for(int i =0; i<CTimeVec.size(); ++i)
// 	 {
// 	   for(int k =0; k<CTimeVec.size(); ++k)
// 		 {
// 		   if(k==i)
// 			 SkinF <<"WELLSTATUS "<<"\""<<wellNames[k]<<"\"  = OPEN  "<<std::endl;
// 		   else
// 			 SkinF <<"WELLSTATUS "<<"\""<<wellNames[k]<<"\"  = CLOSED  "<<std::endl;
// 		 }
// 	   for(int j =0; j<CTimeVec[i].size(); ++j)
// 		 {
// 		   SkinF<<"TIME = "<<vecTime[0]+CTimeVec[i][j]<<std::endl;
// 	   }
// 	 }

//    SkinF.close();

//    std::ofstream SkinZ("SKIN_ZONE.dat");

//    for(int i =0; i<CTimeVec.size(); ++i)
//      for(int j =0; j<CTimeVec[i].size(); ++j)
//    	SkinZ<<"TIME = "<<CTimeVec[i][j]-CTimeVec[i][j-1]<<std::endl;

//    SkinZ.close();
   
  







