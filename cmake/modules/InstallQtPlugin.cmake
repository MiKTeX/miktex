# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

# stibizt: https://github.com/Kitware/CMake/blob/master/Source/QtDialog/CMakeLists.txt

macro(install__plugin _qt_plugin_name _qt_plugins_var)
    get_target_property(_qt_plugin_path "${_qt_plugin_name}" LOCATION)
    if(EXISTS "${_qt_plugin_path}")
        get_filename_component(_qt_plugin_file "${_qt_plugin_path}" NAME)
        get_filename_component(_qt_plugin_type "${_qt_plugin_path}" PATH)
        get_filename_component(_qt_plugin_type "${_qt_plugin_type}" NAME)
        if(APPLE)
            set(_qt_plugin_dir "${MIKTEX_MACOS_BUNDLE_DIR}/PlugIns")
        elseif(WIN32)
            set(_qt_plugin_dir "plugins")
        endif()
        set(_qt_plugin_dest "${_qt_plugin_dir}/${_qt_plugin_type}")
            install(FILES "${_qt_plugin_path}"
            DESTINATION "${_qt_plugin_dest}"
            ${COMPONENT}
        )
        set(${_qt_plugins_var}
            "${${_qt_plugins_var}};\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/${_qt_plugin_dest}/${_qt_plugin_file}"
        )
    else()
        message(FATAL_ERROR "QT plugin ${_qt_plugin_name} not found")
    endif()
endmacro()
