The solution contains the following projects:

4DPrototype 

	A Qt desktop application used to develop the visualization. Uses the Visualization library (below)

QtService

	Qt component that contains the basics to build a Windows Service. Originally intended to be used as a basis for the render service, but not actually used because of a problem known as "Session 0 Isolation". This means that Windows Services don't have access to the graphics hardware, which makes 3D rendering from a service impossible. 

RenderService

	The RemoteViz server. Currently just an application that needs to be started by a user, although the code to make it a Windows Service is still in there behind an #ifdef. Uses the Visualization library (below).

Visualization

	Library that handles the actual visualization. Builds an Open Inventor scene graph from a ProjectHandle as defined by the DataAccess library. Used by 4DPrototype and RenderService.


Outside of the VS2010 solution, there is also the WebClient folder, which contains the html and javascript for the web page.


BUILDING
----------------------

The solution currently depends on the following libraries:

	Open Inventor 9.4 beta
	Open Inventor RemoteViz 9.4 beta
	Qt 4.8.4
	The Basin Modeling 'DataAccess' library


Building the DataAccess library

The DataAccess library uses CMake to build. Generate VS2010 project files by running the 'bootstrap.bat' batch file (in the Basinmodeling/developmen folder), then use VS2010 to build the libraries. To make it easier to build to 4DPrototype solution, I put all the necessary libraries in a single folder:

	CBMGenerics.lib
	DataAccess.lib
	hdf5.lib
	Serial_Hdf5.lib
	SerialDataAccess.lib
	TableIO.lib
	utilities.lib

This folder is then referenced in your linker settings in VS. The base directory is referenced in the project using an environment variable BPALIBDIR, and uses subdirectories 'release' and 'debug'.


Environment variables

The following env variables need to be defined in order to build the project:

	OIVHOME         - Base directory for the Open Inventor installation
	REMOTEVIZHOME   - Base directory for the OIV RemoteViz installation
	QTDIR           - Qt base directory
	BPALIBDIR       - Directory where the compiled DataAccess libs are stored.
	                  The project uses $(BPALIBDIR)\release and $(BPALIBDIR)\debug


Server data loading

At the moment, the server still uses a hardcoded path to the data set. This can be changed in the following file:

	4DPrototype\RenderService\BpaRenderAreaListener.cpp (in function createSceneGraph())


OIV licensing

A license is needed for the Open Inventor libraries. This comes in the form of a password.dat file, that needs to be placed somewhere where OIV can find it: in the 'license' folder in the OIV installation, next to the application executable, or in a place specified by the VSG_LICENSE_FILE environment variable. 