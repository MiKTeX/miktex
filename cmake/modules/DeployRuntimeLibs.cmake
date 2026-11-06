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
# The copies run in one shared custom target so that parallel target
# builds never write to the same file concurrently.
function(miktex_deploy_qt target)
	if(NOT MIKTEX_NATIVE_WINDOWS OR NOT WITH_UI_QT)
		return()
	endif()
	if(NOT TARGET deploy_qt_runtime_windows)
		set(bin_dir "${MIKTEX_SANDBOX_BINARY_DIR}/${MIKTEX_REL_MIKTEX_BIN_DIR}")
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
						COMMAND ${CMAKE_COMMAND} -E copy_directory
							"${qt_root}/plugins/${d}"
							"${bin_dir}/${d}"
					)
				endif()
			endforeach()
		endif()
		add_custom_target(deploy_qt_runtime_windows
			${deploy_commands}
			COMMENT "Deploying Qt runtime to the sandbox"
			VERBATIM
		)
	endif()
	add_dependencies(${target} deploy_qt_runtime_windows)
endfunction()

# Stage the system ICU runtime DLLs. xetex and friends always link against
# the system ICU (USE_SYSTEM_ICU is unconditional), so the executables
# import icuuc<version>.dll but nothing in the build copied them into the
# sandbox on Windows.
function(miktex_deploy_icu target)
	if(NOT MIKTEX_NATIVE_WINDOWS OR NOT USE_SYSTEM_ICU OR NOT TARGET ICU::uc)
		return()
	endif()
	if(NOT TARGET deploy_icu_runtime_windows)
		set(bin_dir "${MIKTEX_SANDBOX_BINARY_DIR}/${MIKTEX_REL_MIKTEX_BIN_DIR}")
		set(icu_uc_lib)
		foreach(cfg IMPORTED_LOCATION_RELEASE IMPORTED_LOCATION)
			get_target_property(icu_uc_lib ICU::uc ${cfg})
			if(icu_uc_lib)
				break()
			endif()
		endforeach()
		if(NOT icu_uc_lib OR icu_uc_lib MATCHES "NOTFOUND")
			return()
		endif()
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
	add_dependencies(${target} deploy_icu_runtime_windows)
endfunction()
