<?xml version="1.0"?>
<package >
  <metadata>
  <id>@PKG_NAMESPACE@.@PKG_NAME@.@BLD_ARCH@</id>
    <version>@PKG_VERSION@</version>
    <authors>@PKG_OWNER@</authors>
    <owners>@PKG_OWNER@</owners>
    <projectUrl>@NUGET_VSTS_FEED_URL@</projectUrl>
    <iconUrl>file://@PKG_NAME@.png</iconUrl>
    <requireLicenseAcceptance>false</requireLicenseAcceptance>
    <description>@PKG_DESCRIPTION@</description>
    <releaseNotes>@PKG_RELEASE_NOTES@</releaseNotes>
    <copyright>@PKG_COPYRIGHT@</copyright>
    <tags>@PKG_TAGS@</tags>
    <references>
      <!-- <reference file="SampleReference.dll" /> -->
    </references>
    <dependencies>
      <!-- <dependency id="SampleDependency" version="1.0" /> -->
    </dependencies>
  </metadata>
  <files>  
    <!-- C++ native library -->
@PKG_LIBS@
    <!-- C++ library includes -->
@PKG_HEADERS@ 
  </files>
</package>
