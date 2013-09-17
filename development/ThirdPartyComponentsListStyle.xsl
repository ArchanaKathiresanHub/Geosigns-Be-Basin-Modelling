<?xml version="1.0"?>

<xsl:stylesheet version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="html"/>

<xsl:template match="/">
<html><head><title>Cauldron's Third Party Component List</title></head>
<body><h1>Cauldron's Third Party Component List</h1>
<xsl:apply-templates select="componentlist" mode="Overview"/>
<hr/>
<xsl:apply-templates select="componentlist" mode="Detail"/>
</body>
</html>
</xsl:template>


<!-- Templates that create an overview in a table -->
<xsl:template mode="Overview" match="componentlist">
<h2>Overview</h2>
<table>
<tr>
  <th align="left">Vendor</th>
  <th align="left">Name</th>
  <th align="left">Version</th>
  <th align="left">License</th>
  <th align="left">Dependency</th>
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
<td><xsl:value-of select="dependency"/></td>
</tr>
</xsl:template>

<!-- Templates that generate detailed info for every component -->
<xsl:template mode="Detail" match="componentlist">
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

<p><h4>Dependency</h4>
<xsl:value-of select="dependency"/>
</p>
<p><h4>License - <xsl:value-of select="license/type"/></h4>
<pre><xsl:value-of select="license/contents"/></pre>
</p>
<hr/>
</xsl:template>

</xsl:stylesheet>
