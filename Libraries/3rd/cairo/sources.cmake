## sources.cmake                                        -*- CMake -*-
##
## Copyright (C) 2013-2018 Christian Schenk
## 
## This file is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published
## by the Free Software Foundation; either version 2, or (at your
## option) any later version.
## 
## This file is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
## General Public License for more details.
## 
## You should have received a copy of the GNU General Public License
## along with this file; if not, write to the Free Software
## Foundation, 59 Temple Place - Suite 330, Boston, MA 02111-1307,
## USA.

set(cairo_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-deprecated.h
)

set(cairo_private
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairoint.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-analysis-surface-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-arc-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-array-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-atomic-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-backend-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-box-inline.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-boxes-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-cache-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-clip-inline.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-clip-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-combsort-inline.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-compiler-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-compositor-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-contour-inline.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-contour-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-composite-rectangles-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-damage-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-default-context-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-device-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-error-inline.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-error-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-fixed-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-fixed-type-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-freelist-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-freelist-type-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-freed-pool-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-fontconfig-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-gstate-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-hash-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-image-info-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-image-surface-inline.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-image-surface-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-line-inline.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-line-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-list-inline.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-list-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-malloc-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-mempool-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-mutex-impl-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-mutex-list-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-mutex-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-mutex-type-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-output-stream-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-paginated-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-paginated-surface-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-path-fixed-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-path-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-pattern-inline.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-pattern-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-pixman-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-recording-surface-inline.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-recording-surface-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-reference-count-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-region-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-rtree-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-scaled-font-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-slope-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-spans-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-spans-compositor-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-stroke-dash-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-inline.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-backend-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-clipper-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-fallback-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-observer-inline.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-observer-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-offset-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-subsurface-inline.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-subsurface-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-snapshot-inline.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-snapshot-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-wrapper-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-time-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-types-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-traps-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-tristrip-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-user-font-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-wideint-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-wideint-type-private.h
)

set(cairo_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-analysis-surface.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-arc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-array.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-atomic.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-base64-stream.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-base85-stream.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-bentley-ottmann.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-bentley-ottmann-rectangular.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-bentley-ottmann-rectilinear.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-botor-scan-converter.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-boxes.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-boxes-intersect.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-cache.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-clip.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-clip-boxes.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-clip-polygon.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-clip-region.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-clip-surface.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-color.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-composite-rectangles.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-compositor.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-contour.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-damage.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-debug.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-default-context.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-device.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-error.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-fallback-compositor.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-fixed.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-font-face.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-font-face-twin.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-font-face-twin-data.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-font-options.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-freelist.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-freed-pool.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-gstate.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-hash.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-hull.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-image-compositor.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-image-info.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-image-source.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-image-surface.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-line.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-lzw.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-matrix.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-mask-compositor.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-mesh-pattern-rasterizer.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-mempool.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-misc.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-mono-scan-converter.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-mutex.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-no-compositor.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-observer.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-output-stream.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-paginated-surface.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-path-bounds.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-path.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-path-fill.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-path-fixed.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-path-in-fill.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-path-stroke.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-path-stroke-boxes.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-path-stroke-polygon.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-path-stroke-traps.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-path-stroke-tristrip.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-pattern.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-pen.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-polygon.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-polygon-intersect.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-polygon-reduce.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-raster-source-pattern.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-recording-surface.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-rectangle.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-rectangular-scan-converter.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-region.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-rtree.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-scaled-font.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-shape-mask-compositor.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-slope.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-spans.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-spans-compositor.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-spline.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-stroke-dash.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-stroke-style.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-clipper.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-fallback.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-observer.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-offset.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-snapshot.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-subsurface.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-wrapper.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-time.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-tor-scan-converter.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-tor22-scan-converter.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-clip-tor-scan-converter.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-toy-font-face.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-traps.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-tristrip.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-traps-compositor.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-unicode.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-user-font.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-version.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-wideint.c
)

set(_cairo_font_subset_private
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-scaled-font-subsets-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-truetype-subset-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-type1-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-type3-glyph-surface-private.h
)

