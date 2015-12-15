#
# gempis_adm - GEneric MPIrun Submission ADMinistration tool
#
#############################################################################

# check that this routine has been called in the correct wat
if [ -z "$GEMPIS_USERNAME" ]; then
    echo "Error! Invalid command format, please use 

    gempis adm

instead.
"
    exit 1
fi

# name use by CSCE to locate this program
export GEMPIS_PROGNAME="gempis_adm"
verbose "$GEMPIS_PROGNAME [$*]."

############################################################################
#
# tell user how I work and exit
#
# This one is defined but not implemented
#               transfer <filename> [<filename> ...]
usage()
{
    # terminate application 
    terminate ERROR "Invalid command line. The options are:

    gempis adm help
               setup
               keygen
               keyrestore
               cleanup
               email <filename> [user@<destination>]
"
}

############################################################################
#
# waits for an input from the user, only the correct answer will continue
#  otherwise aborts
#
enter_value()
{
    response="$1"

    read ans

    echo ""

    if [ "$ans" != "$response" ]; then
	echo "Only the answer [$response] will continue."

	terminate ERROR "Unaccepted value entered."
    fi
}

############################################################################
#
# cleanup the users environment
#
# will remove all temporary and debug gempis files defined in the user
# job directory.
#
cleanup_user()
{
    echo "Clean up temporary directories created by gempis."

    echo "- Checking for gempis directories..."
    cat ~/.gempis/user-START.log | awk -F'|' '{ print $4 }' | sort -u | while read dir dummy; do

	verbose "- Checking directory [$dir]."

	if [ -d "$dir/.gempis_tmp" ]; then

	    echo "- Removing gempis temporary directory [$dir/.gempis_tmp]."
	    rm -rf $dir/.gempis_tmp
	fi
    done
}

############################################################################
#
# copies a file, making a backup of any original file first
#
# $1 - src file
# $2 - dst file
#
# both names need to be complete
#
docopy()
{
    src=$1
    dst=$2

    # check parms
    if [ -z "${src}" -o -z "${dst}" ]; then
	echo "Error!"
	echo "Source ($src) or destination ($dst) file is not defined."
	return
    fi

    # check if file exists
    if [ -f $dst ]; then
	backup="${dst}-`get_datetime`"
	echo "- Backing up file ${dst}."
	cp -f ${dst} $backup
    fi

    # copy file
    echo "- Copying file to ${dst}."
    cp -f ${src} ${dst}
    chmod 755 ${dst}
}

############################################################################
#
# setup the users environment
#
setup_user()
{
    # warn 'em
    echo ""
    echo "This procedure will setup gempis for use."
    echo ""

    # check for blank name
    if [ -z "$region" ]; then

	# save current directory and mobve to resource
	cwd=`pwd`
	cd $GEMPIS_DEFDIR/regional_resources

	echo "The following regions have been defined:"
	echo ""
	ls resources_* | grep -v _TEMPLATE | sed 's/resources_//' | awk '{ printf("    %s\n", $1) }'

	echo ""
	echo "You are not required to define a region."
	$ECHONOCR "Please enter your Region name from the list [press <Enter> for none ]: \c"
	read ans

	# return to previous directory
	cd $cwd

	if [ $GEMPIS_OS = "Solaris" ]; then
	    region=`echo $ans | nawk '{ printf("%s\n", toupper($1)) }'`
	else
	    region=`echo $ans | awk '{ printf("%s\n", toupper($1)) }'`
	fi
    fi

    if [ -z "$region" ]; then
	echo "No region has been defined, setting name to TEMPLATE."
	region=TEMPLATE
    fi

    # check if a resource name exists
    ls $GEMPIS_DEFDIR/regional_resources/resources_$region >/dev/null 2>&1
    if [ $? != 0 ]; then
	echo ""
	echo "Region [$region] does not exist."
	echo "This value will, however, be save in the user definition file."
    fi

    echo ""
    echo "WARNING."
    echo "Continuing this procedure will replace any previous gempis definitions."
    echo ""
    $ECHONOCR "Do you wish to proceed? [Type yes to continue]: \c"
    enter_value yes

    # create directory if it doesn't exist
    if [ ! -d $GEMPIS_USERDIR ]; then
	echo "- Creating GEMPIS user directory [$GEMPIS_USERDIR]."
	mkdir -p $GEMPIS_USERDIR
	chmod -R 755 $GEMPIS_USERDIR
    fi

    # copy over user definition
    echo "- Copying template files to users gempis directory."
    for file in `ls $GEMPIS_TEMPLATE_DIR`; do
    	docopy $GEMPIS_TEMPLATE_DIR/$file $GEMPIS_USERDIR/$file
    done

    # create user file name with region definition
    echo "- Defining region [$region] in $GEMPIS_USERDIR/definitions_user."
    cat $GEMPIS_USERDIR/definitions_user | sed "s/TEMPLATE/$region/g" >$GEMPIS_USERDIR/definitions_user.tmp
    mv $GEMPIS_USERDIR/definitions_user.tmp $GEMPIS_USERDIR/definitions_user

    echo ""
    echo "Setup complete."
    echo ""
}


