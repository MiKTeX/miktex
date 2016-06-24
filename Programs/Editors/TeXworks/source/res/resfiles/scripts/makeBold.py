# TeXworksScript
# Title: Make Bold (python)
# Description: for demonstration only
# Author: Stefan Löffler & Jonathan Kew
# Version: 0.3
# Date: 2010-01-09
# Script-Type: standalone
# Context: TeXDocument

txt = TW.target.selection
if txt == None:
	txt = ""
TW.target.insertText("\\textbf{" + txt + "}")
TW.target.selectRange(TW.target.selectionStart - len(txt) - 1, len(txt))
