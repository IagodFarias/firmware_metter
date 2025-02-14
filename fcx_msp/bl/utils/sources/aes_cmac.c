#include "aes256.h"    // Your existing AES256 driver
#include "hw_memmap.h" // Hardware memory map definitions
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "systick.h"
#include "processing.h"

/** Tick which a new log must be created. */
static uint64_t gull_tickToCmac = SYSTICK_INVALID_TICK;
static uint8_t aes_key_main[16] = {0xFF, 0xFF, 0xAA, 0xAA, 0xFF, 0xFF, 0xAA, 0xAA, 0xFF, 0xFF, 0xAA, 0xAA, 0xFF, 0xFF, 0xAA, 0xAA};
bool gb_isNonVerified = false;
bool b_isTimeToCmac = false;

#define BLOCK_SIZE 16
#define CONST_RB 0x87

static inline bool isTimeToCmac();
static void cmac_updateTimeToCmac();

// Function prototypes
static void generate_subkeys(uint16_t baseAddress, const uint8_t *aes_key, uint8_t *K1, uint8_t *K2);
static void left_shift_one_bit(const uint8_t *input, uint8_t *output);

// Helper function to perform left shift by one bit
static void left_shift_one_bit(const uint8_t *input, uint8_t *output) {
    uint8_t overflow = 0;
    for (int i = BLOCK_SIZE - 1; i >= 0; i--) {
        output[i] = (input[i] << 1) | overflow;
        overflow = (input[i] & 0x80) ? 1 : 0;
    }
}

