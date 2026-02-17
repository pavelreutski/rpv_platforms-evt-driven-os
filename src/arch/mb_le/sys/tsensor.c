#include <stddef.h>

#include "sys/tsensor.h"

#define ADT7420S_BASE            (0x44A10000)

#define ADT7420S                 ((adt7420s_t *) ADT7420S_BASE)

union adt7420s_status_u {

    uint32_t reg;

    struct {

        uint32_t tvalid   : 1;
        uint32_t reserved : 31;
    };
};

struct adt7420s_s {

    volatile union adt7420s_status_u sr;
    volatile uint32_t dt;
};

typedef struct adt7420s_s adt7420s_t;
typedef union adt7420_status_u adt7420_status_t;

bool _tsensor_read(uint16_t *t) {
    
    if ((t == NULL) || 
            !(ADT7420S -> sr).tvalid) {
        return false;
    }

    uint32_t dt = (ADT7420S -> dt);
    *t = (uint16_t)(dt & 0xFFFF);

    return true;
}