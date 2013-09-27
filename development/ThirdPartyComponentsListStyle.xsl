<?xml version="1.0"?>

<xsl:stylesheet version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="html"/>

<xsl:template match="/">
<html>
<head>
<title><xsl:value-of select="release-info/name"/> - 
   <xsl:value-of select="release-info/version"/> -
   Third Party Component List
</title>
</head>

<body>
<h1><xsl:value-of select="release-info/name"/> - 
   <xsl:value-of select="release-info/version"/> -  Third Party Component List
</h1>
<xsl:apply-templates select="release-info/third-party" mode="Overview"/>
<hr/>
<xsl:apply-templates select="release-info/third-party" mode="Detail"/>
</body>
</html>
</xsl:template>


<!-- Templates that create an overview in a table -->
<xsl:template mode="Overview" match="third-party">
<h2>Overview</h2>
<table rules="rows">
<tr>
  <th align="left">Vendor</th>
  <th align="left">Name</th>
  <th align="left">Version</th>
  <th align="left">License</th>
  <th align="left">Required At</th>
  <th align="left">Origin Country</th>
  <th align="left">Shipped</th>
  <th align="left">Inclusion</th>
  <th align="left">Useable stand-alone</th>
  <th align="left">Crypto</th>
  <th align="left">ECCN</th>
</tr>
<xsl:apply-templates select="component" mode="Overview"/>
</table>
</xsl:template>

<xsl:template mode="Overview" match="component">
<tr>
<td><xsl:value-of select="vendor"/></td>
<td><a><xsl:attribute name="href">#<xsl:value-of select="name"/></xsl:attribute><xsl:value-of select="name"/></a></td>
<td><xsl:value-of select="version"/></td>
<td><xsl:value-of select="license/type"/></td>
<td><xsl:value-of select="required/at"/></td>
<td><xsl:value-of select="export-control/country-of-origin"/></td>
<td><xsl:value-of select="export-control/shipped"/></td>
<td><xsl:value-of select="export-control/inclusion-type"/></td>
<td><xsl:value-of select="export-control/stand-alone"/></td>
<td><xsl:value-of select="export-control/crypto"/></td>
<td><xsl:value-of select="export-control/eccn"/></td>
</tr>
</xsl:template>

<!-- Templates that generate detailed info for every component -->
<xsl:template mode="Detail" match="third-party">
<h2>Detail</h2>
<xsl:apply-templates mode="Detail" select="component"/>
</xsl:template>

<xsl:template mode="Detail" match="component">
<h3><xsl:value-of select="vendor"/> - 
<a><xsl:attribute name="name"><xsl:value-of select="name"/></xsl:attribute><xsl:value-of select="name"/></a>
- <xsl:value-of select="version"/> </h3>

<p><h4>Description</h4>
<xsl:value-of select="description"/></p>

<p><h4>URL</h4>
<a><xsl:attribute name="href"><xsl:value-of select="url"/></xsl:attribute><xsl:value-of select="url"/></a>
</p>

<p><h4>Dependency Info</h4>
This capability <strong><xsl:value-of select="required/id"/></strong> is
required at <strong><xsl:value-of select="required/at"/></strong>
<xsl:if test="required/by">
  and used by capability
  <xsl:for-each select="required/by">
    <strong><xsl:value-of select="child::text()"/></strong>
  </xsl:for-each>
</xsl:if>
</p>
<p><h4>Export Control Classification</h4>
<table rules="rows">
<tr><td>Country of origin</td><td><xsl:value-of select="export-control/country-of-origin"/></td></tr>
<tr><td>Is this package shipped with the release?</td><td><xsl:value-of select="export-control/shipped"/></td></tr>
<tr><td>If shipped, how is the component connected to the application?</td><td><xsl:value-of select="export-control/inclusion-type"/></td></tr>
<tr><td>As delivered with this application, does the component offer standalone functionality that could be used separately from your application</td><td><xsl:value-of select="export-control/stand-alone"/></td></tr>
<tr><td>Does the component contain or perform encryption or decryption?</td><td><xsl:value-of select="export-control/crypto"/></td></tr>
<tr><td>ECCN</td><td><xsl:value-of select="export-control/eccn"/></td></tr>
<xsl:for-each select="export-control/eccn-evidence">
    <tr><td>See also</td><td><a><xsl:attribute name="href"><xsl:value-of select="child::text()"/></xsl:attribute><xsl:value-of select="child::text()"/></a></td></tr>
</xsl:for-each>
</table>
</p>
<p><h4>License - <xsl:value-of select="license/type"/></h4>
<pre><xsl:value-of select="license/contents"/></pre>
</p>
<hr/>
</xsl:template>

</xsl:stylesheet>