#if DEBUG_INTEGRITY
// Helper function to print a byte array in hex
static void print_hex(const char *label, const uint8_t *data, size_t len) {
    printf("(%s): ", label);
    for(size_t i = 0; i < len; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
}
#endif

// Helper function to generate subkeys K1 and K2
static void generate_subkeys(uint16_t baseAddress, const uint8_t *aes_key, uint8_t *K1, uint8_t *K2) {
    uint8_t zero_block[BLOCK_SIZE] = {0};
    uint8_t L[BLOCK_SIZE];

    // Load AES key for AES-128
    AES256_setCipherKey(baseAddress, aes_key, AES256_KEYLENGTH_128BIT);

#if DEBUG_INTEGRITY
    print_hex("AES Key", aes_key, 16);
#endif

    // Encrypt zero block to get L
    AES256_encryptData(baseAddress, zero_block, L);

#if DEBUG_INTEGRITY
    print_hex("Encrypted Zero Block (L)", L, BLOCK_SIZE);
#endif

    // Generate K1 = L << 1
    left_shift_one_bit(L, K1);
    if (L[0] & 0x80) {
        K1[BLOCK_SIZE - 1] ^= CONST_RB;
    }

#if DEBUG_INTEGRITY
    print_hex("Subkey K1", K1, BLOCK_SIZE);
#endif

    // Generate K2 = K1 << 1
    left_shift_one_bit(K1, K2);
    if (K1[0] & 0x80) {
        K2[BLOCK_SIZE - 1] ^= CONST_RB;
    }

#if DEBUG_INTEGRITY
    print_hex("Subkey K2", K2, BLOCK_SIZE);
#endif
}

// Main function to calculate CMAC
bool calcCmacFromMem(uint32_t ul_offset,
                     uint32_t ul_length,
                     const uint8_t *aes_key,
                     uint8_t *cmac_out) {
    uint16_t baseAddress = AES256_BASE; // Define your AES base address
    uint8_t K1[BLOCK_SIZE], K2[BLOCK_SIZE];
    uint8_t M_last[BLOCK_SIZE];
    uint8_t cmac_prev[BLOCK_SIZE] = {0}; // Initialize to zero
    uint8_t temp_out[BLOCK_SIZE];
    bool complete_block;
    uint8_t *data_ptr = (uint8_t *)ul_offset;
    uint32_t n_blocks = ul_length / BLOCK_SIZE;
    uint32_t remaining = ul_length % BLOCK_SIZE;

#if DEBUG_INTEGRITY
    printf("(OPTCOMM): Starting CMAC Calculation\n");
    print_hex("Data Slice", data_ptr, ul_length);
    printf("(OPTCOMM): Checksum Offset: %u\n", ul_offset);
    printf("(OPTCOMM): Checksum Length: %u\n", ul_length);
#endif

    // Generate subkeys K1 and K2
    generate_subkeys(baseAddress, aes_key, K1, K2);

    // Determine if the last block is complete
    if (remaining == 0 && ul_length != 0) {
        complete_block = true;
    } else {
        complete_block = false;
        n_blocks += 1;
    }

#if DEBUG_INTEGRITY
    printf("(OPTCOMM): Number of Blocks: %u\n", n_blocks);
    printf("(OPTCOMM): Complete Block: %s\n", complete_block ? "True" : "False");
#endif

    // Process all blocks except the last
    for (uint32_t i = 0; i < n_blocks - 1; i++) {
        uint8_t block[BLOCK_SIZE];
        memcpy(block, data_ptr + (i * BLOCK_SIZE), BLOCK_SIZE);

#if DEBUG_INTEGRITY
        printf("(OPTCOMM): Processing Block %u\n", i + 1);
        print_hex("Block Before XOR", block, BLOCK_SIZE);
#endif

        // XOR with previous CMAC
        for (int j = 0; j < BLOCK_SIZE; j++) {
            block[j] ^= cmac_prev[j];
        }

#if DEBUG_INTEGRITY
        print_hex("Block After XOR with Previous CMAC", block, BLOCK_SIZE);
#endif

        // Encrypt the block
        AES256_encryptData(baseAddress, block, temp_out);

#if DEBUG_INTEGRITY
        print_hex("Encrypted Block", temp_out, BLOCK_SIZE);
#endif

        // Update cmac_prev with the encrypted output
        memcpy(cmac_prev, temp_out, BLOCK_SIZE);
    }

    // Handle the last block
    if (complete_block && ul_length != 0) {
        // Last block is complete; XOR with K1
        memcpy(M_last, data_ptr + ((n_blocks - 1) * BLOCK_SIZE), BLOCK_SIZE);
        for (int j = 0; j < BLOCK_SIZE; j++) {
            M_last[j] ^= K1[j];
        }

#if DEBUG_INTEGRITY
        printf("(OPTCOMM): Last Block is complete. XOR with K1.\n");
        print_hex("M_last After XOR with K1", M_last, BLOCK_SIZE);
#endif
    } else {
        // Last block is not complete; apply padding and XOR with K2
        memset(M_last, 0, BLOCK_SIZE);
        if (ul_length > 0){
            memcpy(M_last, data_ptr + ((n_blocks - 1) * BLOCK_SIZE), remaining);
        }
        M_last[remaining] = 0x80; // Append '1' bit followed by '0's
        // The rest is already zeroed by memset

#if DEBUG_INTEGRITY
        printf("(OPTCOMM): Last Block is not complete. Applying padding and XOR with K2.\n");
        print_hex("M_last After Padding", M_last, BLOCK_SIZE);
#endif

        for (int j = 0; j < BLOCK_SIZE; j++) {
            M_last[j] ^= K2[j];
        }

#if DEBUG_INTEGRITY
        print_hex("M_last After XOR with K2", M_last, BLOCK_SIZE);
#endif
    }

    if(ul_length != 0){
        // XOR the last block with the previous CMAC
        for (int j = 0; j < BLOCK_SIZE; j++) {
            M_last[j] ^= cmac_prev[j];
        }

#if DEBUG_INTEGRITY
        print_hex("M_last After XOR with Previous CMAC", M_last, BLOCK_SIZE);
#endif

        // Encrypt the last block to get the final CMAC
        AES256_encryptData(baseAddress, M_last, cmac_out);

#if DEBUG_INTEGRITY
        print_hex("Final CMAC", cmac_out, BLOCK_SIZE);
#endif
    }
    else{
        // If ul_length is 0, define CMAC as zero block or handle accordingly
        memset(cmac_out, 0, BLOCK_SIZE);
#if DEBUG_INTEGRITY
        printf("(OPTCOMM): ul_length is 0. CMAC set to zero block.\n");
        print_hex("CMAC_out", cmac_out, BLOCK_SIZE);
#endif
    }

    return true; // Return true on success
}

void cmac_init(){

    if (gull_tickToCmac == SYSTICK_INVALID_TICK){ // only init, not every reboot.
        cmac_updateTimeToCmac();
    }
}

static void cmac_updateTimeToCmac(){
    static struct tm *gps_timestampcmac;

    // Get current system timestamp
    gps_timestampcmac = systick_getActualDateTime();

    gps_timestampcmac->tm_hour += 24;

    // OFFSET correction given default Texas Implementation of Timezones
    gps_timestampcmac->tm_hour -= SYSTICK_CTS_TO_UTC_OFFSET;

    // Get absolute system tick for the given calendar
    gull_tickToCmac = (uint64_t) mktime(gps_timestampcmac) * 1000;

}

static inline bool isTimeToCmac()
{
    // Get current system tick
    uint64_t ull_systick = systick_getSysTickMs();

    // Check if it's time to register a log
    if(ull_systick >= gull_tickToCmac)
    {
        b_isTimeToCmac = true;
    }

    return b_isTimeToCmac;
}

bool cmac_update(uint8_t *cmac_test){
    uint8_t cmac_out[16] = {0};

    if(isTimeToCmac()){
        b_isTimeToCmac = false;
        bool b_isEqual = true;
        calcCmacFromMem(0x4000, 0x5500, aes_key_main, cmac_out);
        for(int i = 0; i < 16; i++){
            if(cmac_test[i] != cmac_out[i]){
                b_isEqual = false;
            }
        }
        if(!b_isEqual){
            gb_isNonVerified = true;
            proc_disableProcessing();
            return false;
        }
        else{
        cmac_updateTimeToCmac();
        }
    }


    return true;
}

bool cmac_isNonVerified(){
    return gb_isNonVerified;
}
