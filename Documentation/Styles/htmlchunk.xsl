<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version='1.0'>

  <xsl:import href="http://docbook.sourceforge.net/release/xsl/current/html/chunk.xsl"/>

  <xsl:param name="admon.graphics" select="1"/>
  <xsl:param name="graphic.default.extension">png</xsl:param>
  <xsl:param name="html.stylesheet">miktexman.css</xsl:param>
  <xsl:param name="navig.graphics" select="1"/>
  <xsl:param name="navig.graphics.extension" select="'.png'"/>
  <xsl:param name="navig.showtitles" select="0"/>
  <xsl:param name="suppress.footer.navigation" select="1"/>
  <xsl:param name="use.id.as.filename" select="1"/>
  <xsl:param name="chunker.output.encoding">UTF-8</xsl:param>

  <!-- ==================================================================== -->
  
  <xsl:template name="header.navigation">
    <xsl:param name="prev" select="/foo"/>
    <xsl:param name="next" select="/foo"/>
    <xsl:param name="nav.context"/>
    
    <xsl:variable name="up" select="parent::*"/>
    
    <xsl:if test="$suppress.navigation = '0' and $suppress.header.navigation = '0'">
      <nav>
	<ul class="pager">
	  <li>
	    <xsl:if test="count($prev)=0">
	      <xsl:attribute name="class">
		disabled
	      </xsl:attribute>
	    </xsl:if>
	    <a aria-label="Prev">
	      <xsl:attribute name="href">
		<xsl:call-template name="href.target">
                  <xsl:with-param name="object" select="$prev"/>
		</xsl:call-template>
              </xsl:attribute>
              <span aria-hidden="true">Prev</span>
	    </a>
	  </li>
	  <li>
	    <xsl:if test="count($up)=0">
	      <xsl:attribute name="class">
		disabled
	      </xsl:attribute>
	    </xsl:if>
	    <a aria-label="Up">
	      <xsl:attribute name="href">
		<xsl:call-template name="href.target">
                  <xsl:with-param name="object" select="$up"/>
		</xsl:call-template>
              </xsl:attribute>
              <span aria-hidden="true">Up</span>
	    </a>
	  </li>
	  <li>
	    <xsl:if test="count($next)=0">
	      <xsl:attribute name="class">
		disabled
	      </xsl:attribute>
	    </xsl:if>
	    <a aria-label="Next">
	      <xsl:attribute name="href">
		<xsl:call-template name="href.target">
                  <xsl:with-param name="object" select="$next"/>
		</xsl:call-template>
              </xsl:attribute>
              <span aria-hidden="true">Next</span>
	    </a>
	  </li>
	</ul>
      </nav>
    </xsl:if>
  </xsl:template>
  
</xsl:stylesheet>