############################################################################
#
# moves a directory to a backup location
#
# $1 - src directory
#
domove()
{
    src=$1

    # check parms
    if [ -z "$src" ]; then
	echo "Error!"
	echo "Source ($src) directory is not defined."
	return
    fi

    # check if directory exists
    if [ ! -d ${src} ]; then
	echo "Source ($src) directory does not exist."
	return
    fi

    backup="BACKUP-${src}-`get_datetime`"
    echo "- Moving original ${src} file to $backup."
    mv -f ${src} $backup
}


############################################################################
#
# generates a new user ssh public and private domain keys
#
key_gen()
{
    cat <<EOF

This procedure will create new ssh keys in order to 
allow gempis to auto-login to remote computers. 

EOF
    # move into user home directory
    cd

    # check that the home diretory is defined in such a way that 
    # 
    id=`/apps/oss/bin/whoami`
    perms=`ls -ld ../$id | awk '{ print $1 }'`

    group="`echo $perms | cut -c6`"
    other="`echo $perms | cut -c9`"

    if [ "$group" != "-" -o "$other" != "-" ]; then

	# warn'em
	cat <<EOF 
---------------------------------------------------------
                        WARNING.....

  ssh requires that a user HOME directory have limited
        access permission for automatic logins. 

  Your HOME directory is not correctly configured and 
             will be changed to permission
                       user  = rwx
                       group = r-x
                       other = r-x
  which may allow other users to view files in your home 
                        directory. 

Higher protection is allowed by ssh. If this is required 
   exit this script and change the directory by hand.
---------------------------------------------------------

EOF
    else
	echo "The user HOME directory permissions are correct for ssh."
	echo ""

	# warn'em
	cat <<EOF 
---------------------------------------------------------
                        WARNING.....

  This procedure will create a new set of ssh keys. Any 
   previous ssh keys will be removed by the procedure.
---------------------------------------------------------

EOF
    fi

    echo ""
    $ECHONOCR "Do you wish to proceed? [Type 'yes' to continue]: \c"
    enter_value yes

    if [ "$group" != "-" -o "$other" != "-" ]; then

	echo "- Changing user HOME directory to rwxr-x-r-x"
	chmod 755 ../$id
	echo ""

	if [ $? -ne 0 ]; then
	    echo "ERROR attempting to change file permission of your HOME directory."
	    echo "The directory is currently set as shown"

	    ls -ld ../$id

	    echo ""
	    echo "It should be set with your userid as owner and file permissions"
	    echo "set to drwxr-xr-x"
	    echo ""
	    echo " Please contact your support department."
	    exit 1
	else
	    echo "- Directory permission changed successfully."
        fi
    fi

    echo "- Saving previous ssh configuration."
    domove .ssh

    echo "- Generating ssh keys"
    echo ""
    echo "    Do what the program tells you,"
    echo "       but leave the password blank... "
    echo ""

    # run the keygen command
    ssh-keygen -b 1024 -t dsa 

    echo ""

    # check that the ssh directory has been created
    [ ! -d .ssh ] && terminate ERROR "HOME/.ssh was not created."

    echo "- Copying ~/.ssh/id_dsa.pub to ~/.ssh/authorized_keys." 
    cd .ssh
    cp id_dsa.pub authorized_keys
    chmod 644 authorized_keys

    echo ""
    echo "Procedure complete."
    echo ""
    return 0
}

