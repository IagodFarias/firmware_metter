#include "aes256.h"    // Your existing AES256 driver
#include "hw_memmap.h" // Hardware memory map definitions
#include <stdint.h>
#include <stdbool.h>

#define BLOCK_SIZE 16
#define CONST_RB 0x87

void cmac_init();
bool cmac_update(uint8_t *cmac_out);
bool cmac_isNonVerified();


// Function to calculate CMAC from memory
bool calcCmacFromMem(uint32_t ul_offset,
                               uint32_t ul_length,
                               const uint8_t *aes_key,
                               uint8_t *cmac_out);

static inline bool calcCmacFromMemory(void);
