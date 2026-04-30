#include <stdint.h>

#include "service.h"

#include "security/sec_rt.h"

#include "security/sec_act.h"
#include "security/sec_storage.h"

enum {

    SEC_SVC_RUN,
    SEC_SVC_BIND
};

static void sec_service(void);

_SERVICE(sec_svc, sec_service);

static uint8_t secsvc_reg = SEC_SVC_BIND;

/******************************** protected region **************************************************/

static uint8_t protected_code[1024];

void sec_service() {

    switch (secsvc_reg) {
        
        case SEC_SVC_BIND: {

            for (size_t i = 0; i < sizeof(protected_code); i++) {
                protected_code[i] = ((i + 1) & 0xFF);
            }

            _sec_context(protected_code, sizeof(protected_code));

            secsvc_reg = SEC_SVC_RUN;
            sec_service();
        } break;

        case SEC_SVC_RUN: _sec_runtime(); break;
        default: _sec_recovery(); break;
    }
}

/********************************** security storage glue *********************************************/

bool _sec_storeread(void *blck, const size_t blck_size) {

    (void) blck;
    (void) blck_size;

    return false;
}

bool _sec_storewrite(void const* blck, const size_t blck_size) {

    (void) blck;
    (void) blck_size;

    return false;
}

/************************************ security actions glue ******************************************/

void _sec_recovery(void) { }

bool _sec_nonce(void *nonce, const size_t nonce_size) {

    (void) nonce;
    (void) nonce_size;

    return false;
}

bool _sec_verify(void *region_proof, void const* protected_region, const size_t proof_size) {

    (void) proof_size;
    (void) region_proof;

    (void) protected_region;

    return false;
}

bool _sec_hash(void const* region, const size_t region_size, void *hash, const size_t hash_size) {

    (void) region;
    (void) region_size;

    (void) hash;
    (void) hash_size;

    return false;
}