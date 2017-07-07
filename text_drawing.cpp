#include "stdafx.h"

SCRIPT_DIGITSUBSTITUTE uih::UniscribeTextRenderer::m_sdg;
bool uih::UniscribeTextRenderer::m_sdg_valid;

#define ELLIPSIS "\xe2\x80\xa6"//"\x85"
#define ELLIPSIS_LEN 3

namespace uih
{

    bool check_colour_marks(const char * src, unsigned int len = -1)
    {
        const char * ptr = src;
        while(*ptr && (unsigned)(ptr-src) < len)
        {
            if (*ptr==3)
            {
                return true;
            }
            ptr++;
        }
        return false;
    }

    void remove_color_marks(const char * src, pfc::string_base& out, t_size len)
    {
        out.reset();
        const char * ptr = src;
        while(*src && static_cast<t_size>(src - ptr) < len)
        {
            if (*src==3)
            {
                src++;
                while(*src && *src!=3) src++;
                if (*src==3) src++;
            }
            else out.add_byte(*src++);
        }
    }

    unsigned get_text_truncate_point(const char * src, unsigned len)
    {
        unsigned rv = len;

        const char * ptr = src;
        ptr += len-1;
        while (ptr>src && (*ptr == ' ' || *ptr == '.')) {ptr --;rv--;}
        return rv;
    }

    BOOL CharacterExtentsCalculator::run(HDC dc, const char * text, int length, int max_width, LPINT max_chars, LPINT width_array, LPSIZE sz, unsigned * width_out, bool trunc)
    {
        const char * src = text;

        if (check_colour_marks(text, length))
        { 
            remove_color_marks(text, temp, length);
            src = temp;
            length = temp.length();
        }

        text_w.convert(src, length);
        UniscribeTextRenderer p_ScriptString;
        p_ScriptString.analyse(dc, text_w.get_ptr(), text_w.length(), max_width, max_chars != nullptr);

        int _max_chars = p_ScriptString.get_output_character_count();
        if (width_array)
            p_ScriptString.get_character_logical_extents(width_array);
        if (max_chars) *max_chars = _max_chars;
        if (trunc || width_out)
        {
            w_utf8.convert(text_w.get_ptr(), *max_chars);
            *max_chars = trunc ? get_text_truncate_point(w_utf8, w_utf8.length()) : w_utf8.length();
            if (width_out)
                *width_out = get_text_width_colour(dc, w_utf8, *max_chars);
        }
        return TRUE;
    }

    bool is_rect_null_or_reversed(const RECT * r)
    {
        return r->right <= r->left || r->bottom <= r->top;
    }

    void get_text_size(HDC dc,const char * src,int len, SIZE & sz)
    {
        sz.cx = 0;
        sz.cy = 0;
        if (!dc) return;
        if (len<=0) return;

        pfc::stringcvt::string_wide_from_utf8 wstr(src, len);
        t_size wlen = wstr.length();

        UniscribeTextRenderer p_ScriptString(dc, wstr, wlen, NULL, false);
        p_ScriptString.get_output_size(sz);
    }

    int get_text_width(HDC dc,const char * src,int len)
    {
        SIZE sz;
        get_text_size(dc, src, len, sz);
        return sz.cx;
    }

    int get_text_width_colour(HDC dc,const char * src,int len, bool b_ignore_tabs)
    {
        if (!dc) return 0;
        int ptr = 0;
        int start = 0;
        int rv = 0;
        if (len<0) len = strlen(src);
        while(ptr<len)
        {
            if (src[ptr]==3)
            {
                rv += get_text_width(dc,src+start,ptr-start);
                ptr++;
                while(ptr<len && src[ptr]!=3) ptr++;
                if (ptr<len) ptr++;
                start = ptr;
            }
            else if (b_ignore_tabs && src[ptr]=='\t')
            {
                rv += get_text_width(dc,src+start,ptr-start);
                while (ptr<len && src[ptr]=='\t') ptr++;
            }
            else ptr++;
        }
        rv += get_text_width(dc,src+start,ptr-start);
        return rv;
    }

    int strlen_utf8_colour(const char * src,int len=-1)
    {
        int ptr = 0;
        int start = 0;
        int rv = 0;
        if (len<0) len = strlen(src);
        while(ptr<len)
        {
            if (src[ptr]==3)
            {
                rv += pfc::strlen_utf8(src+start,ptr-start);
                ptr++;
                while(ptr<len && src[ptr]!=3) ptr++;
                if (ptr<len) ptr++;
                start = ptr;
            }
            else ptr++;
        }
        rv += pfc::strlen_utf8(src+start,ptr-start);
        return rv;
    }


