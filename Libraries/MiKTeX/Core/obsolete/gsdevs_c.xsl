<?xml version='1.0'?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                xmlns:fo="http://www.w3.org/1999/XSL/Format"
                version='1.0'>

<!-- __________________________________________________________________

     gsdevs_c.xsl: catalog of Ghostscript devices

     Copyright (C) 2002-2016 Christian Schenk

     This file is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published
     by the Free Software Foundation; either version 2, or (at your
     option) any later version.
     
     This file is distributed in the hope that it will be useful, but
     WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     General Public License for more details.
     
     You should have received a copy of the GNU General Public License
     along with this file; if not, write to the Free Software
     Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
     USA.

     __________________________________________________________________ -->

<xsl:output method="text" encoding="iso-8859-1"/>

<xsl:template match="ghostscript-devices">
  <xsl:text>GSDEVINFO const l_gsdevs[] = {
  </xsl:text>
  <xsl:apply-templates select="device"/>
  <xsl:text>};
  </xsl:text>
</xsl:template>

<xsl:template match="device">
  <xsl:text>{"</xsl:text>
  <xsl:value-of select="mnemonic"/>
  <xsl:text>", "</xsl:text>
  <xsl:value-of select="displayname"/>
  <xsl:text>"},
  </xsl:text>
</xsl:template>

</xsl:stylesheet>
