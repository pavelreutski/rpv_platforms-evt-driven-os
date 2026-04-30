#include <stdint.h>
#include <string.h>

#include "security/sec_rt.h"

#include "security/sec_act.h"
#include "security/sec_storage.h"

#include "security/sec_conf.h"

enum {

    SEC_INIT,
    SEC_RUNTIME,
    SEC_RECOVERY
};

union sec_artifact_s {        
        
    struct {

        uint8_t proof[SEC_HASH_SIZE];
        uint8_t protec_hash[SEC_HASH_SIZE]; 
        uint8_t nonce[SEC_NONCE_SIZE];

    };

    uint8_t seq[SEC_HASH_SIZE * 3];
};

typedef union sec_artifact_s sec_art_t;

static void const* protected = NULL;
static size_t protected_size = 0;

static uint8_t nonce[SEC_NONCE_SIZE];

static uint8_t sec_reg = SEC_INIT;

void _sec_context(void const* protected_region, const size_t region_size) {

    sec_reg = SEC_INIT;

    protected = protected_region;
    protected_size = region_size;
    
    _sec_runtime();
}

void _sec_runtime(void) {

    switch (sec_reg) {

        case SEC_INIT: {

            sec_reg = SEC_RUNTIME;
                        
            sec_art_t art;
            uint8_t hash[SEC_HASH_SIZE];

            if (!_sec_verify(art.proof, protected, sizeof(art.proof))) {

                sec_reg = SEC_RECOVERY;
                _sec_runtime();

                return;
            }
            
            _sec_nonce(nonce, sizeof(nonce)); // generate nonce
            _sec_hash(protected, protected_size, art.protec_hash, sizeof(art.protec_hash)); // hash(code)

            memcpy(art.nonce, nonce, sizeof(art.nonce));
            
            _sec_hash(art.proof, sizeof(art.proof), hash, sizeof(hash));
            _sec_storewrite(hash, sizeof(hash)); // S_0 = hash(proof) -> S_static

            _sec_hash(art.seq, sizeof(art.seq), hash, sizeof(hash));
            _sec_storewrite(hash, sizeof(hash)); // S_1 = hash(S_static || hash(code) || nonce) -> S_epoch_dynamic            

        } break;

        case SEC_RUNTIME: {

            // S_0 (S_static): hash(proof)
            // S_1 (S_epoch_dynamic): hash(S_static || nonce)

            // verification: hash(S_static || hash(code) || nonce) == S_1

        } break;

        case SEC_RECOVERY: {

            _sec_recovery();
            sec_reg = SEC_RECOVERY;
        } break;
        
        default: { sec_reg = SEC_RECOVERY; } break;
    }
}