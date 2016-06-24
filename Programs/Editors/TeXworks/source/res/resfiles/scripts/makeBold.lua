--[[TeXworksScript
Title: Make Bold (lua)
Description: for demonstration only
Author: Stefan Löffler & Jonathan Kew
Version: 0.3
Date: 2010-01-09
Script-Type: standalone
Context: TeXDocument
]]

txt = TW.target.selection
if (txt == nil) then txt = "" end
TW.target.insertText("\\textbf{" .. txt .. "}")
TW.target.selectRange(TW.target.selectionStart - string.len(txt) - 1, string.len(txt))

