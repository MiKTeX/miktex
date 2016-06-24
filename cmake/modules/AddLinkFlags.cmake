## AddLinkFlags.cmake
##
## Copyright (c) 2006, Oswald Buddenhagen, <ossi@kde.org>
##
## Redistribution and use is allowed according to the terms of the BSD license.
## For details see the accompanying COPYING-CMAKE-SCRIPTS file.

macro(add_link_flags _target _link_flags_to_be_added)
  get_target_property(_link_flags ${_target} LINK_FLAGS)
  if (_link_flags)
    set(_link_flags "${_link_flags} ${_link_flags_to_be_added}")
  else(_link_flags)
    set(_link_flags "${_link_flags_to_be_added}")
  endif(_link_flags)
  set_target_properties(${_target} PROPERTIES LINK_FLAGS "${_link_flags}")
endmacro(add_link_flags)
