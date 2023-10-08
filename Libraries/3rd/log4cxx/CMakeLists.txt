## CMakeLists.txt
##
## Copyright (C) 2015-2023 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

include(component.cmake)

set(MIKTEX_CURRENT_FOLDER "${MIKTEX_IDE_3RD_LIBRARIES_FOLDER}/log4cxx")

if(MIKTEX_NATIVE_WINDOWS)
    add_definitions(
        -DWINNT
    )
endif()

set(LOG4CXX_CHAR "utf-8")
set(LOGCHAR_IS_UNICHAR 0)
set(LOGCHAR_IS_WCHAR 0)
set(LOGCHAR_IS_UTF8 1)

set(CFSTRING_API 0)
set(CHARSET_EBCDIC 0)
set(CHARSET_ISO88591 0)
set(CHARSET_USASCII 0)
set(CHARSET_UTF8 1)
set(CHAR_API 1)
set(HAS_FWIDE 0)
set(HAS_LIBESMTP 0)
set(HAS_MBSRTOWCS 0)
set(HAS_ODBC 0)
set(HAS_PTHREAD_GETNAME 0)
set(HAS_PTHREAD_SETNAME 0)
set(HAS_PTHREAD_SIGMASK 0)
set(HAS_STD_LOCALE 1)
set(HAS_SYSLOG 0)
set(HAS_THREAD_LOCAL 0)
set(HAS_WCSTOMBS 0)
set(INIT_IOS_BASE 0)
set(MULTIPROCESS_RFA 0)
set(NETWORKING_SUPPORT 1)
set(STD_MAKE_UNIQUE_FOUND 1)
set(UNICHAR_API 0)
set(WCHAR_T_API 1)

include_directories(BEFORE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}/privateinclude
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/include
)

set(public_include_directories
    ${CMAKE_CURRENT_BINARY_DIR}/include
)

