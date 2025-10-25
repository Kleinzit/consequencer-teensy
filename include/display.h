#pragma once
#include <U8g2lib.h>
#include "config.h"

class Display {
public:
    Display();
    void init();
    void updateGrid(bool grid[ROWS][COLS], int currentStep);
    void clear();

private:
    U8G2_SH1106_128X64_NONAME_F_4W_SW_SPI oled;
};
