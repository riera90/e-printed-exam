#include "painter.h"

void PainterClear(struct Painting* painting, int colored) {
    for (int x = 0; x < painting->w; x++) {
        for (int y = 0; y < painting->h; y++) {
            PainterDrawAbsolutePixel(painting, x, y, colored);
        }
    }
}


void PainterDrawAbsolutePixel(struct Painting* painting, int x, int y, int colored) {
    if (x < 0 || x >= painting->w || y < 0 || y >= painting->h) {
        return;
    }
    if (painting->inv) {
        if (colored) {
            painting->data[(x + y * painting->w) / 8] |= 0x80 >> (x % 8); // mem =& ~(0x01 >> bit) mem =| (0x01 >> bit)
        } else {
            painting->data[(x + y * painting->w) / 8] &= ~(0x80 >> (x % 8));
        }
    } else {
        if (colored) {
            painting->data[(x + y * painting->w) / 8] &= ~(0x80 >> (x % 8));
        } else {
            painting->data[(x + y * painting->w) / 8] |= 0x80 >> (x % 8);
        }
    }
}


void PainterDrawPixel(struct Painting* painting, int x, int y, int colored) {
int point_temp;
    if (painting->rot == ROTATE_0) {
        if(x < 0 || x >= painting->w || y < 0 || y >= painting->h) {
            return;
        }
        PainterDrawAbsolutePixel(painting, x, y, colored);
    } else if (painting->rot == ROTATE_90) {
        if(x < 0 || x >= painting->h || y < 0 || y >= painting->w) {
          return;
        }
        point_temp = x;
        x = painting->w - y;
        y = point_temp;
        PainterDrawAbsolutePixel(painting, x, y, colored);
    } else if (painting->rot == ROTATE_180) {
        if(x < 0 || x >= painting->w || y < 0 || y >= painting->h) {
          return;
        }
        x = painting->w - x;
        y = painting->h - y;
        PainterDrawAbsolutePixel(painting, x, y, colored);
    } else if (painting->rot == ROTATE_270) {
        if(x < 0 || x >= painting->h || y < 0 || y >= painting->w) {
          return;
        }
        point_temp = x;
        x = y;
        y = painting->h - point_temp;
        PainterDrawAbsolutePixel(painting, x, y, colored);
    }
}


void PainterDrawLine(struct Painting* painting, int x0, int y0, int x1, int y1, int colored) {
    /* Bresenham algorithm */
    int dx = x1 - x0 >= 0 ? x1 - x0 : x0 - x1;
    int sx = x0 < x1 ? 1 : -1;
    int dy = y1 - y0 <= 0 ? y1 - y0 : y0 - y1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while((x0 != x1) && (y0 != y1)) {
        PainterDrawPixel(painting, x0, y0 , colored);
        if (2 * err >= dy) {     
            err += dy;
            x0 += sx;
        }
        if (2 * err <= dx) {
            err += dx; 
            y0 += sy;
        }
    }
}


void PainterDrawHorizontalLine(struct Painting* painting, int x, int y, int line_width, int colored) {
    int i;
    for (i = x; i < x + line_width; i++) {
        PainterDrawPixel(painting, i, y, colored);
    }
}


void PainterDrawVerticalLine(struct Painting* painting, int x, int y, int line_height, int colored) {
    int i;
    for (i = y; i < y + line_height; i++) {
        PainterDrawPixel(painting, x, i, colored);
    }
}


void PainterDrawRectangle(struct Painting* painting, int x0, int y0, int x1, int y1, int colored) {
    int min_x, min_y, max_x, max_y;
    min_x = x1 > x0 ? x0 : x1;
    max_x = x1 > x0 ? x1 : x0;
    min_y = y1 > y0 ? y0 : y1;
    max_y = y1 > y0 ? y1 : y0;
    
    PainterDrawHorizontalLine(painting, min_x, min_y, max_x - min_x + 1, colored);
    PainterDrawHorizontalLine(painting, min_x, max_y, max_x - min_x + 1, colored);
    PainterDrawVerticalLine(painting, min_x, min_y, max_y - min_y + 1, colored);
    PainterDrawVerticalLine(painting, max_x, min_y, max_y - min_y + 1, colored);
}


