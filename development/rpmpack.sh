#!/bin/bash
set -e
CLDRN_VERSION=$1;
Build_BinariesDirectory=$2;

# /apps/sss
instapath=$3;
distb=$4;

rm -rf ${Build_BinariesDirectory}/*
Build_BuildNumber=11;

archi="";
specPath="";
if [ ${OSTYPE} == "cygwin" ];then
	archi="cygwin"
	specPath="C:\Users\Arijit.Khan\Source\Repos\Basin-Modelling\development"
else
	archi=`getos2`;
	specPath="./Basin-Modelling/development/my_ibs_version.spec "
fi

echo ${archi}
pwd
rpmbuild -bb -v  --buildroot=${Build_BinariesDirectory}/rpmbuild \
				 --define "_dbpath ${Build_BinariesDirectory}/rpmdb" \
				 --define "_installpath ${instapath}" \
				 --define "_topdir ${Build_BinariesDirectory}/" \
				 --define "_tmppath %{_topdir}/tmp" \
				 --define "_builddir ${Build_BinariesDirectory}" \
				 --define "_version ${CLDRN_VERSION}" \
				 --define "release ${Build_BuildNumber}" \
				 --define "architecture ${archi}" \
				 --define "_rpm_version ${archi}" \
				 --define "_rpmdir ${Build_BinariesDirectory}/RPMS" \
				 --define '_os anyos' \
				 --define '_cpu x86_64' \
				 --define "_distb ${distb}" \
				${specPath}
###
