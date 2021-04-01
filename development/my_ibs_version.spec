#### 	Modified by			Date				Reason
####	Arijit Khan 		27th March 2021		Indroduction	
#============================================================	
#	From rpmbuild command pass
# 		1. _version
# 		2. _rpm_version 
#		3. _os
#		4. _cpu
#		5. _builddir  =%{_installpath}/ibs
#============================================================
%global __os_install_post %{nil}
%define _sssname ibs
#%define __prelink_undo_cmd %{nil}

Provides:		%{_distb}-%{_sssname} = %{version}
Summary: 		IBS installation
Name: 			%{_distb}-%{_sssname}-%{_version}
Version: 		%{_version}
Release: 		%{_rpm_version}
License: 		FlxLm
Group: 			Applications/Shell/Subsurface/%{_sssname}
URL: 			arijit.khan@shell.com
Distribution: 	%{_distb} 1.0
Packager: 		Zulu
AutoReqProv: 	no
Prefix: 		%{_installpath}
BuildRoot:		%{buildroot}
BuildArch: 		%{_cpu}

%define cpu_x86_64          %([ %{_cpu} = "x86_64"          ] && echo 1 || echo 0)

%define os_anyos            %([ %{_os} =  "anyos"           ] && echo 1 || echo 0)
%define os_linuxrhel_74ws   %([ %{_os} =  "linuxrhel_74ws"  ] && echo 1 || echo 0)

%define _rpmfilename %{_os}/%%{NAME}-%%{RELEASE}.rpm
%define _vvers %(echo "v%{_version}" | sed -e "s#\\.#_#g")
%define _target_suffix %{_os}-%{_cpu}

%define RPMFiles $RPM_BUILD_DIR/%{name}.files
%define target %{buildroot}%{_installpath}/%{_sssname}/%{DVersion}


%description
The ibs v%{_version} software
%if %{os_anyos}
%package common-%{_target_suffix}
Provides: %{_distb}-%{_sssname} = %{version}
Summary: Platform independent, version specific, %{_sssname} files and directories for all platforms.
Group: Applications/Shell/Subsurface/%{_sssname}
AutoReqProv: no
Requires: %{_distb}-%{_sssname}-base
%description common-%{_target_suffix}
The common, version specific, %{_sssname} files and directories for all platforms.
#%package documentation-%{_target_suffix}
#Provides: %{_distb}-%{_sssname} = %{version}
#Summary: The IBS, version specific, user documentation.
#Group: Applications/Shell/Subsurface/%{_sssname}
#AutoReqProv: no
#Requires: %{name}-common-anyos-noarch
#%description documentation-%{_target_suffix}
#The ibs, version specific, user documentation.
%else
%package %{_target_suffix}
Provides: %{_distb}-%{_sssname} = %{version}
Summary: %{_os} version of %{_sssname}
Group: Applications/Shell/Subsurface/%{_sssname}
AutoReqProv: no
Requires: %{name}-common-anyos-noarch
%description %{_target_suffix}
The %{_version} version of %{_sssname} for %{_os}
%endif

%prep

%build

%install
# make sure to start clean
rm -rf %{buildroot}
# WARNING: Make sure you use the correct destination version directory.
# SVersion is the source version directory
# DVersion is the destination version directory
# SVersion and DVersion can differ. Please check carefully!!!
# This can be: %{_vvers} or %{_version} or v%{_version}
%define SVersion v%{_version}
## The VERSION name remanes same as in /sssden/ibs with a appended _rc
%define DVersion v%{_version}_rc
echo "INSTALL STEP of RPM Build"
# if there are additional subdirectories that are not copied entirely, they should be created 
# here as well.
echo "The Build Root is $RPM_BUILD_ROOT"
echo %{buildroot}
if [[ ! -d "%{buildroot}%{_installpath}/%{_sssname}/%{DVersion}" ]];then		#&& mkdir -p %{buildroot}%{_installpath}/%{_sssname}/%{DVersion}
	mkdir -p %{buildroot}%{_installpath}/%{_sssname}/%{DVersion} 2>/dev/null || { echo "error in creating dir"; exit; }
fi

%if %{os_anyos}
   %define ToCopy misc LinuxRHEL64 %{architecture}
%else
   echo "I dont know what just hit me!"
%endif
> %{RPMFiles}

for FINFO in %{ToCopy}
do
   if [ -r /apps/sssdev/%{_sssname}/%{SVersion}/${FINFO} ]
   then
      if [ -d /apps/sssdev/%{_sssname}/%{SVersion}/${FINFO} ]
      then
         DESTDIR=`dirname %{target}/${FINFO}`
         [ ! -d ${DESTDIR} ] && mkdir -p ${DESTDIR}
		 echo "****************************************"
         cp -a /apps/sssdev/%{_sssname}/%{SVersion}/${FINFO} ${DESTDIR}
      else
         cp -a /apps/sssdev/%{_sssname}/%{SVersion}/${FINFO} %{buildroot}%{_installpath}/%{_sssname}/%{DVersion}/${FINFO}
		 echo "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^"
      fi
      echo "%{buildroot}%{_installpath}/%{_sssname}/%{DVersion}/${FINFO}" >> %{RPMFiles}
   fi
done

## These is the standard from GAD team
# %if %{os_anyos}
# %files common-%{_target_suffix} -f %{name}.files
# %defattr(755,s_epwa00,g_epwa00,g_psaz00,g_bpacgi)
# %exclude /%{name}.files
# %else
# %files %{_target_suffix} -f %{name}.files
# %defattr(755,s_epwa00,g_epwa00,g_psaz00,g_bpacgi)
# %exclude /%{name}.files
# %endif

%files
%defattr(-,root,root,-)
%config(noreplace) %{_installpath}/%{_sssname}/v%{_version}_rc/*
#%defattr(-,g_psaz00,g_bpacgi)

%clean
## These is the standard from GAD team
#chmod -R u+w %{_installpath}/%{_sssname}
#rm -rf _installpath/%{_sssname}

## These is the standard from GAD team
%changelog
# * Thurs May 09 2013 Owner
# - 1.0 r1 First release
