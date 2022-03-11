#pragma once

namespace uih {
void draw_drag_image_background(
    HWND wnd, bool is_themed, HTHEME theme, HDC dc, COLORREF selection_background_colour, const RECT& rc);
void draw_drag_image_label(
    HWND wnd, bool is_themed, HTHEME theme, HDC dc, const RECT& rc, COLORREF selection_text_colour, const char* text);
void draw_drag_image_icon(HDC dc, const RECT& rc, HICON icon);
BOOL create_drag_image(HWND wnd, bool is_themed, HTHEME theme, COLORREF selection_background_colour,
    COLORREF selection_text_colour, HICON icon, const LPLOGFONT font, bool show_text, const char* text,
    LPSHDRAGIMAGE lpsdi);
} // namespace uih
