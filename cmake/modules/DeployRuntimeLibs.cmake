## DeployRuntimeLibs.cmake
##
## Copyright (C) 2026 MV25
##
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

# Stage the Qt runtime DLLs and plugins into the sandbox binary directory.
# On macOS this is done by fixup_bundle() at install time; on Windows
# nothing equivalent ran, so freshly configured builds produced
# Qt-dependent executables that could not start (missing Qt6*.dll).
function(miktex_setup_runtime_deployment)
	if(NOT MIKTEX_NATIVE_WINDOWS)
		return()
	endif()
	set(bin_dir "${MIKTEX_SANDBOX_BINARY_DIR}/${MIKTEX_REL_MIKTEX_BIN_DIR}")
	if(WITH_UI_QT AND TARGET Qt${QT_SERIES}::Core)
		set(qt_libs Core Gui Widgets Network Svg Concurrent PrintSupport Qml UiTools Xml)
		if(QT_SERIES EQUAL 6)
			list(APPEND qt_libs Core5Compat OpenGL OpenGLWidgets)
		endif()
		if(QT_SERIES EQUAL 5)
			list(APPEND qt_libs WinExtras)
		endif()
		set(deploy_commands)
		foreach(m IN LISTS qt_libs)
			if(TARGET Qt${QT_SERIES}::${m})
				list(APPEND deploy_commands
					COMMAND ${CMAKE_COMMAND} -E copy_if_different
						"$<TARGET_FILE:Qt${QT_SERIES}::${m}>"
						"${bin_dir}/"
				)
			endif()
		endforeach()
		# plugin categories live under the Qt kit's plugins root, next to the
		# lib directory where the Qt6::Gui import library resides
		set(qt_gui_loc)
		foreach(cfg IMPORTED_LOCATION_RELEASE IMPORTED_LOCATION)
			get_target_property(qt_gui_loc Qt${QT_SERIES}::Gui ${cfg})
			if(qt_gui_loc)
				break()
			endif()
		endforeach()
		if(qt_gui_loc AND NOT qt_gui_loc MATCHES "NOTFOUND")
			get_filename_component(qt_root "${qt_gui_loc}" DIRECTORY)
			get_filename_component(qt_root "${qt_root}/.." ABSOLUTE)
			foreach(d platforms imageformats iconengines generic)
				if(EXISTS "${qt_root}/plugins/${d}")
					list(APPEND deploy_commands
						COMMAND ${CMAKE_COMMAND} -E make_directory
							"${bin_dir}/${d}"
					)
					# release plugins only: a release build never loads the
					# debug variants, and they pull in the debug Qt DLLs.
					# a plugin is a debug variant when dropping the trailing
					# "d" yields the release plugin's file name (this keeps
					# release plugins that merely end in "d", e.g. qdirect2d)
					file(GLOB plugin_dlls "${qt_root}/plugins/${d}/*.dll")
					foreach(pdll IN LISTS plugin_dlls)
						get_filename_component(pstem "${pdll}" NAME_WE)
						if(pstem MATCHES "d$")
							string(REGEX REPLACE "d$" "" pbase "${pstem}")
							if(EXISTS "${qt_root}/plugins/${d}/${pbase}.dll")
								continue()
							endif()
						endif()
						list(APPEND deploy_commands
							COMMAND ${CMAKE_COMMAND} -E copy_if_different
								"${pdll}"
								"${bin_dir}/${d}/"
						)
					endforeach()
				endif()
			endforeach()
		endif()
		add_custom_target(deploy_qt_runtime_windows
			${deploy_commands}
			COMMENT "Deploying Qt runtime to the sandbox"
			VERBATIM
		)
	endif()
	if(USE_SYSTEM_ICU AND TARGET ICU::uc)
		set(icu_uc_lib)
		foreach(cfg IMPORTED_LOCATION_RELEASE IMPORTED_LOCATION)
			get_target_property(icu_uc_lib ICU::uc ${cfg})
			if(icu_uc_lib)
				break()
			endif()
		endforeach()
		if(icu_uc_lib AND NOT icu_uc_lib MATCHES "NOTFOUND")
			get_filename_component(icu_bin_dir "${icu_uc_lib}" DIRECTORY)
			get_filename_component(icu_bin_dir "${icu_bin_dir}/../bin" ABSOLUTE)
			file(GLOB icu_dlls "${icu_bin_dir}/icu*.dll")
			set(deploy_commands)
			foreach(d IN LISTS icu_dlls)
				list(APPEND deploy_commands
					COMMAND ${CMAKE_COMMAND} -E copy_if_different
						"${d}"
						"${bin_dir}/"
				)
			endforeach()
			add_custom_target(deploy_icu_runtime_windows
				${deploy_commands}
				COMMENT "Deploying ICU runtime to the sandbox"
				VERBATIM
			)
		endif()
	endif()
endfunction()

# Attach the runtime deployment to a Qt UI executable.
function(miktex_deploy_qt target)
	if(NOT MIKTEX_NATIVE_WINDOWS OR NOT WITH_UI_QT)
		return()
	endif()
	if(TARGET deploy_qt_runtime_windows)
		add_dependencies(${target} deploy_qt_runtime_windows)
	endif()
endfunction()

# Attach the runtime deployment to an executable that links the system ICU.
function(miktex_deploy_icu target)
	if(NOT MIKTEX_NATIVE_WINDOWS OR NOT USE_SYSTEM_ICU)
		return()
	endif()
	if(TARGET deploy_icu_runtime_windows)
		add_dependencies(${target} deploy_icu_runtime_windows)
	endif()
endfunction()
