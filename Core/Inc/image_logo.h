/*
 * image_logo.h
 *
 *  Created on: Aug 18, 2025
 *      Author: y_akagi2025
 */

#ifndef INC_IMAGE_LOGO_H_
#define INC_IMAGE_LOGO_H_

#include <stdint.h>

typedef struct {
    const uint8_t *data;
    uint16_t width;
    uint16_t height;
    uint8_t bpp;
} tImage;

extern const uint8_t image_data_Image_logo[];
extern const tImage Image_logo;

#endif /* INC_IMAGE_LOGO_H_ */