void PainterDrawFilledRectangle(struct Painting* painting, int x0, int y0, int x1, int y1, int colored) {
    int min_x, min_y, max_x, max_y;
    int i;
    min_x = x1 > x0 ? x0 : x1;
    max_x = x1 > x0 ? x1 : x0;
    min_y = y1 > y0 ? y0 : y1;
    max_y = y1 > y0 ? y1 : y0;
    
    for (i = min_x; i <= max_x; i++) {
      PainterDrawVerticalLine(painting, i, min_y, max_y - min_y + 1, colored);
    }
}


void PainterDrawCircle(struct Painting* painting, int x, int y, int radius, int colored) {
    /* Bresenham algorithm */
    int x_pos = -radius;
    int y_pos = 0;
    int err = 2 - 2 * radius;
    int e2;

    do {
        PainterDrawPixel(painting, x - x_pos, y + y_pos, colored);
        PainterDrawPixel(painting, x + x_pos, y + y_pos, colored);
        PainterDrawPixel(painting, x + x_pos, y - y_pos, colored);
        PainterDrawPixel(painting, x - x_pos, y - y_pos, colored);
        e2 = err;
        if (e2 <= y_pos) {
            err += ++y_pos * 2 + 1;
            if(-x_pos == y_pos && e2 <= x_pos) {
              e2 = 0;
            }
        }
        if (e2 > x_pos) {
            err += ++x_pos * 2 + 1;
        }
    } while (x_pos <= 0);
}


void PainterDrawFilledCircle(struct Painting* painting, int x, int y, int radius, int colored) {
    /* Bresenham algorithm */
    int x_pos = -radius;
    int y_pos = 0;
    int err = 2 - 2 * radius;
    int e2;

    do {
        PainterDrawPixel(painting, x - x_pos, y + y_pos, colored);
        PainterDrawPixel(painting, x + x_pos, y + y_pos, colored);
        PainterDrawPixel(painting, x + x_pos, y - y_pos, colored);
        PainterDrawPixel(painting, x - x_pos, y - y_pos, colored);
        PainterDrawHorizontalLine(painting, x + x_pos, y + y_pos, 2 * (-x_pos) + 1, colored);
        PainterDrawHorizontalLine(painting, x + x_pos, y - y_pos, 2 * (-x_pos) + 1, colored);
        e2 = err;
        if (e2 <= y_pos) {
            err += ++y_pos * 2 + 1;
            if(-x_pos == y_pos && e2 <= x_pos) {
                e2 = 0;
            }
        }
        if(e2 > x_pos) {
            err += ++x_pos * 2 + 1;
        }
    } while(x_pos <= 0);
}


void PainterDrawCharAt(struct Painting* painting, int x, int y, char ascii_char, struct sFONT* font, int colored) {
    int i, j;
    unsigned int char_offset = (ascii_char - ' ') * font->Height * (font->Width / 8 + (font->Width % 8 ? 1 : 0));
    const unsigned char* ptr = &font->table[char_offset];

    for (j = 0; j < font->Height; j++) {
        for (i = 0; i < font->Width; i++) {
            if ((*ptr) & (0x80 >> (i % 8))) {
                PainterDrawPixel(painting, x + i, y + j, colored);
            }
            if (i % 8 == 7) {
                ptr++;
            }
        }
        if (font->Width % 8 != 0) {
            ptr++;
        }
    }
}
void PainterDrawStringAt(struct Painting* painting, int x, int y, const char* text, struct sFONT* font, int colored) {
    const char* p_text = text;
    unsigned int counter = 0;
    int refcolumn = x;
    
    // Send the string character by character on EPD
    while (*p_text != 0) {
        // Display one character on EPD
        PainterDrawCharAt(painting, refcolumn, y, *p_text, font, colored);
        // Decrement the column position by 16
        refcolumn += font->Width;
        // Point on the next character
        p_text++;
        counter++;
    }
}