    BOOL text_out_colours_ellipsis(HDC dc,const char * src_c,int src_c_len,int x_offset,int pos_y,const RECT * base_clip,bool selected,bool show_ellipsis,DWORD default_color,alignment align, unsigned * p_width, bool b_set_default_colours, int * p_position)
    {
        class colour_change_data
        {
        public:
            t_size index, length;
            COLORREF colour;
        };

        pfc::stringcvt::string_wide_from_utf8 wstr(src_c, src_c_len);

        pfc::array_t<wchar_t> src_s;
        const wchar_t * src = wstr.get_ptr();

        t_size wLen = wstr.length();

        if (!base_clip) return FALSE;

        if (base_clip)
        {
            if (is_rect_null_or_reversed(base_clip) || base_clip->right<=base_clip->left+x_offset || base_clip->bottom<=pos_y) return TRUE;
        }


        pfc::list_t<colour_change_data, pfc::alloc_fast_aggressive> colourChanges;

        bool b_lastColourUsed = true;
        COLORREF cr_last = NULL;

        {
            const wchar_t * p_start = src;
            const wchar_t * p_block_start = src;
            const wchar_t * p_current = src;

            COLORREF cr_current = b_set_default_colours ? default_color : GetTextColor(dc);

            while (size_t(p_current - p_start) < wLen)
            {
                p_block_start = p_current;
                while (size_t(p_current - p_start) < wLen && p_current[0] != 3 )
                {
                    p_current++;
                }
                if (p_current > p_block_start)
                {
                    colour_change_data temp;
                    temp.index = src_s.get_size();
                        //p_block_start - src;
                    temp.length = p_current-p_block_start;
                    temp.colour = cr_current;
                    colourChanges.add_item(temp);
                    src_s.append_fromptr(p_block_start, p_current-p_block_start);
                    b_lastColourUsed = true;
                }
                else if (size_t(p_current - p_start) >= wLen)
                    break; 
                if (size_t(p_current - p_start) + 1 < wLen) //need another at least char for valid colour code operation
                {
                    if (p_current[0] == 3)
                    {
                        COLORREF new_colour;
                        COLORREF new_colour_selected;

                        bool have_selected = false;

                        if (p_current[1]==3) 
                        {
                            new_colour=new_colour_selected=default_color;
                            have_selected=true;
                            p_current+=2;
                        }
                        else
                        {
                            p_current++;
                            new_colour = mmh::strtoul_n(p_current,min(6,wLen-(p_current-p_start)), 0x10);
                            while(size_t(p_current-p_start)<wLen && p_current[0]!=3 && p_current[0]!='|')
                            {
                                p_current++;
                            }
                            if (size_t(p_current-p_start)<wLen && p_current[0]=='|')
                            {
                                if (size_t(p_current-p_start)+1<wLen)
                                {
                                    p_current++;
                                    new_colour_selected = mmh::strtoul_n(p_current,min(6,wLen-(p_current-p_start)), 0x10);
                                    have_selected = true;
                                }
                                else
                                    break; //unexpected eos, malformed string

                                while(size_t(p_current-p_start)<wLen && p_current[0]!=3) p_current++;
                            }
                            p_current++;
                        }
                        if (selected) new_colour = have_selected ? new_colour_selected : 0xFFFFFF - new_colour;
                        
                        b_lastColourUsed = false;
                        cr_last = (cr_current = new_colour);
                    }
                }
                else
                    break; //unexpected eos, malformed string
            }

        }

        RECT clip = *base_clip, ellipsis = {0,0,0,0};

        t_size i, colourChangeCount = colourChanges.get_count();
        int widthTotal = 0, availableWidth = clip.right - clip.left - x_offset;

        src = src_s.get_ptr();
        wLen = src_s.get_size();

        pfc::array_staticsize_t<UniscribeTextRenderer> scriptStrings(colourChangeCount);
        pfc::array_staticsize_t<int> characterExtents (wLen);

        for (i=0; i<colourChangeCount; i++)
        {
            t_size thisWLen = colourChanges[i].length;
            if (thisWLen)
            {
                const wchar_t * thisWStr = &src[colourChanges[i].index];
                scriptStrings[i].analyse(dc, thisWStr, thisWLen, /*availableWidth - widthTotal*/ -1, false);
                scriptStrings[i].get_character_logical_extents(&characterExtents[colourChanges[i].index], i && colourChanges[i].index ? characterExtents[colourChanges[i].index-1] : 0);
                widthTotal += scriptStrings[i].get_output_width();
                if (widthTotal > availableWidth)
                {
                    colourChangeCount = i+1;
                    break;
                }
            }
        }


        COLORREF cr_old = GetTextColor(dc);

        SetTextAlign(dc,TA_LEFT);
        SetBkMode(dc,TRANSPARENT);
        //if (b_set_default_colours)
        //    SetTextColor(dc, default_color);

        int position = clip.left;//item.left+BORDER;

        if (show_ellipsis)
        {
            if (widthTotal > availableWidth) 
            {
                int ellipsis_width = get_text_width(dc, ELLIPSIS, ELLIPSIS_LEN)+2;
                if (ellipsis_width <= clip.right - clip.left - x_offset)
                {
                    for (i=wLen; i; i--)
                    {
                        if (colourChangeCount && colourChanges[colourChangeCount-1].length == 0)
                        {
                            colourChangeCount--;
                        }
                        else if ( (i > 2 && characterExtents[i-2] + ellipsis_width <= availableWidth) || i == 1)
                        {
                            src_s[i-1] = 0x2026;
                            src_s.set_size(i);
                            src = src_s.get_ptr();
                            wLen = i;
                            if (colourChangeCount)
                            {
                                widthTotal -= scriptStrings[colourChangeCount-1].get_output_width();
                                scriptStrings[colourChangeCount-1].analyse(dc, &src[colourChanges[colourChangeCount-1].index], colourChanges[colourChangeCount-1].length, -1, false);
                                widthTotal += scriptStrings[colourChangeCount-1].get_output_width();
                            }
                            break;
                        }
                        else if (colourChangeCount && colourChanges[colourChangeCount-1].index == i-1)
                        {
                            colourChangeCount--;
                            widthTotal -= scriptStrings[colourChangeCount].get_output_width();
                        }
                        else if (colourChangeCount)
                        {
                            colourChanges[colourChangeCount-1].length = i - colourChanges[colourChangeCount-1].index - 1;
                        }
                    }
                }
            }
        }

        if (align == ALIGN_CENTRE)
        {
            position += (clip.right - clip.left - widthTotal)/2;
        }
        else if (align == ALIGN_RIGHT)
        {
            position += (clip.right - clip.left - widthTotal - x_offset);
        }
        else 
        { 
            position+=x_offset;
        }

        if (p_width) *p_width = NULL;
        for (i=0; i<colourChangeCount; i++)
        {
            SetTextColor(dc, colourChanges[i].colour);
            scriptStrings[i].text_out(position,pos_y,ETO_CLIPPED,&clip);
            int thisWidth = scriptStrings[i].get_output_width();
            position += thisWidth;
            if (p_width) *p_width += thisWidth;
        }

        if (p_position) *p_position = position;

        if (b_set_default_colours)
            SetTextColor(dc, cr_old);
        else if (!b_lastColourUsed)
            SetTextColor(dc, cr_last);

        return TRUE;
    }

