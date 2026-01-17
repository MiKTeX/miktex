## dyn.sed
##
## Copyright (C) 1991-2026 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.
s/^\([_A-Za-z0-9]*\) \(fontflags\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(fontlayoutengine\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(fontletterspace\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
s/^\([_A-Za-z0-9]*\) \(fontmapping\)\[ [A-Za-z0-9]* \];$/\1 *\2;/
