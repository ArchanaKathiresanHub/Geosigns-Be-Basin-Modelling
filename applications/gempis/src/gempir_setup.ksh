#
# gempir_setup - sets up the environment for the gempil application
#
# Description:
# This script will attempt to load any definitions defined in the system and 
# user definition files. Definitions that are absolutely required by gempil 
# can be included. ANy specific definitions should be defined in either the
# system or user definition files.
#
# Syntax: . gempir_setup
#
# Notes:
# 
############################################################################

# define the name of the gempis system definitions file
export GEMPIS_SYSTEM_DEF="$IBS_NAMES/gempis/definitions_system"

# define the name of the gempis user definitions file
export GEMPIS_USER_DEF="~/.gempis/defintions_user"

#
# check that system definitions file has been found, and run
[ -x "$GEMPIS_SYSTEM_DEF" ] && . $GEMPIS_SYSTEM_DEF

#
# define user definitions if they exist 
[ -x "$GEMPIS_USER_DEF" ] && . $GEMPIS_USER_DEF

#
# enable VERBOSE messages in all cases
export GEMPIS_VERBOSE=yes

#
# enable gempis to display formatted messages
export GEMPIS_FORMAT_MESSAGES=yes

# End script
