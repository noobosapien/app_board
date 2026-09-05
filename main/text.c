#include "app_pch.h"

extern const int display_height;
extern const int display_width;

// Need these two to change to landscape
int real_width = 0;
int real_height = 0;

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

void draw_pixel(int16_t x, int16_t y, uint16_t color, uint8_t *frame_buffer) {

  // Change to landscape
  int16_t t;
  t = x;
  x = display_width - 1 - y;
  y = t;
  real_width = display_height;
  real_height = display_width;
    
  
  if (frame_buffer) {
    uint8_t *ptr = &frame_buffer[(x / 8) + y * ((display_width + 7) / 8)]; // Select the pixel

    if (color)
    *ptr |= 0x80 >> (x & 7); // Color the pixel
    else
    *ptr &= ~(0x80 >> (x & 7)); // Black

  }
}

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

void draw_char(int16_t x, int16_t y, unsigned char c, uint16_t color, uint8_t size_x, uint8_t size_y, uint8_t *frame_buffer){
    if(frame_buffer == NULL){
        return;
    }

    // Change to landscape
    real_width = display_height;
    real_height = display_width;
  
    // Out of bounds check
    if ((x >= real_width) ||             
        (y >= real_height) ||           
        ((x + 6 * size_x - 1) < 0) || 
        ((y + 8 * size_y - 1) < 0))  
      return;

    // Each glyph is 5 bytes long
    for (int8_t i = 0; i < 5; i++) { 
      uint8_t line = font[c * 5 + i]; // Select the glyph

      // For 8 bits, also shift right the line by 1
      for (int8_t j = 0; j < 8; j++, line >>= 1) {
        if (line & 1) { // If needed to be pixelated
          if (size_x == 1 && size_y == 1)
            draw_pixel(x + i, y + j, color, frame_buffer); // Onlly 1 pixel needs coloring
        } else {
          if (size_x == 1 && size_y == 1)
            draw_pixel(x + i, y + j, color ^ 0xff, frame_buffer); // Background
        }
      }
    }
}

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

void draw_char_line(int16_t x, int16_t y, const char* text, uint16_t len, uint16_t color, uint8_t *frame_buffer){
  
  // Iterate over all characters
  for(uint16_t i = 0; i < len; i++){
    draw_char(x + i*10, y, text[i], color, 1, 1, frame_buffer); // Keep a space after each in the x direction
  }
}