#!/bin/bash

#########################################################################
#                                                                       #
# Copyright (C) 2015-2016 Shell International Exploration & Production. #
# All rights reserved.                                                  #
#                                                                       #
# Developed under license for Shell by PDS BV.                          #
#                                                                       #
# Confidential and proprietary source code of Shell.                    #
# Do not distribute without written permission from Shell.              #
#                                                                       #
#########################################################################

########################################################
# This script is used to run Jenkins jobs from VSTS    #
#       Step 1 - Convert user to email                 #
#       Step 2 - Launch jenkins build                  #
########################################################

umask 002
# check argument count
if [ "$#" -ne 4 ]; then
   echo "Not the right number of arguments ($#) for fastretrieve, should be 4: runJenkins <JenkinsURL> <JobName> <jobToken> <RequestedBy>"
   exit 1
fi
JENKINS_URL=$1
JOB_NAME=$2
JOB_TOKEN=$3
REQUESTED_BY=$4

# convert user name to email
if echo $REQUESTED_BY | grep -iq Agrawal; then
   export EMAIL=Prakrati.Agrawal@shell.com;
elif echo $REQUESTED_BY | grep -iq Dinko; then
   export EMAIL=dinko-m.ivanov@pds.nl;
elif echo $REQUESTED_BY | grep -iq Fuss; then
   export EMAIL=gaetan.fuss@pds.nl;
elif echo $REQUESTED_BY | grep -iq Parlakov; then
   export EMAIL=georgi.parlakov@pds.nl;
elif echo $REQUESTED_BY | grep -iq Lee; then
   export EMAIL=jane.lee@pds.nl;
elif echo $REQUESTED_BY | grep -iq Jokela; then
   export EMAIL=jarmo.jokela@pds.nl;
elif echo $REQUESTED_BY | grep -iq Leventis; then
   export EMAIL=konstantinos.leventis@pds.nl;
elif echo $REQUESTED_BY | grep -iq Carniato; then
   export EMAIL=luca.carniato@pds.nl;
elif echo $REQUESTED_BY | grep -iq Cougnenc; then
   export EMAIL=magali.cougnenc@pds.nl;
elif echo $REQUESTED_BY | grep -iq Lassche; then
   export EMAIL=mark.lassche@pds.nl;
elif echo $REQUESTED_BY | grep -iq Beccaria; then
   export EMAIL=beccaria.massimiliano@pds.nl;
elif echo $REQUESTED_BY | grep -iq Kuznetsova; then
   export EMAIL=natalya.kuznetsova@pds.nl;
elif echo $REQUESTED_BY | grep -iq Baramov; then
   export EMAIL=pavel.baramov@pds.nl;
elif echo $REQUESTED_BY | grep -iq Groenenberg; then
   export EMAIL=remco.groenenberg@pds.nl;
elif echo $REQUESTED_BY | grep -iq Kole; then
   export EMAIL=sebastiaan.kole@pds.nl;
elif echo $REQUESTED_BY | grep -iq Ree; then
   export EMAIL=serge.vanderree@pds.nl;
elif echo $REQUESTED_BY | grep -iq Koshelev; then
   export EMAIL=sergey.koshelev@pds.nl;
elif echo $REQUESTED_BY | grep -iq Senior; then
   export EMAIL=william.senior@pds.nl;
else
   echo "ERROR: User $REQUESTED_BY is not authorized to launch checkin builds."
   echo "ERROR: Please contact GAetan Fuss or Sergey Koshelev to be added to the list of athorized users."
   exit 1
fi

# launch jenkins build
JENKINS_COMMAND="curl -X POST $JENKINS_URL/job/$JOB_NAME/build?token=$JOB_TOKEN --data-urlencode json='{\"parameter\": [{\"name\":\"REQUESTED_FOR\", \"value\":\"$EMAIL\"}]}'"
echo "$JENKINS_COMMAND"
eval $JENKINS_COMMAND
