//
// Created by furture on 2017/9/22.
//

#ifndef LITEHTML_MOBILE_CONTAINER_H
#define LITEHTML_MOBILE_CONTAINER_H

#include <jni.h>
#include "litehtml.h"
#include "litebrowsercommon.h"



using namespace litehtml;

class mobile_container : public litehtml::document_container {


public:
    litehtml::position* clientFrame;
    mobile_container(LiteBrowserJava* liteBrowserJava, jobject thisObject);
    ~mobile_container();

    virtual litehtml::uint_ptr	create_font(const litehtml::tchar_t* faceName, int size, int weight, litehtml::font_style italic, unsigned int decoration, litehtml::font_metrics* fm);
    virtual void				delete_font(litehtml::uint_ptr hFont);
    virtual int					text_width(const litehtml::tchar_t* text, litehtml::uint_ptr hFont);
    virtual void				draw_text(litehtml::uint_ptr hdc, const litehtml::tchar_t* text, litehtml::uint_ptr hFont, litehtml::web_color color, const litehtml::position& pos);
    virtual int					pt_to_px(int pt);
    virtual int					get_default_font_size() const;
    virtual const litehtml::tchar_t*	get_default_font_name()const;

    void draw_list_marker(litehtml::uint_ptr hdc, const litehtml::list_marker &marker);

    void load_image(const litehtml::tchar_t *src, const litehtml::tchar_t *baseurl, bool redraw_on_ready);

    void get_image_size(const litehtml::tchar_t *src, const litehtml::tchar_t *baseurl, litehtml::size &sz);

    void draw_background(litehtml::uint_ptr hdc, const litehtml::background_paint &bg);

    void draw_borders(litehtml::uint_ptr hdc, const litehtml::borders &borders, const litehtml::position &draw_pos,
                      bool root);

    void set_caption(const litehtml::tchar_t *caption);

    void set_base_url(const litehtml::tchar_t *base_url);

    void link(const std::shared_ptr<litehtml::document> &doc, const litehtml::element::ptr &el);

    void on_anchor_click(const litehtml::tchar_t *url, const litehtml::element::ptr &el);

    void set_cursor(const litehtml::tchar_t *cursor);

    void transform_text(litehtml::tstring &text, litehtml::text_transform tt);

    void import_css(litehtml::tstring &text, const litehtml::tstring &url, litehtml::tstring &baseurl);

    void set_clip(const litehtml::position &pos, const litehtml::border_radiuses &bdr_radius, bool valid_x,
                  bool valid_y);

    void del_clip();

    void get_client_rect(litehtml::position &client) const;

    std::shared_ptr<element> create_element(const litehtml::tchar_t *tag_name, const litehtml::string_map &attributes,
                                            const std::shared_ptr<litehtml::document> &doc);

    void get_media_features(litehtml::media_features &media) const;

    void get_language(litehtml::tstring &language, litehtml::tstring &culture) const;


private:
    jobject  _thisObject;
    LiteBrowserJava* _liteBrowserJava;
};


#endif //LITEHTML_MOBILE_CONTAINER_H
