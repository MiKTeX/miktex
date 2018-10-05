@echo off

set headers=source/fontconfig/fontconfig.h 
set headers=%headers% source/fontconfig/fcfreetype.h
set headers=%headers% source/fontconfig/fcprivate.h

echo EXPORTS

( cat %headers% & echo ( ) ^
  | grep '^^Fc[^^ ]* *(' ^
  | sed -e 's/ *(.*$//' -e 's/^^/	/' ^
  | sort
