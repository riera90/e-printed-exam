#ifndef PAINTER_H
#define PAINTER_H

#define WIDTH               400
#define HEIGHT              300
#define ROTATE_0            0
#define ROTATE_90           1
#define ROTATE_180          2
#define ROTATE_270          3
#define COLORED             0
#define UNCOLORED           1

struct Painting{
    unsigned char* data;
    int w;
    int h;
    int rot;
    unsigned char inv;
};


void PainterClear(struct Painting* painting, int colored);
void PainterDrawAbsolutePixel(struct Painting* painting, int x, int y, int colored);
void PainterDrawPixel(struct Painting* painting, int x, int y, int colored);
void PainterDrawLine(struct Painting* painting, int x0, int y0, int x1, int y1, int colored);
void PainterDrawHorizontalLine(struct Painting* painting, int x, int y, int width, int colored);
void PainterDrawVerticalLine(struct Painting* painting, int x, int y, int height, int colored);
void PainterDrawRectangle(struct Painting* painting, int x0, int y0, int x1, int y1, int colored);
void PainterDrawFilledRectangle(struct Painting* painting, int x0, int y0, int x1, int y1, int colored);
void PainterDrawCircle(struct Painting* painting, int x, int y, int radius, int colored);
void PainterDrawFilledCircle(struct Painting* painting, int x, int y, int radius, int colored);
//void PainterDrawCharAt(struct Painting* painting, int x, int y, char ascii_char, sFONT* font, int colored);
//void PainterDrawStringAt(struct Painting* painting, int x, int y, const char* text, sFONT* font, int colored);

#endif