#ifdef MASTER_MODE

#include "display.h"

Display::Display() : oled(U8G2_R0, OLED_CLK, OLED_MOSI, OLED_CS, OLED_DC, OLED_RESET) {}

void Display::init() {
    oled.begin();
    oled.clearBuffer();
    oled.setFont(u8g2_font_6x10_tr);
    oled.sendBuffer();
}

void Display::updateGrid(bool grid[ROWS][COLS], int currentStep) {
    oled.clearBuffer();
    
    const int cellWidth = 14;
    const int cellHeight = 14;
    const int gapX = 2;
    const int gapY = 2;
    const int gridStartX = 2;
    const int gridStartY = 2;
    
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            int x = gridStartX + col * (cellWidth + gapX);
            int y = gridStartY + row * (cellHeight + gapY);
            
            bool isActive = grid[row][col];
            bool isCurrent = (col == currentStep);
            
            if (isCurrent && isActive) {
                oled.drawBox(x, y, cellWidth, cellHeight);
                oled.setDrawColor(0);
                oled.drawBox(x + 3, y + 3, cellWidth - 6, cellHeight - 6);
                oled.setDrawColor(1);
            } else if (isCurrent) {
                oled.drawFrame(x, y, cellWidth, cellHeight);
                oled.drawFrame(x + 1, y + 1, cellWidth - 2, cellHeight - 2);
            } else if (isActive) {
                oled.drawBox(x, y, cellWidth, cellHeight);
            } else {
                oled.drawFrame(x, y, cellWidth, cellHeight);
            }
        }
    }
    
    oled.sendBuffer();
}

void Display::clear() {
    oled.clearBuffer();
    oled.sendBuffer();
}

#endif // MASTER_MODE
