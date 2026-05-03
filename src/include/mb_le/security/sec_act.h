#pragma once

#include <stddef.h>
#include <stdbool.h>

/**
 * @defgroup sec_callbacks Security Callbacks
 * @brief consumer-provided security primitives required by the component.
 *
 * These callbacks must be implemented by the consumer (application or platform)
 * to provide cryptographic and security-related functionality.
 *
 * The library relies on these functions for secure operation. Incorrect or
 * insecure implementations may compromise the entire system.
 *
 * @{
 */

/**
 * @brief Security recovery callback.
 *
 * This function is implemented by the consumer to perform recovery of the
 * security subsystem.
 *
 * @note This is a consumer-provided callback.
 *
 * @warning The implementation must ensure the system is returned to a secure
 *          and consistent state. It is expected that this callback ill not return. 
 *          If it returns the security component still assumes a security breach
 */
void _sec_recovery(void);

/**
 * @brief Nonce generation callback.
 *
 * Generates a cryptographically secure nonce.
 *
 * @param[out] nonce Buffer where the nonce will be written.
 * @param[in] nonce_size Size of the nonce buffer in bytes.
 *
 * @return true on success, false on failure.
 *
 * @note This is a consumer-provided callback.
 *
 * @warning The nonce must be unpredictable and unique. Use a cryptographically
 *          secure random number generator (CSPRNG).
 */
bool _sec_nonce(void *nonce, const size_t nonce_size);

/**
 * @brief Region verification callback.
 *
 * Verifies integrity and/or authenticity of a protected memory region and gives
 * a its proof on verification success.
 *
 * @param[in] region_proof Verification proof (e.g., MAC, signature).
 * @param[in] protected_region Memory region to verify.
 * @param[in] proof_size Size of the proof in bytes.
 *
 * @return true if verification succeeds, false otherwise.
 *
 * @note This is a consumer-provided callback.
 *
 * @warning The verification must be resistant to forgery and timing attacks
 *          where applicable. AES algorithms usage expected.
 */
bool _sec_verify(void *region_proof, void const* protected_region, const size_t proof_size);

/**
 * @brief Hash computation callback.
 *
 * Computes a cryptographic hash of the input region.
 *
 * @param[in] region Input data.
 * @param[in] region_size Size of input data.
 * @param[out] hash Output buffer for the hash.
 * @param[in] hash_size Size of the hash buffer.
 *
 * @return true on success, false on failure.
 *
 * @note This is a consumer-provided callback.
 *
 * @warning Use a secure cryptographic hash function (e.g., SHA-256 or stronger).
 *          The output size must match the selected algorithm.
 */
bool _sec_hash(void const* region, const size_t region_size, void *hash, const size_t hash_size);

/**
 * @brief Encryption callback.
 *
 * Encrypts plaintext into ciphertext.
 *
 * @param[in] plain_blck Input plaintext buffer.
 * @param[out] enc_blck Output buffer for ciphertext.
 * @param[in] plain_size Size of plaintext.
 * @param[in] enc_size Size of ciphertext buffer.
 *
 * @return true on success, false on failure.
 *
 * @note This is a user-provided callback.
 *
 * @warning The implementation must use a secure encryption scheme (e.g., AES-GCM,
 *          ChaCha20-Poly1305). Ensure proper handling of IVs/nonces and authentication.
 */
bool _sec_encrypt(void const* plain_blck, void *enc_blck, const size_t plain_size, const size_t enc_size);

/**
 * @brief Decryption callback.
 *
 * Decrypts ciphertext into plaintext.
 *
 * @param[in] enc_blck Input ciphertext buffer.
 * @param[out] plain_blck Output buffer for plaintext.
 * @param[in] enc_size Size of ciphertext.
 * @param[in] plain_size Size of plaintext buffer.
 *
 * @return true on success, false on failure.
 *
 * @note This is a user-provided callback.
 *
 * @warning The implementation must validate authenticity (if applicable) before
 *          returning decrypted data. Do not output plaintext if verification fails.
 */
bool _sec_decrypt(void const* enc_blck, void *plain_blck, const size_t enc_size, const size_t plain_size);

/** @} */  /* end of sec_callbacks */