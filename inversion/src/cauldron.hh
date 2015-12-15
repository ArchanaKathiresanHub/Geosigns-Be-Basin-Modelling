#ifndef CAULDRON_HH
#define CAULDRON_HH

struct Cauldron
{

  std::string Project3d;
  std::string OutputDir;
  std::string Identifier; //experiment name
  std::string LaunchDir;

  std::string CommandOptions;//command line option, version 
  std::string CauldronMode;//itcoupled 
  std::string CauldronVersion;
  int Processors;
  std::string Cluster;
  //fprintf(finput,'Launcher %s\n',CauldronLauncher);
  //fprintf(finput,'Command %s\n',CauldronCommand);
   
};

#endif