set(_cairo_font_subset_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-cff-subset.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-scaled-font-subsets.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-truetype-subset.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-type1-fallback.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-type1-glyph-names.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-type1-subset.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-type3-glyph-surface.c
)

set(cairo_private
  ${cairo_private}
  ${_cairo_font_subset_private}
)

set(cairo_sources
  ${cairo_sources}
  ${_cairo_font_subset_sources}
)

#cairo_egl_sources =
#cairo_glx_sources =
#cairo_wgl_sources =

set(_cairo_pdf_operators_private
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-pdf-operators-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-pdf-shading-private.h
)

set(_cairo_pdf_operators_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-pdf-operators.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-pdf-shading.c
)

set(cairo_private
  ${cairo_private}
  ${_cairo_pdf_operators_private}
)

set(cairo_sources
  ${cairo_sources}
  ${_cairo_pdf_operators_sources}
)

set(cairo_png_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-png.c
)

set(cairo_ps_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-ps.h
)

set(cairo_ps_private
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-ps-surface-private.h
)

set(cairo_ps_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-ps-surface.c
)

set(_cairo_deflate_stream_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-deflate-stream.c
)

set(cairo_sources
  ${cairo_sources}
  ${_cairo_deflate_stream_sources}
)

set(cairo_pdf_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-pdf.h
)

set(cairo_pdf_private
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-pdf-surface-private.h
)

set(cairo_pdf_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-pdf-surface.c
)

set(cairo_svg_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-svg.h
)

set(cairo_svg_private
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-svg-surface-private.h
)

set(cairo_svg_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-svg-surface.c
)

set(cairo_ft_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-ft.h
)

set(cairo_ft_private
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-ft-private.h
)

set(cairo_ft_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-ft-font.c
)

set(cairo_test_surfaces_private
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/test-compositor-surface.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/test-compositor-surface-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/test-null-compositor-surface.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/test-paginated-surface.h
)

set(cairo_test_surfaces_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/test-compositor-surface.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/test-null-compositor-surface.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/test-base-compositor-surface.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/test-paginated-surface.c
)

set(cairo_xlib_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xlib.h
)

set(cairo_xlib_private
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xlib-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xlib-surface-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xlib-xrender-private.h
)

set(cairo_xlib_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xlib-display.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xlib-core-compositor.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xlib-fallback-compositor.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xlib-render-compositor.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xlib-screen.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xlib-source.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xlib-surface.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xlib-surface-shm.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xlib-visual.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xlib-xcb-surface.c
)

set(cairo_xlib_xrender_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xlib-xrender.h
)

set(cairo_xcb_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xcb.h
)

set(cairo_xcb_private
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xcb-private.h
)

set(cairo_xcb_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xcb-connection.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xcb-connection-core.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xcb-connection-render.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xcb-connection-shm.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xcb-screen.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xcb-shm.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xcb-surface.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xcb-surface-core.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xcb-surface-render.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xcb-resources.c
)

set(cairo_qt_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-qt.h
)

set(cairo_qt_cxx_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-qt-surface.cpp
)

set(cairo_quartz_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-quartz.h
)

set(cairo_quartz_private
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-quartz-private.h
)

set(cairo_quartz_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-quartz-surface.c
)

set(cairo_quartz_image_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-quartz-image.h
)

set(cairo_quartz_image_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-quartz-image-surface.c
)

set(cairo_quartz_font_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-quartz-font.c
)

set(cairo_win32_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-win32.h
)

set(cairo_win32_private
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/win32/cairo-win32-private.h
)

set(cairo_win32_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/win32/cairo-win32-debug.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/win32/cairo-win32-device.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/win32/cairo-win32-gdi-compositor.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/win32/cairo-win32-system.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/win32/cairo-win32-surface.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/win32/cairo-win32-display-surface.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/win32/cairo-win32-printing-surface.c
)

set(cairo_win32_font_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/win32/cairo-win32-font.c
)

