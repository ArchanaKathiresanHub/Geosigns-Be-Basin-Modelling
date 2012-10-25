#!/bin/csh 
# ------------------------------------------------------------------
# goibs_develop_startup.login: 
# Set environment variables for IBS development environment.
# ------------------------------------------------------------------

# Set the version of the IBS system to run 
# (production version, test or develop version):
  setenv ibsversion Development

# Set name of directory for version:
  setenv IBS_VERSION d

# Set ibs directory structure variable:
  setenv IBS_HOME /nfs/rvl/users/ibs

# do the version independent IBS setup:
  source ${IBS_HOME}/bin/goibs_startup_2.login

# tell user goibs_develop_starup has been run:
  echo "goibs_develop_startup.login executed."

### AFS settings
##
 set path = (/usr/afsws/bin $path)
##
### do not clash with personal afs aliasses [al]:
## alias ibsafs 'cd /.../users/ibs'
