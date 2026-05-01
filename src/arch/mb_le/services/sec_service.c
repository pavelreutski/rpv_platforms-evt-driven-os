#include <stdint.h>
#include <string.h>

#include "service.h"

#include "kernel_stdio.h"

#include "security/sec_rt.h"
#include "security/sec_conf.h"

#include "security/sec_act.h"
#include "security/sec_storage.h"

enum {

    SEC_SVC_RUN,
    SEC_SVC_BIND
};

static void sec_service(void);

_SERVICE(sec_svc, sec_service);

static uint8_t secsvc_reg = SEC_SVC_BIND;

/***************************************** nonce ****************************************************/

static uint8_t nonce_counter = 1;

/************************************* secure storage ***********************************************/

static uint8_t secure_store[2][SEC_HASH_SIZE];

/************************************ protected region **********************************************/

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

bool _sec_storeread(uint8_t blck_uid, void *blck, const size_t blck_size) {

    bool is_blckvalid = (blck_uid > 1) && (blck_size <= sizeof(secure_store[0]));

    if (is_blckvalid) {
        memcpy(blck, secure_store[blck_uid], blck_size);
    }

    return is_blckvalid;
}

bool _sec_storewrite(uint8_t blck_uid, void const* blck, const size_t blck_size) {

    bool is_blckvalid = (blck_uid > 1) && (blck_size <= sizeof(secure_store));

    if (is_blckvalid) {
        memcpy(secure_store[blck_uid], blck, blck_size);
    }

    return is_blckvalid;
}

/************************************ security actions glue ******************************************/

void _sec_recovery(void) {

    _kernel_outString("Security breach detected !\n");
    _kernel_outString("Recovery in progress...\n");

    secsvc_reg = SEC_SVC_BIND;
    sec_service();
}

bool _sec_nonce(void *nonce, const size_t nonce_size) {

    size_t i;
    uint8_t *nonce_blck;

    for (nonce_blck = (uint8_t *) nonce, i = 0; i < nonce_size; i++) {
        nonce_blck[i] = nonce_counter++;
    }

    return true;
}

bool _sec_verify(void *region_proof, void const* protected_region, const size_t proof_size) {

    if (protected_region != protected_code) {
        return false;
    }

    if (memcmp(protected_code, protected_region, sizeof(protected_code)) != 0) {
        return false;
    }

    memcpy(region_proof, protected_code, proof_size);
    return true;
}

bool _sec_hash(void const* region, const size_t region_size, void *hash, const size_t hash_size) {    
    
    (void) region_size;

    size_t i;

    uint8_t *hash_blck;
    uint8_t const* region_blck;

    for (hash_blck = hash, region_blck = region, i = 0; i < hash_size; i++) {        
        hash_blck[i] = 
            ((i % 2) == 0) ? (region_blck[i] ^ 0xAA) : (region_blck[i] ^ 0x55);
    }

    return true;
}