## sources.cmake
##
## Copyright (C) 2013-2024 Christian Schenk
## 
## This file is free software; the copyright holder gives
## unlimited permission to copy and/or distribute it, with or
## without modifications, as long as this notice is preserved.

set(cairo_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-analysis-surface.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-arc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-array.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-atomic.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-base64-stream.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-base85-stream.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-bentley-ottmann-rectangular.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-bentley-ottmann-rectilinear.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-bentley-ottmann.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-botor-scan-converter.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-boxes-intersect.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-boxes.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-cache.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-cff-subset.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-clip-boxes.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-clip-polygon.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-clip-region.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-clip-surface.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-clip-tor-scan-converter.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-clip.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-color.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-composite-rectangles.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-compositor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-contour.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-damage.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-debug.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-default-context.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-deflate-stream.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-device.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-error.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-fallback-compositor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-fixed.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-font-face-twin-data.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-font-face-twin.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-font-face.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-font-options.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-freed-pool.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-freelist.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-gstate.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-hash.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-hull.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-image-compositor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-image-info.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-image-source.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-image-surface.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-line.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-lzw.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-mask-compositor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-matrix.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-mempool.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-mesh-pattern-rasterizer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-misc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-mono-scan-converter.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-mutex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-no-compositor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-observer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-output-stream.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-paginated-surface.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-path-bounds.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-path-fill.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-path-fixed.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-path-in-fill.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-path-stroke-boxes.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-path-stroke-polygon.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-path-stroke-traps.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-path-stroke-tristrip.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-path-stroke.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-path.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-pattern.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-pdf-operators.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-pdf-shading.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-pen.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-polygon-intersect.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-polygon-reduce.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-polygon.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-raster-source-pattern.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-recording-surface.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-rectangle.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-rectangular-scan-converter.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-region.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-rtree.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-scaled-font-subsets.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-scaled-font.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-shape-mask-compositor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-slope.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-spans-compositor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-spans.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-spline.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-stroke-dash.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-stroke-style.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-clipper.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-fallback.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-observer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-offset.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-snapshot.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-subsurface.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-wrapper.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-tag-attributes.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-tag-stack.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-time.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-tor-scan-converter.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-tor22-scan-converter.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-toy-font-face.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-traps-compositor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-traps.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-tristrip.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-truetype-subset.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-type1-fallback.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-type1-glyph-names.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-type1-subset.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-type3-glyph-surface.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-unicode.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-user-font.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-version.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-wideint.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo.c
)

set(cairo_headers
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-deprecated.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo.h
)

set(ignore_headers
    # Internal headers
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairoint.h
    ${CMAKE_CURRENT_BINARY_DIR}/include/cairo-features.h

    # Inlined API
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-box-inline.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-clip-inline.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-combsort-inline.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-contour-inline.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-error-inline.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-image-surface-inline.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-line-inline.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-list-inline.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-pattern-inline.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-recording-surface-inline.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-inline.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-observer-inline.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-snapshot-inline.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-subsurface-inline.h

    # Private headers
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-analysis-surface-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-arc-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-array-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-atomic-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-backend-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-boxes-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-cache-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-clip-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-compiler-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-composite-rectangles-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-compositor-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-contour-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-damage-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-default-context-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-deprecated.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-device-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-error-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-fixed-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-fixed-type-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-fontconfig-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-freed-pool-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-freelist-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-freelist-type-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-ft-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-gstate-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-hash-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-image-info-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-image-surface-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-line-private.h
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
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-pattern-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-pdf-operators-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-pdf-shading-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-pdf-surface-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-pixman-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-ps-surface-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-quartz-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-recording-surface-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-reference-count-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-region-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-rtree-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-scaled-font-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-scaled-font-subsets-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-script-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-slope-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-spans-compositor-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-spans-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-stroke-dash-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-backend-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-clipper-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-fallback-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-observer-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-offset-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-snapshot-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-subsurface-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-surface-wrapper-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-svg-surface-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-tag-attributes-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-tag-stack-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-time-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-traps-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-tristrip-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-truetype-subset-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-type1-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-type3-glyph-surface-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-types-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-user-font-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-wideint-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-wideint-type-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xcb-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xlib-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xlib-surface-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xlib-xrender-private.h
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/test-compositor-surface-private.h
)

set(cairo_feature_sources_cairo_png
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-png.c
)

set(cairo_feature_sources_cairo_ft_font
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-colr-glyph-render.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-ft-font.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-svg-glyph-render.c
)

set(cairo_feature_sources_cairo_xlib
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

set(cairo_feature_sources_xcb
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

set(cairo_feature_sources_cairo_quartz
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-quartz-surface.c
)

set(cairo_feature_sources_cairo_quartz_image
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-quartz-image-surface.c
)

set(cairo_feature_sources_cairo_quartz_font
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-quartz-font.c
)

set(cairo_feature_sources_cairo_win32
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/win32/cairo-win32-debug.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/win32/cairo-win32-device.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/win32/cairo-win32-gdi-compositor.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/win32/cairo-win32-system.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/win32/cairo-win32-surface.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/win32/cairo-win32-display-surface.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/win32/cairo-win32-printing-surface.c
)

set(cairo_feature_sources_cairo_win32_font
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/win32/cairo-win32-font.c
)

set(cairo_feature_sources_cairo_dwrite_font
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/win32/cairo-dwrite-font.cpp
)

set(cairo_feature_sources_cairo_script
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-script-surface.c
)

set(cairo_feature_sources_cairo_ps
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-ps-surface.c
)

set(cairo_feature_sources_cairo_pdf
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-pdf-surface.c
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-pdf-interchange.c
)

set(cairo_feature_sources_cairo_svg
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-svg-surface.c
)

set(cairo_feature_sources_cairo_tee
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-tee-surface.c
)

set(cairo_feature_headers_cairo_ps
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-ps.h
)

set(cairo_feature_headers_cairo_pdf
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-pdf.h
)

set(cairo_feature_headers_cairo_svg
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-svg.h
)

set(cairo_feature_headers_cairo_ft_font
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-ft.h
)

set(cairo_feature_headers_cairo_xlib
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xlib.h
)

set(cairo_feature_headers_cairo_xlib_render
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xlib-xrender.h
)

set(cairo_feature_headers_cairo_xcb
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-xcb.h
)

set(cairo_feature_headers_cairo_quartx
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-quartz.h
)

set(cairo_feature_headers_cairo_quartz_image
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-quartz-image.h
)

set(cairo_feature_headers_cairo_win32
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-win32.h
)

set(cairo_feature_headers_cairo_dwrite_font
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-dwrite.h
)

set(cairo_feature_headers_cairo_script
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-script.h
)

set(cairo_feature_headers_cairo_tee
    ${CMAKE_CURRENT_SOURCE_DIR}/source/src/cairo-tee.h
)