# log4cxx/
set(public_headers
    log4cxx/appender.h
    log4cxx/appenderskeleton.h
    log4cxx/asyncappender.h
    log4cxx/basicconfigurator.h
    log4cxx/config/propertysetter.h
    log4cxx/consoleappender.h
    log4cxx/db/dbappender.h
    log4cxx/db/odbcappender.h
    log4cxx/defaultconfigurator.h
    log4cxx/defaultloggerfactory.h
    log4cxx/file.h
    log4cxx/fileappender.h
    log4cxx/filter/andfilter.h
    log4cxx/filter/denyallfilter.h
    log4cxx/filter/expressionfilter.h
    log4cxx/filter/levelmatchfilter.h
    log4cxx/filter/levelrangefilter.h
    log4cxx/filter/locationinfofilter.h
    log4cxx/filter/loggermatchfilter.h
    log4cxx/filter/mapfilter.h
    log4cxx/filter/propertyfilter.h
    log4cxx/filter/stringmatchfilter.h
    log4cxx/fmtlayout.h
    log4cxx/helpers/absolutetimedateformat.h
    log4cxx/helpers/appenderattachableimpl.h
    log4cxx/helpers/aprinitializer.h
    log4cxx/helpers/bufferedwriter.h
    log4cxx/helpers/bytearrayinputstream.h
    log4cxx/helpers/bytearrayoutputstream.h
    log4cxx/helpers/bytebuffer.h
    log4cxx/helpers/cacheddateformat.h
    log4cxx/helpers/charsetdecoder.h
    log4cxx/helpers/charsetencoder.h
    log4cxx/helpers/class.h
    log4cxx/helpers/classregistration.h
    log4cxx/helpers/cyclicbuffer.h
    log4cxx/helpers/datagrampacket.h
    log4cxx/helpers/datagramsocket.h
    log4cxx/helpers/date.h
    log4cxx/helpers/dateformat.h
    log4cxx/helpers/datetimedateformat.h
    log4cxx/helpers/exception.h
    log4cxx/helpers/fileinputstream.h
    log4cxx/helpers/fileoutputstream.h
    log4cxx/helpers/filewatchdog.h
    log4cxx/helpers/inetaddress.h
    log4cxx/helpers/inputstream.h
    log4cxx/helpers/inputstreamreader.h
    log4cxx/helpers/integer.h
    log4cxx/helpers/iso8601dateformat.h
    log4cxx/helpers/loader.h
    log4cxx/helpers/locale.h
    log4cxx/helpers/loglog.h
    log4cxx/helpers/messagebuffer.h
    log4cxx/helpers/object.h
    log4cxx/helpers/onlyonceerrorhandler.h
    log4cxx/helpers/optionconverter.h
    log4cxx/helpers/outputstream.h
    log4cxx/helpers/outputstreamwriter.h
    log4cxx/helpers/pool.h
    log4cxx/helpers/properties.h
    log4cxx/helpers/propertyresourcebundle.h
    log4cxx/helpers/reader.h
    log4cxx/helpers/relativetimedateformat.h
    log4cxx/helpers/resourcebundle.h
    log4cxx/helpers/serversocket.h
    log4cxx/helpers/simpledateformat.h
    log4cxx/helpers/socket.h
    log4cxx/helpers/socketoutputstream.h
    log4cxx/helpers/strftimedateformat.h
    log4cxx/helpers/strictmath.h
    log4cxx/helpers/stringhelper.h
    log4cxx/helpers/stringtokenizer.h
    log4cxx/helpers/syslogwriter.h
    log4cxx/helpers/system.h
    log4cxx/helpers/systemerrwriter.h
    log4cxx/helpers/systemoutwriter.h
    log4cxx/helpers/tchar.h
    log4cxx/helpers/threadlocal.h
    log4cxx/helpers/threadspecificdata.h
    log4cxx/helpers/threadutility.h
    log4cxx/helpers/timezone.h
    log4cxx/helpers/transcoder.h
    log4cxx/helpers/transform.h
    log4cxx/helpers/writer.h
    log4cxx/helpers/xml.h
    log4cxx/hierarchy.h
    log4cxx/htmllayout.h
    log4cxx/jsonlayout.h
    log4cxx/layout.h
    log4cxx/level.h
    log4cxx/logger.h
    log4cxx/logmanager.h
    log4cxx/logstring.h
    log4cxx/mdc.h
    log4cxx/ndc.h
    log4cxx/net/smtpappender.h
    log4cxx/net/socketappenderskeleton.h
    log4cxx/net/syslogappender.h
    log4cxx/net/telnetappender.h
    log4cxx/net/xmlsocketappender.h
    log4cxx/nt/EventLogCategories.mc
    log4cxx/nt/nteventlogappender.h
    log4cxx/nt/outputdebugstringappender.h
    log4cxx/pattern/classnamepatternconverter.h
    log4cxx/pattern/colorendpatternconverter.h
    log4cxx/pattern/colorstartpatternconverter.h
    log4cxx/pattern/datepatternconverter.h
    log4cxx/pattern/filedatepatternconverter.h
    log4cxx/pattern/filelocationpatternconverter.h
    log4cxx/pattern/formattinginfo.h
    log4cxx/pattern/fulllocationpatternconverter.h
    log4cxx/pattern/integerpatternconverter.h
    log4cxx/pattern/levelpatternconverter.h
    log4cxx/pattern/linelocationpatternconverter.h
    log4cxx/pattern/lineseparatorpatternconverter.h
    log4cxx/pattern/literalpatternconverter.h
    log4cxx/pattern/loggerpatternconverter.h
    log4cxx/pattern/loggingeventpatternconverter.h
    log4cxx/pattern/mdcpatternconverter.h
    log4cxx/pattern/messagepatternconverter.h
    log4cxx/pattern/methodlocationpatternconverter.h
    log4cxx/pattern/nameabbreviator.h
    log4cxx/pattern/namepatternconverter.h
    log4cxx/pattern/ndcpatternconverter.h
    log4cxx/pattern/patternconverter.h
    log4cxx/pattern/patternparser.h
    log4cxx/pattern/propertiespatternconverter.h
    log4cxx/pattern/relativetimepatternconverter.h
    log4cxx/pattern/shortfilelocationpatternconverter.h
    log4cxx/pattern/threadpatternconverter.h
    log4cxx/pattern/threadusernamepatternconverter.h
    log4cxx/pattern/throwableinformationpatternconverter.h
    log4cxx/patternlayout.h
    log4cxx/private/action_priv.h
    log4cxx/private/appenderskeleton_priv.h
    log4cxx/private/aprdatagramsocket.h
    log4cxx/private/aprserversocket.h
    log4cxx/private/aprsocket.h
    log4cxx/private/datagramsocket_priv.h
    log4cxx/private/fileappender_priv.h
    log4cxx/private/filter_priv.h
    log4cxx/private/nteventlogappender_priv.h
    log4cxx/private/odbcappender_priv.h
    log4cxx/private/patternconverter_priv.h
    log4cxx/private/rollingpolicybase_priv.h
    log4cxx/private/serversocket_priv.h
    log4cxx/private/socketappenderskeleton_priv.h
    log4cxx/private/socket_priv.h
    log4cxx/private/string_c11.h
    log4cxx/private/syslogappender_priv.h
    log4cxx/private/writerappender_priv.h
    log4cxx/propertyconfigurator.h
    log4cxx/provisionnode.h
    log4cxx/rolling/action.h
    log4cxx/rolling/filerenameaction.h
    log4cxx/rolling/filterbasedtriggeringpolicy.h
    log4cxx/rolling/fixedwindowrollingpolicy.h
    log4cxx/rolling/gzcompressaction.h
    log4cxx/rolling/manualtriggeringpolicy.h
    log4cxx/rolling/multiprocessrollingfileappender.h
    log4cxx/rolling/rollingfileappender.h
    log4cxx/rolling/rollingpolicy.h
    log4cxx/rolling/rollingpolicybase.h
    log4cxx/rolling/rolloverdescription.h
    log4cxx/rolling/sizebasedtriggeringpolicy.h
    log4cxx/rolling/timebasedrollingpolicy.h
    log4cxx/rolling/triggeringpolicy.h
    log4cxx/rolling/zipcompressaction.h
    log4cxx/simplelayout.h
    log4cxx/spi/appenderattachable.h
    log4cxx/spi/configurator.h
    log4cxx/spi/defaultrepositoryselector.h
    log4cxx/spi/errorhandler.h
    log4cxx/spi/filter.h
    log4cxx/spi/hierarchyeventlistener.h
    log4cxx/spi/location/locationinfo.h
    log4cxx/spi/loggerfactory.h
    log4cxx/spi/loggerrepository.h
    log4cxx/spi/loggingevent.h
    log4cxx/spi/optionhandler.h
    log4cxx/spi/repositoryselector.h
    log4cxx/spi/rootlogger.h
    log4cxx/spi/triggeringeventevaluator.h
    log4cxx/stream.h
    log4cxx/varia/fallbackerrorhandler.h
    log4cxx/writerappender.h
    log4cxx/xml/domconfigurator.h
    log4cxx/xml/xmllayout.h
)