    BOOL text_out_colours_tab(HDC dc,const char * display,int display_len,int left_offset,int border,const RECT * base_clip,bool selected,DWORD default_color,bool columns,bool use_tab,bool show_ellipsis,alignment align, unsigned * p_width, bool b_set_default_colours, bool b_vertical_align_centre, int * p_position)
    {

        if (!base_clip) return FALSE;

        RECT clip = *base_clip;

        if (is_rect_null_or_reversed(&clip)) return TRUE;

        int extra = (clip.bottom-clip.top - uih::get_dc_font_height(dc));

        int pos_y = clip.top + (b_vertical_align_centre ? (extra/2) : extra);

        int n_tabs = 0;

        display_len = pfc::strlen_max(display, display_len);
        

        columns = use_tab; //always called equal

        if (use_tab)
        {
            int start = 0;
            int n;
            for(n=0;n<display_len;n++)
            {
                if (display[n]=='\t')
                {
                    start = n+1;
                    n_tabs++;
                }
            }
        }

        if (n_tabs == 0)
        {
            clip.left += border;
            clip.right -= border;
            return text_out_colours_ellipsis(dc, display, display_len, left_offset,pos_y,&clip,selected,show_ellipsis,default_color,align, p_width, b_set_default_colours, p_position);
        }

        if (p_width)
            *p_width = clip.right;
        if (p_position)
            *p_position = clip.right;

        clip.left += left_offset;
        int tab_total = clip.right - clip.left;
        
        int ptr = display_len;
        int tab_ptr = 0;
        int written = 0;
        int clip_x = clip.right;
        RECT t_clip = clip;

        do
        {
            int ptr_end = ptr;
            while(ptr>0 && display[ptr-1]!='\t') ptr--;
            const char * t_string = display + ptr;
            int t_length = ptr_end - ptr;
            if (t_length>0)
            {
                t_clip.right -= border;

                if (tab_ptr)
                    t_clip.left = clip.right - MulDiv(tab_ptr,tab_total,n_tabs) + border;
                

                unsigned p_written = NULL;
                text_out_colours_ellipsis(dc,t_string,t_length,0,pos_y,&t_clip,selected,show_ellipsis,default_color,tab_ptr ? ALIGN_LEFT : ALIGN_RIGHT, &p_written, b_set_default_colours);

                if (tab_ptr == 0)
                    t_clip.left = t_clip.right - p_written;

                t_clip.right = t_clip.left - border;
            }

            if (ptr>0)
            {
                ptr--;//tab char
                tab_ptr++;
            }
        }
        while(ptr>0);

        return TRUE;
    }
}