set(cairo_skia_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-skia.h
)

set(cairo_skia_private
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/skia/cairo-skia-private.h
)

set(cairo_skia_cxx_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/skia/cairo-skia-context.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/skia/cairo-skia-surface.cpp
)

set(cairo_os2_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-os2.h
)

set(cairo_os2_private
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-os2-private.h
)

set(cairo_os2_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-os2-surface.c
)

set(cairo_beos_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-beos.h
)

set(cairo_beos_cxx_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-beos-surface.cpp
)

set(cairo_gl_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-gl.h
)

set(cairo_gl_private
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-gl-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-gl-dispatch-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-gl-ext-def-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-gl-gradient-private.h
)

set(cairo_gl_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-gl-composite.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-gl-device.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-gl-dispatch.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-gl-glyphs.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-gl-gradient.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-gl-info.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-gl-operand.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-gl-shaders.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-gl-msaa-compositor.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-gl-spans-compositor.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-gl-traps-compositor.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-gl-source.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-gl-surface.c
)

set(cairo_glesv2_headers
  ${cairo_gl_headers}
)

set(cairo_glesv2_private
  ${cairo_gl_private}
)

set(cairo_glesv2_sources
  ${cairo_gl_sources}
)

set(cairo_egl_sources
  ${cairo_egl_sources}
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-egl-context.c
)

set(cairo_glx_sources
  ${cairo_glx_sources}
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-glx-context.c
)

set(cairo_wgl_sources
  ${cairo_wgl_sources}
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-wgl-context.c
)

set(cairo_directfb_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-directfb.h
)

set(cairo_directfb_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-directfb-surface.c
)

set(cairo_drm_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-drm.h
)

set(cairo_drm_private
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/drm/cairo-drm-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/drm/cairo-drm-ioctl-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/drm/cairo-drm-intel-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/drm/cairo-drm-intel-brw-defines.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/drm/cairo-drm-intel-brw-structs.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/drm/cairo-drm-intel-brw-eu.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/drm/cairo-drm-intel-command-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/drm/cairo-drm-intel-ioctl-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/drm/cairo-drm-i915-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/drm/cairo-drm-i965-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/drm/cairo-drm-radeon-private.h
)

set(cairo_drm_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/drm/cairo-drm.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/drm/cairo-drm-bo.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/drm/cairo-drm-surface.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/drm/cairo-drm-intel.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/drm/cairo-drm-intel-debug.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/drm/cairo-drm-intel-surface.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/drm/cairo-drm-i915-surface.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/drm/cairo-drm-i915-glyphs.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/drm/cairo-drm-i915-shader.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/drm/cairo-drm-i915-spans.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/drm/cairo-drm-i965-surface.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/drm/cairo-drm-i965-glyphs.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/drm/cairo-drm-i965-shader.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/drm/cairo-drm-i965-spans.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/drm/cairo-drm-intel-brw-eu.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/drm/cairo-drm-intel-brw-eu-emit.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/drm/cairo-drm-intel-brw-eu-util.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/drm/cairo-drm-radeon.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/drm/cairo-drm-radeon-surface.c
)

set(cairo_gallium_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/drm/cairo-drm-gallium-surface.c
)

set(cairo_script_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-script.h
)

set(cairo_script_private
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-script-private.h
)

set(cairo_script_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-script-surface.c
)

set(cairo_tee_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-tee.h
)

set(cairo_tee_private
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-tee-surface-private.h
)

set(cairo_tee_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-tee-surface.c
)

set(cairo_xml_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xml.h
)

set(cairo_xml_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xml-surface.c
)

set(cairo_vg_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-vg.h
)

set(cairo_vg_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-vg-surface.c
)

set(cairo_cogl_headers
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-cogl.h
)

set(cairo_cogl_private
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-cogl-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-cogl-gradient-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-cogl-context-private.h
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-cogl-utils-private.h
)

set(cairo_cogl_sources
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-cogl-surface.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-cogl-gradient.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-cogl-context.c
  ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-cogl-utils.c
)
