#pragma once

#include <stdint.h>
#include <stddef.h>

/**
 * @defgroup sec_storage Security Storage Callbacks
 * @brief User-provided secure storage interface.
 *
 * These callbacks must be implemented by the consumer to provide persistent
 * storage for security-critical data (e.g., keys, reference hashes, state).
 *
 * The security component relies on this interface to maintain integrity across
 * resets and power cycles where it persists any plain or encrypted security artifacts.
 *
 * @note Implementations may use general purpose or OTP flash region, EEPROM, HSM (e.g. TPM 2.0 compliant) SoC`s internal/external chip, or other
 *       non-volatile storage mechanisms.
 *
 * @{
 */

/**
 * @brief Read a block from secure storage.
 *
 * Retrieves a stored data block identified by a unique ID.
 *
 * @param[in] blck_uid Identifier of the storage block.
 * @param[out] blck Buffer where the retrieved data will be stored.
 * @param[in] blck_size Size of the buffer in bytes.
 *
 * @return true on success, false on failure.
 *
 * @note This is a component-provided callback.
 *
 * @warning The implementation must ensure:
 *          - Data integrity (detect corruption)
 *          - Correct block mapping (UID isolation)
 *          - No leakage of sensitive data outside the buffer
 */
bool _sec_storeread(uint8_t blck_uid, void *blck, const size_t blck_size);

/**
 * @brief Write a block to secure storage.
 *
 * Stores a data block identified by a unique ID.
 *
 * @param[in] blck_uid Identifier of the storage block.
 * @param[in] blck Buffer containing data to store.
 * @param[in] blck_size Size of the data in bytes.
 *
 * @return true on success, false on failure.
 *
 * @note This is a component-provided callback.
 *
 * @warning The implementation must ensure:
 *          - Atomicity (no partial writes on failure)
 *          - Data integrity (e.g., checksum, ECC, redundancy)
 *          - Wear management if using flash memory
 *          - Protection against unauthorized modification
 */
bool _sec_storewrite(uint8_t blck_uid, void const* blck, const size_t blck_size);

/** @} */ /* end of sec_storage */