#
# gempis_setup - sets up the environment for the gempil application
#
# Description:
# This script will attempt to load any definitions defined in the system and 
# user definition files. Definitions that are absolutely required by gempil 
# can be included. ANy specific definitions should be defined in either the
# system or user definition files.
#
# Syntax: . gempil_setup
#
# Notes:
# 
############################################################################

# define the name of the gempis system definitions file
export GEMPIS_SYSTEM_DEF="$IBS_NAMES/gempis/definitions_system"

# define the name of the gempis user definitions file
export GEMPIS_USER_DEF="~/.gempis/defintions_user"

#
# define the default values of the string to be ignored by GEMPIL
# 
# Note: these values can be overridden in the sytstem and user definitions file
# Individual string are separated by a '|' symbol
# Here are some examples:
#   Warning: No xauth data; using fake authentication data for X11 forwarding.
#   Warning: Permanently added the RSA host key for IP address '134.144.125.83' to the list of known hosts.
#   Warning: Permanently added 'wss25ay,138.54.18.93' (RSA) to the list of known hosts.
#
# HOWEVER, this one needs to be flagged: 
#   No RSA host key is known for amsdc1-n-c00046 and you have requested strict checking. 
#
export GEMPIL_IGNORE_WARNINGS="X11|RSA host key|(RSA)|No xauth data"

#
# Here are some examples:
#   Error: X11 connection rejected because of wrong authentication
export GEMPIL_IGNORE_ERRORS=""

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
