#ifndef CSTRUCTURES_HASH_H
#define CSTRUCTURES_HASH_H

#include "cstructures/config.h"
#include <stdint.h>

C_BEGIN

typedef uint32_t hash32_t;
typedef hash32_t (*hash32_func)(const void*, uintptr_t);

CSTRUCTURES_PUBLIC_API hash32_t
hash32_jenkins_oaat(const void* key, uintptr_t len);

CSTRUCTURES_PUBLIC_API hash32_t
hash32_ptr(const void* ptr, uintptr_t len);

CSTRUCTURES_PUBLIC_API hash32_t
hash32_aligned_ptr(const void* ptr, uintptr_t len);

/*!
 * @brief Taken from boost::hash_combine. Combines two hash values into a
 * new hash value.
 */
CSTRUCTURES_PUBLIC_API hash32_t
hash32_combine(hash32_t lhs, hash32_t rhs);

C_END

#endif /* CSTRUCTURES_HASH_H */