foreach(h ${public_headers})
    configure_file(
        source/src/main/include/${h}
        ${CMAKE_CURRENT_BINARY_DIR}/include/${h}
        COPYONLY
    )
    list(APPEND log4cxx_sources ${CMAKE_CURRENT_BINARY_DIR}/include/${h})
endforeach()

configure_file(
    miktex-log4cxx-version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/miktex-log4cxx-version.h
)

list(APPEND log4cxx_sources ${CMAKE_CURRENT_BINARY_DIR}/miktex-log4cxx-version.h)

configure_file(
    ${CMAKE_CURRENT_BINARY_DIR}/source/src/main/include/log4cxx/log4cxx.hw
    ${CMAKE_CURRENT_BINARY_DIR}/include/log4cxx/log4cxx.h
)

list(APPEND log4cxx_sources ${CMAKE_CURRENT_BINARY_DIR}/include/log4cxx/log4cxx.h)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/include/log4cxx/private/log4cxx_private.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/log4cxx/private/log4cxx_private.h
)

list(APPEND log4cxx_sources ${CMAKE_CURRENT_BINARY_DIR}/include/log4cxx/private/log4cxx_private.h)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cmake/compiler-features/makeunique.h.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/include/log4cxx/helpers/makeunique.h
)

list(APPEND log4cxx_sources ${CMAKE_CURRENT_BINARY_DIR}/log4cxx/helpers/makeunique.h)

# ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/
list(APPEND log4cxx_sources
    # LOG4CXX_NETWORKING_SUPPORT
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/datagrampacket.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/datagramsocket.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/aprdatagramsocket.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/serversocket.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/aprserversocket.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/socket.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/aprsocket.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/telnetappender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/socketappenderskeleton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/socketoutputstream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/xmlsocketappender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/syslogwriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/syslogappender.cpp

    # ENABLE_FMT_LAYOUT
	${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/fmtlayout.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/action.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/andfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/appenderattachableimpl.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/appenderskeleton.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/aprinitializer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/asyncappender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/basicconfigurator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/bufferedwriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/bytearrayinputstream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/bytearrayoutputstream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/bytebuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/cacheddateformat.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/charsetdecoder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/charsetencoder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/class.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/classnamepatternconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/classregistration.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/colorstartpatternconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/colorendpatternconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/configurator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/consoleappender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/cyclicbuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/date.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/dateformat.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/datepatternconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/dbappender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/defaultconfigurator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/defaultloggerfactory.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/defaultrepositoryselector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/domconfigurator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/exception.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/fallbackerrorhandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/file.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/fileappender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/filedatepatternconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/fileinputstream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/filelocationpatternconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/fileoutputstream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/filerenameaction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/filewatchdog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/filter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/filterbasedtriggeringpolicy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/fixedwindowrollingpolicy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/formattinginfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/fulllocationpatternconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/gzcompressaction.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/hierarchy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/htmllayout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/inetaddress.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/inputstream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/inputstreamreader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/integer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/integerpatternconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/jsonlayout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/layout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/level.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/levelmatchfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/levelpatternconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/levelrangefilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/linelocationpatternconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/lineseparatorpatternconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/literalpatternconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/loader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/locale.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/locationinfo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/locationinfofilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/loggermatchfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/loggerpatternconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/loggingevent.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/loggingeventpatternconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/loglog.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/logmanager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/logstream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/manualtriggeringpolicy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/mapfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/mdc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/messagebuffer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/messagepatternconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/methodlocationpatternconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/nameabbreviator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/namepatternconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/ndc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/mdcpatternconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/ndcpatternconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/nteventlogappender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/odbcappender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/onlyonceerrorhandler.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/optionconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/outputdebugstringappender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/outputstream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/outputstreamwriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/patternconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/patternlayout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/patternparser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/pool.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/properties.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/propertiespatternconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/propertyconfigurator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/propertyresourcebundle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/propertysetter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/reader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/relativetimedateformat.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/relativetimepatternconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/resourcebundle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/rollingfileappender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/rollingpolicy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/rollingpolicybase.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/rolloverdescription.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/rootlogger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/shortfilelocationpatternconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/simpledateformat.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/simplelayout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/sizebasedtriggeringpolicy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/smtpappender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/strftimedateformat.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/stringhelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/stringmatchfilter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/stringtokenizer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/system.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/systemerrwriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/systemoutwriter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/threadlocal.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/threadpatternconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/threadusernamepatternconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/threadspecificdata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/threadutility.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/throwableinformationpatternconverter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/timebasedrollingpolicy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/timezone.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/transcoder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/transform.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/triggeringpolicy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/writer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/writerappender.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/xmllayout.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/zipcompressaction.cpp
  )

if(TRUE)
    list(APPEND log4cxx_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/asyncappender.cpp
    )
else()
    list(APPEND log4cxx_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/source/src/main/cpp/asyncappender_nonblocking.cpp
    )
endif()

if(MIKTEX_NATIVE_WINDOWS)
    list(APPEND system_libraries
        ODBC32.LIB
    )
endif()

add_subdirectory(shared)
