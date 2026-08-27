#ifndef TEXT_H
#define TEXT_H

// Draw a pixel
/**************************************************************************/
/*!
   @brief   Draw a pixel on the screen
    @param    x   X coordinate of the pixel
    @param    y   Y coordinate of the pixel
    @param    color   Color of the pixel 0x00 for no color
    @param    frame_buffer   Framebuffer to write the pixel
*/
/**************************************************************************/

void draw_pixel(int16_t x, int16_t y, uint16_t color, uint8_t *frame_buffer);


// Draw a character to the display
/**************************************************************************/
/*!
   @brief   Use the default font to render a character to the display
    @param    x   The x coordinate of the character
    @param    y   The y coordinate of the character
    @param    c   The ASCII character
    @param    color   Color of the character
    @param    size_x   Scale in x direction
    @param    size_y   Scale in y direction
    @param    frame_buffer   Framebuffer to draw
*/
/**************************************************************************/
void draw_char(int16_t x, int16_t y, unsigned char c, uint16_t color, uint8_t size_x, uint8_t size_y, uint8_t *frame_buffer);

// Write a sentence to the display
/**************************************************************************/
/*!
   @brief   Write a sentence from a char array of text
    @param    x   The x coordinate of the text
    @param    y   The y coordinate of the text
    @param    text   The ASCII text
    @param    len   Length of characters
    @param    color   Color of the text
    @param    frame_buffer   Framebuffer to write
*/
/**************************************************************************/
void draw_char_line(int16_t x, int16_t y, const char* text, uint16_t len, uint16_t color, uint8_t *frame_buffer);

#endif