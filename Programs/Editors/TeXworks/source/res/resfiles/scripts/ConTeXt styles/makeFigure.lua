--[[TeXworksScript
Title: Make Figure (lua)
Description: derived from example by Stefan Löffler & Jonathan Kew
Author: H. Ramm
Version: 0.1
Date: 2024-04-07
Script-Type: standalone
Context: TeXDocument
]]

txt = TW.target.selection
if (txt == nil) then txt = "" end
TW.target.insertText("\\startplacefigure[location={here},reference=fig:,title={}]\n" .. txt .. "\n\\stopplacefigure")
TW.target.selectRange(TW.target.selectionStart - string.len(txt) - 1, string.len(txt))
