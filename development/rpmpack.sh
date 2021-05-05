#!/bin/bash
set -e
set -f
CLDRN_VERSION=$1;
SOURCE_CLDRN_VERSION=$6;
Build_BinariesDirectory=$2;

if [ -z "${SOURCE_CLDRN_VERSION}" ]
then
      echo "\$SOURCE_CLDRN_VERSION is empty, searching by the same name as set in TARGET vesrion!"
	  SOURCE_CLDRN_VERSION=${CLDRN_VERSION};
else
      echo "\$SOURCE_CLDRN_VERSION is NOT empty, using the name provided!"
fi

# /apps/sss
instapath=$3;
distb=$4;

rm -rf ${Build_BinariesDirectory}/*
Build_BuildNumber=$5;

archi="";
specPath="";
rpm_version="";
if [ ${OSTYPE} == "cygwin" ];then
	archi="cygwin"
	specPath="C:\Users\Arijit.Khan\Source\Repos\Basin-Modelling\development"
else
	#LinuxRHEL64_x86_64_79WS
	archi=`getos2`; 
	array=(${archi//_/ });
	for i in "${!array[@]}"
		do
			array[i]=`echo "${array[i]}" | tr '[:upper:]' '[:lower:]'`;
		done
	#convert to linuxrhel_79ws-x86_64
	SUF=$(printf '%s' "${array[0]}" | tr -d '0123456789')
	rpm_version=$SUF"_"${array[3]};
	specPath="./Basin-Modelling/development/rpm_ibs_version.spec "
fi

echo "The OS is ${archi} and rpm_version is ${rpm_version}"
pwd
rpmbuild -bb -v  --buildroot=${Build_BinariesDirectory}/rpmbuild \
				 --define "_dbpath ${Build_BinariesDirectory}/rpmdb" \
				 --define "_installpath ${instapath}" \
				 --define "_topdir ${Build_BinariesDirectory}/" \
				 --define "_tmppath %{_topdir}/tmp" \
				 --define "_builddir ${Build_BinariesDirectory}" \
				 --define "_version ${CLDRN_VERSION}" \
				 --define "_Sversion ${SOURCE_CLDRN_VERSION}" \
				 --define "_bldnum ${Build_BuildNumber}" \
				 --define "_archi ${archi}" \
				 --define "_rpm_version ${rpm_version}" \
				 --define "_rpmdir ${Build_BinariesDirectory}/RPMS" \
				 --define '_os anyos' \
				 --define '_cpu x86_64' \
				 --define "_distb ${distb}" \
				${specPath}
###
