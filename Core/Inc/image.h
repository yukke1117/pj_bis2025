#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>

typedef struct {
    const uint8_t *data;
    uint16_t width;
    uint16_t height;
    uint8_t bpp;
} tImage;

extern const uint8_t image_data_Image[];
extern const tImage Image;

#endif // IMAGE_H
