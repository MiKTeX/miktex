#!/bin/sh -e

case "$1" in
    remove)
	${MIKTEX_SETUP_EXECUTABLE} factoryreset
	;;
    upgrade)
	${MIKTEX_INITEXMF_EXECUTABLE} --admin --clean
	;;
    failed-upgrade)
	;;
    deconfigure)
	;;
    *)
	;;
esac
