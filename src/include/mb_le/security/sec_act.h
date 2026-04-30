#pragma once

#include <stddef.h>
#include <stdbool.h>

void _sec_recovery(void);

bool _sec_nonce(void *nonce, const size_t nonce_size);

bool _sec_verify(void *region_proof, void const* protected_region, const size_t proof_size);
bool _sec_hash(void const* region, const size_t region_size, void *hash, const size_t hash_size);

bool _sec_encrypt(void const* plain_blck, void *enc_blck, const size_t plain_size, size_t *enc_size);
bool _sec_decrypt(void const* enc_blck, void *plain_blck, const size_t enc_size, size_t *plain_size);