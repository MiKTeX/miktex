remove_cxx_flag("/O2")
add_cxx_flag("/Od")

set(function_count 344)
foreach(func RANGE 1 ${function_count})
  set(tex${func}_dll_name "miktex209-tex${func}")
  list(APPEND ${tex${func}_dll_name}_sources
    ${CMAKE_CURRENT_BINARY_DIR}/tex_pool.cpp
  )
  set(tex${func}_web_file ${CMAKE_CURRENT_SOURCE_DIR}/source/tex.web)
  set(tex${func}_include_file ${CMAKE_CURRENT_SOURCE_DIR}/miktex-tex.h)
  set(tex${func}_change_file ${CMAKE_CURRENT_BINARY_DIR}/tex-miktex.ch)
  create_web_app(TeX${func})
  target_compile_definitions(${tex${func}_dll_name}
    PRIVATE
      -DMIKTEX_TEX
      -DTEXAPP=g_TEX${func}App
      -DTEXCLASS=TEX${func}
      -DC4P_OPT_${func}="gty"
  )
  target_link_libraries(${tex${func}_dll_name}
    PRIVATE
      ${w2cemu_dll_name}
  )
endforeach()
