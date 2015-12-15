
#include <iostream>

using std::cout;
using std::cin;
using std::endl;

#include <hdf5.h>

// this example writes data to the HDF5 file
// data conversion is performed during write operation

// the main function

int main()
{
     cout << " --> define local variables" << endl;

     const int NX = 600;
     const int NY = 600;
     const int NZ = 600;
     const int RANK = 3;
	 const int I_MAX = 20;
     const char * FILE_LOC = "SDS_3D.h5";
     const char * DATASETNAME = "IntArrayAlpha3D";

     hid_t  file; // file handle
     hid_t  dataset; // dataset handles
     hid_t  datatype; // datatype handle
     hid_t  dataspace; // dataspace handle
     hsize_t  dimsf[3]; // dataset dimensions
     herr_t  status;

     // data to write

     cout << " --> create the data to write" << endl;

     int *** data = new int ** [NX];

     for (int i = 0; i != NX; i++) {
          data[i] = new int * [NY];
     }

     for (int i = 0; i != NX; i++) {
		for (int j = 0; j != NY; j++)
		{
          	data[i][j] = new int [NZ];
		}
     }


     // data and output buffer initialization

     for (int i = 0; i != NX; i++) {
          for (int j = 0; j != NY; j++) {
			for (int k = 0; k != NZ; k++)
			{
               	data[i][j][k] = i + j + k;
			}
          }
     }

     // Create a new file using H5F_ACC_TRUNC access,
     // default file creation properties, and default file
     // access properties

     cout << " --> create an HDF5 file" << endl;

     file = H5Fcreate(FILE_LOC, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

     // Describe the size of the array
     // and create the data space for fixed size dataset

     cout << " --> describe the size of the array" << endl;
     cout << " --> and create the data space for fixed size dataset" << endl;

     dimsf[0] = NX;
     dimsf[1] = NY;
     dimsf[2] = NZ;

     dataspace = H5Screate_simple(RANK, dimsf, NULL);

     // define datatype for the data in the file
     // we will store little endian INT numbers

     cout << " --> define datatype for the data in the file" << endl;

     datatype = H5Tcopy(H5T_NATIVE_INT);
     status = H5Tset_order(datatype, H5T_ORDER_LE);

     cout << " --> status = " << status << endl;

     // Create a new dataset within the file using defined dataspace and
     // datatype and default dataset creation properties

     cout << " --> create a new dataset within the file using defined dataspace" << endl;
     cout << " --> and datatype and default dataset creation properties" << endl;

     dataset = H5Dcreate1(file,
                          DATASETNAME,
                          datatype,
                          dataspace,
                          H5P_DEFAULT);

     // write the data to the dataset using default transfer properties

     cout << " --> write data to the dataset using default transfer properties" << endl;

	double t1 = clock();

	for (int i = 0; i != I_MAX; i++)
	{
		cout << "---------------------------------------------------->> " << i << endl;

     	status = H5Dwrite(dataset,
          	              H5T_NATIVE_INT,
               	          H5S_ALL,
                    	  H5S_ALL,
                       	  H5P_DEFAULT,
                          data);
	}

     cout << " --> status = " << status << endl;

	double t2 = clock();

     cout << " --> time used = " << (t2-t1)/CLOCKS_PER_SEC << " secs" << endl;

     double totalBytes = NX * NY * NZ * sizeof(int) * I_MAX;
     
     cout << " --> data written = "
          << totalBytes/1024.0/1024.0 << " MBytes" << endl;

     cout << " --> I/O speed = "
          << totalBytes/1024.0/1024.0/((t2-t1)/CLOCKS_PER_SEC)
          << " MBytes/sec" << endl;

     // Close and release resources

     H5Sclose(dataspace);
     H5Tclose(datatype);
     H5Dclose(dataset);
     H5Fclose(file);

     return 0;
}

// END
