#ifndef _YUV_PICTURE_OPERATOR_H_
#define _YUV_PICTURE_OPERATOR_H_
#include "MemMgmt.h"

namespace sirius {

int32_t loadLogo(uint8_t **logo, int32_t *size, int32_t type = 0);

int32_t loadLogoWithIndentifier(uint8_t **logo, int32_t *size, uint8_t indentifier, int32_t type = 0);

int64_t hashLogo(uint8_t *logo, int32_t size);

int32_t getLogoIndentifier(uint8_t *logo, int32_t size, uint8_t *indentifier);

int32_t setLogoIndentifier(uint8_t *logo, int32_t size, uint8_t indentifier);

int32_t getLogoSize(int32_t type, int32_t *w, int32_t *h, int32_t *stride, int32_t *scanline);

};

#endif
