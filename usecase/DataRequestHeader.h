#ifndef _DATA_REQUEST_HEADER_H_
#define _DATA_REQUEST_HEADER_H_

namespace sirius {

#define CUSTOM_DATA_MEMORY_NUM    6
#define DATA_TRANSMIT_MEMORY_SIZE (1 * 1024 * 1024)

struct DataRequestHeader {
    int32_t type;
    int32_t block;
    int32_t blocks;
    int32_t size;
};

};

#endif
