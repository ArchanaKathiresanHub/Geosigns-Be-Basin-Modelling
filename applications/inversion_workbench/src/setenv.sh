echo =========================================
echo Loading inversion development environment
echo -----------------------------------------

# Some directories
export INVERSION_ROOT=/nfs/rvl/groups/ept-sg/SWEast/Cauldron/inversion
export CAULDRON_DEV=$INVERSION_ROOT/src/c++/cauldron
export EOSPACKDIR=$CAULDRON_DEV/geocase/misc/eospack
export GENEXDIR=$CAULDRON_DEV/geocase/misc/genex40  
export GENEX5DIR=$CAULDRON_DEV/geocase/misc/genex50 
export GENEX6DIR=$CAULDRON_DEV/geocase/misc/genex60
export OTGCDIR=$CAULDRON_DEV/geocase/misc/OTGC 


echo Setting PATH variable
# Setting the PATH variable for things like 'gempis'
if ( echo $PATH | grep -v -q '/apps/sssdev/share/\?\($\|:\)' ) ; then
  export PATH=/apps/sssdev/share:$PATH
fi

echo Importing license server list from $INVERSION_ROOT/licenses
source $INVERSION_ROOT/licenses

# set umask group friendly
echo -n "Setting 'umask' from `umask -S` to "
umask 0006
echo `umask -S`

# building Cauldron
echo Loading functions:
echo "- cauldron_make_clean: removes all generated binary files"
function cauldron_make_clean()
{  
  echo -n Removing all obj.Linux64, bin.Linux64, and Linux64 directories ...
  find $CAULDRON_DEV -name obj.Linux64 -or -name bin.Linux64 -or -name Linux64 | xargs rm -Rf
  echo DONE
}

echo "- cauldron_make: builds the standard cauldron applications"
function cauldron_make()
{
  case $1 in
    debug) 
      echo Making DEBUG build
      pushd $CAULDRON_DEV/applications
      IBS_LIMITEDBUILDANDINSTALL=1 DEBUGFLAGS="-g" ./BuildAll debug
      popd
      ;;

    optimized)
      echo Making optimized build
      pushd $CAULDRON_DEV/applications
      IBS_LIMITEDBUILDANDINSTALL=1 ./BuildAll optimized
      popd
      ;;

    *)
      echo "Usage: cauldron_make [debug|optimized]"
      ;;

  esac
}

# Running cauldron
echo "- cauldron_run: runs fastcauldron"
function cauldron_run()
{
  FC=$CAULDRON_DEV/applications/fastcauldron/obj.Linux64/fastcauldron
  if [ ! -e $FC ]; then
    echo The fastcauldron executable doesn't exist. You either haven't build
    echo the project yet, use 'cauldron_make' for that, or the build has failed,
    echo in which case you can consult $CAULDRON_DEV/applications/fastcauldron.log for more details.
    return 1;
  fi
    
  $FC $*
}

# Back up (my apologies for not using version control system like subversion)
echo "- inversion_backup: Backups inversion related files and puts them in $INVERSION_ROOT/backup directory"
echo "                    It DOES NOT BACKUP the cauldron source files, because they are already in TFS"
function inversion_backup()
{
  TARBALL="inversion-$( date -u +%Y%m%dT%H%M%SZ ).tar.gz"

  pushd $INVERSION_ROOT > /dev/null
  find . '(' -wholename "./src/c++/cauldron" -o -wholename "./backup" ')' -prune -o -type f -print | xargs tar czvf backup/$TARBALL
  popd > /dev/null

  echo Succesfully wrote backup to $INVERSION_ROOT/backup/$TARBALL
}

echo DONE
echo =========================================

