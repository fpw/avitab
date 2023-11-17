if(APPLE)
	set(CMAKE_C_ARCHIVE_FINISH "<CMAKE_RANLIB> -c <TARGET>")
endif()

list(APPEND lvgl_sources
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_core/lv_debug.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_core/lv_disp.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_core/lv_group.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_core/lv_indev.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_core/lv_obj.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_core/lv_refr.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_core/lv_style.c
)

list(APPEND lvgl_sources
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_draw/lv_draw.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_draw/lv_draw_arc.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_draw/lv_draw_basic.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_draw/lv_draw_img.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_draw/lv_draw_label.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_draw/lv_draw_line.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_draw/lv_draw_rect.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_draw/lv_draw_triangle.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_draw/lv_img_cache.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_draw/lv_img_decoder.c
)

list(APPEND lvgl_sources
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_font/lv_font.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_font/lv_font_fmt_txt.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_font/lv_font_roboto_12.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_font/lv_font_roboto_16.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_font/lv_font_roboto_22.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_font/lv_font_roboto_28.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_font/lv_font_unscii_8.c
)

list(APPEND lvgl_sources
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_hal/lv_hal_disp.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_hal/lv_hal_indev.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_hal/lv_hal_tick.c
)

list(APPEND lvgl_sources
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_misc/lv_anim.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_misc/lv_area.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_misc/lv_async.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_misc/lv_bidi.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_misc/lv_circ.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_misc/lv_color.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_misc/lv_fs.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_misc/lv_gc.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_misc/lv_ll.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_misc/lv_log.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_misc/lv_math.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_misc/lv_mem.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_misc/lv_printf.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_misc/lv_task.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_misc/lv_templ.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_misc/lv_txt.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_misc/lv_utils.c
)

list(APPEND lvgl_sources
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_objx/lv_arc.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_objx/lv_bar.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_objx/lv_btn.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_objx/lv_btnm.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_objx/lv_calendar.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_objx/lv_canvas.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_objx/lv_cb.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_objx/lv_chart.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_objx/lv_cont.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_objx/lv_cpicker.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_objx/lv_ddlist.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_objx/lv_gauge.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_objx/lv_img.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_objx/lv_imgbtn.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_objx/lv_kb.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_objx/lv_label.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_objx/lv_led.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_objx/lv_line.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_objx/lv_list.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_objx/lv_lmeter.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_objx/lv_mbox.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_objx/lv_objx_templ.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_objx/lv_page.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_objx/lv_preload.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_objx/lv_roller.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_objx/lv_slider.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_objx/lv_spinbox.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_objx/lv_sw.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_objx/lv_ta.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_objx/lv_table.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_objx/lv_tabview.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_objx/lv_tileview.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_objx/lv_win.c
)

list(APPEND lvgl_sources
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_themes/lv_theme.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_themes/lv_theme_alien.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_themes/lv_theme_default.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_themes/lv_theme_material.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_themes/lv_theme_mono.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_themes/lv_theme_nemo.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_themes/lv_theme_night.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_themes/lv_theme_templ.c
    ${CMAKE_CURRENT_LIST_DIR}/lvgl/src/lv_themes/lv_theme_zen.c
)

list(APPEND lvgl_sources
    ${CMAKE_CURRENT_LIST_DIR}/lv_font_roboto_20.c
)