############################################################################
#
# restores a previous user ssh keys, if defined 
#
key_restore()
{
    echo "Restoring previous ssh keys."
    echo ""

    cd 

    echo "- Searching for last stored ssh directory."
    last=`ls -dtr BACKUP-.ssh* | tail -1 2>/dev/null`
    result=$?
    if [ $result != 0 ]; then
	echo "No previous version was found."
	exit 1
    fi

    if [ ! -d .ssh ]; then
	echo "No previous ssh directory."
    else
	echo "- Removing current .ssh directory."
	rm -rf .ssh
    fi

    echo "- Restoring previous ssh directory $last to .ssh."
    mv -f $last .ssh

    # make sure mode is set correctly
    chmod 700 .ssh

    echo ""
    echo "Procedure complete."
    echo ""
    return 0
}

############################################################################
#
# send the contents of a file to support
# First check the file type, if not ascii will send it as an attachment
#
email_file()
{
    file=$1
    dest=$2

    # CHECK FILE
    if [ -z "$file" ]; then
	terminate ERROR "No file name defined."
    fi
    if [ ! -f $file ]; then
	echo "ERROR! File $file not found."
	exit 1
    fi

    # check destination
    if [ -z "$dest" ]; then
	dest=$GEMPIS_EMAIL_HELPDESK
    fi
    if [ -z "$dest" ]; then
	echo "ERROR! destination has not been defined."
	exit 1
    fi

    # check the file type, if not ASCII, send as an attachment
    echo "E-mailing file [$file] to [$dest]."
    typ=`file $file`
    verbose "File type is [$typ]."
    asci=`echo "$typ" | grep -i ASCII`
    if [ -n "$asci" ]; then
        # send contents of file as email
	verbose "File [$file] is sent as contents of email."
	cat $file | $GEMPIS_MAIL -s "Gempis file $file" $dest
    else
	# send file as attachment
	verbose "File [$file] is sent as attachment."
	uuencode $file $file | $GEMPIS_MAIL -s "Gempis file $file" $dest
    fi
    result=$?

    # return the result
    echo ""
    return $result
}


############################################################################
#
# package files into tarbal and use scp to trnasfer
#
transfer_files()
{
    filelist="$*"

    echo "Creating compressed tar file."
    echo ""

    tar cvfz $filename $filelist
    result=$?

    if [ $result != 0 ]; then
	echo "ERROR creating file $filename."
	#scp $filename 
    fi
    return $result
}


############################################################################
#
# main
#

# if no parameters then display usage message
[ $# -lt 1 ] && usage

# define the system and user environment settings
load_definitions

# process some of the simple commands
if [ "$1" = "help" ]; then
    show_help
    exit 0
fi
if [ "$1" = "setup" ]; then
    setup_user $2
    exit $?
fi

# load resources
load_resources ignore

if [ "$1" = "cleanup" ]; then
    cleanup_user
    exit $?
fi
if [ "$1" = "keygen" ]; then
    key_gen
    exit $$$?
fi
if [ "$1" = "keyrestore" ]; then
    key_restore
    exit $$$?
fi
if [ "$1" = "email" ]; then
    shift
    email_file $*
    exit $?
fi
if [ "$1" = "transfer" ]; then
    shift
    transfer_files $*
    exit $?
fi

echo "Unknown option [$1]"
exit 1

# End script
