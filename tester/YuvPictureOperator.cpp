#include "logs.h"
#include "common.h"
#include "YuvPictureOperator.h"

namespace sirius {

#define HASH_POSITION        128

static uint8_t gDefaultLogo[] = {
    #include "resources/SiriusLogoDefault_1600x1200.24bit.nv21.dat"
};

static uint8_t gLogoType1[] = {
    #include "resources/SiriusLogo1_1664x500.24bit.nv21.dat"
};

static uint8_t gLogoType2[] = {
    #include "resources/SiriusLogo2_1200x1000.24bit.nv21.dat"
};

static const struct LogoDim {
    int32_t w;
    int32_t h;
    int32_t stride;
    int32_t scanline;
} gLogoDim[] = {
    {
        .w = 1600,
        .h = 1200,
        .stride   = 1,
        .scanline = 1,
    },
    {
        .w = 1664,
        .h = 500,
        .stride   = 1,
        .scanline = 1,
    },
    {
        .w = 1200,
        .h = 1000,
        .stride   = 1,
        .scanline = 1,
    },
    {
        .w = 1200,
        .h = 1000,
        .stride   = 1,
        .scanline = 1,
    },
};

int32_t loadLogo(uint8_t **logo, int32_t *size, int32_t type)
{
    uint8_t *logoType;
    int32_t rc = NO_ERROR;

    switch (type) {
        case 1: {
            logoType = gLogoType1;
            *size = sizeof(gLogoType1);
        }; break;
        case 2: {
            logoType = gLogoType2;
            *size = sizeof(gLogoType2);
        }; break;
        case 3: {
            logoType = gLogoType2;
            *size = sizeof(gLogoType2) * 2 / 3;
        }; break;
        case 0:
        default: {
            logoType = gDefaultLogo;
            *size = sizeof(gDefaultLogo);
        }; break;
    }

    *logo = (uint8_t *)Malloc(*size);

    if (ISNULL(logo)) {
        rc = NO_MEMORY;
    } else {
        memcpy(*logo, logoType, *size);
    }

    return rc;
}

int32_t loadLogoWithIndentifier(
    uint8_t **logo, int32_t *size, uint8_t indentifier, int32_t type)
{
    loadLogo(logo, size, type);
    return setLogoIndentifier(*logo, *size, indentifier);
}

int32_t getLogoSize(int32_t type, int32_t *w,
    int32_t *h, int32_t *stride, int32_t *scanline)
{
    int32_t rc = NO_ERROR;

    if (type < 0 || (uint32_t)type > ARRAYSIZE(gLogoDim)) {
        rc = PARAM_INVALID;
    } else {
        *w = gLogoDim[type].w;
        *h = gLogoDim[type].h;
        *stride = gLogoDim[type].stride;
        *scanline = gLogoDim[type].scanline;
    }

    return rc;
}

int64_t hashLogo(uint8_t *logo, int32_t size)
{
    int64_t result = 0LL;
    for (int32_t i = 0; i * HASH_POSITION < size; i++) {
        result += logo[i * HASH_POSITION];
    }

    return result;
}

int32_t getLogoIndentifier(uint8_t *logo, int32_t size, uint8_t *indentifier)
{
    int32_t rc = NO_ERROR;
    uint8_t result = 0x0;

    if (SUCCEED(rc)) {
        if (size >= HASH_POSITION) {
            result = logo[HASH_POSITION - 1];
        } else {
            rc = PARAM_INVALID;
        }
    }

    if (SUCCEED(rc)) {
        for (int32_t i = 1; i *  HASH_POSITION < size; i++) {
            if (result != logo[i * HASH_POSITION - 1]) {
                result = 0x0;
                rc = INVALID_FORMAT;
                break;
            }
        }
    }

    *indentifier = result;
    return rc;
}

int32_t setLogoIndentifier(uint8_t *logo, int32_t size, uint8_t indentifier)
{
    int32_t rc = NO_ERROR;

    if (SUCCEED(rc)) {
        if (size < HASH_POSITION) {
            rc = INVALID_FORMAT;
        }
    }
    if (SUCCEED(rc)) {
        for (int32_t i = 1; i * HASH_POSITION < size; i++) {
            logo[i * HASH_POSITION - 1] = indentifier;
        }
    }

    return rc;
}

};
