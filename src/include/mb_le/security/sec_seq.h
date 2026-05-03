#pragma once

#include <stddef.h>
#include <stdint.h>

/**
 * @defgroup sec_utils Security Utilities
 * @brief Utility functions provided by the security component.
 *
 * These functions offer auxiliary capabilities such as generation of noise-alike sequences based on either 
 * pseudo-random (PRNG) or true random (RNG) generators. They are optional and are not required for the core security flow.
 *
 * @note These functions are implemented by the security component.
 *
 * @warning Unless explicitly stated otherwise, these utilities are not guaranteed
 *          to be cryptographically secure.
 *
 * @{
 */

/**
 * @brief Seed the internal pseudo-random number generator (PRNG).
 *
 * Initializes the PRNG state used by `_sec_genprngseq`.
 *
 * @param[in] seed Seed value for the PRNG.
 *
 * @note A deterministic seed will produce deterministic output sequences.
 *
 * @warning For security-sensitive use, the seed must contain high entropy.
 *          Otherwise, generated sequences may be predictable.
 */
void _sec_prngseed(uint32_t seed);

/**
 * @brief Generate a pseudo-random byte sequence.
 *
 * Produces a noise-like sequence of bytes using the internal PRNG.
 *
 * @param[out] seq Output buffer for the generated sequence.
 * @param[in] seq_size Number of bytes to generate.
 *
 * @note This function is intended for non-critical uses such as testing,
 *       obfuscation, or simulation. Or if applicable by the security policy and demands 
 *       to achieve a lightweight context can be used by '_sec_nonce' implementation
 *
 * @warning The generated sequence may not AES compliant.
 *          Use AES compliant algorithms for security-critical randomness or true RNG generator
 */
void _sec_genprngseq(void *seq, const size_t seq_size);

/** @} */ /* end of sec_utils */