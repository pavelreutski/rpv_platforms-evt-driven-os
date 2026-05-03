#pragma once

#include <stddef.h>

/**
 * @defgroup sec_execution Security Execution
 * @brief Core security execution and lifecycle entry points.
 *
 * These functions define the main runtime behavior of the security component.
 * They are responsible for initializing and continuously enforcing protection
 * over the designated memory region.
 *
 * @{
 */

/**
 * @brief Initialize the security context.
 *
 * Establishes the security context for a protected memory region. This function
 * acts as a "guardian constructor", preparing any internal state required to
 * monitor and verify the integrity of the region during runtime.
 *
 * @param[in] protected_region Pointer to the memory region to protect.
 * @param[in] region_size Size of the protected region in bytes.
 *
 * @note Must be called before `_sec_runtime`.
 *
 * @note Typically computes and stores initial reference data (e.g., hash,
 *       authentication state) for later verification.
 *
 * @warning The provided memory region must remain valid and unchanged unless
 *          explicitly allowed by the security policy.
 */
void _sec_context(void const* protected_region, const size_t region_size);

/**
 * @brief Execute the security runtime loop.
 *
 * Runs the main security "guardian" logic responsible for continuous monitoring
 * and enforcement of the protected region's integrity.
 *
 * @note Must be called after `_sec_context` has initialized the system.
 *
 * @note This function may:
 *       - Periodically verify integrity of the protected region
 *       - Detect tampering or corruption
 *       - Trigger recovery via `_sec_recovery` if needed
 *
 * @note Intended usage patterns:
 *       - Infinite loop in a dedicated task/thread
 *       - Periodic invocation from a timer/interrupt
 *
 * @warning The execution frequency directly impacts detection latency and
 *          system performance. Choose an appropriate scheduling strategy.
 */
void _sec_runtime(void);

/** @} */ /* end of sec_execution */