#pragma once

FT_Library ft;
FT_Face font_face;

void init_freetype() {
    FT_Init_FreeType(&ft);
    FT_New_Face(ft, "../MonospaceBold.ttf", 0, &face);
    
    
}
