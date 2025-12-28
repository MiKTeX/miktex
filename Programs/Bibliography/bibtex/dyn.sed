## dyn.sed
##
## Copyright (C) 1991-2025 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

s/^\([_A-Za-z0-9]*\) \(bibfile\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(biblist\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(citeinfo\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(citelist\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(entryexists\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(entryints\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(entrystrs\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(fieldinfo\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(fntype\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(glbstrend\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(glbstrptr\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(glbstrstr\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(globalstrs\)\[ [0-9]* \]\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(hashilk\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(hashnext\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(hashtext\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(ilkinfo\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(litstack\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(litstktype\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(nameoffile\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(namesepchar\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(nametok\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(singlfunction\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(spreamble\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(strpool\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(strstart\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(typelist\)\[ [0-9]* \];$/\1* \2;/
s/^\([_A-Za-z0-9]*\) \(wizfunctions\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^typedef \([_A-Za-z0-9]*\) \(buftype\)\[ [0-9]* \];$/typedef \1 *\2;/
