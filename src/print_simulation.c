#include <stdio.h>

#ifndef GENKAT_TEST
#include "demo_system.h"
#endif

#include "randombytes.h"
#include <string.h>
#include <inttypes.h>
#include <stdarg.h>  // for variable arguments
#include "stdlib.h"

#include "api.h"

#include "crypto_aead.h"

#include "ascon.h"
#include "shares.h"
#include "word.h"

#include "bendian.h"
#include "forceinline.h"
#include "interleave.h"

#include "permutations.h"

uint32_t offset = 0;
ascon_state_t old_state = {0};
uint64_t old_sums[6] = {0};



// Print a char as a hexadecimal number using putchar()
// The lowest 2 bits of the colour_code determine if the Specific ASCII characters will be coloured or not.
// The lowest of the 2 will determine the *left* most character, the second lowest will determine the *right* most character.
// The switch between left and right is because of endianness
void puthex_char(unsigned char h, uint16_t colour_code) {
    // ANSI escape code for red text
    const char* red_start = "\033[31m";
    // ANSI escape code to reset color
    const char* color_reset = "\033[0m";



    int cur_digit;
    // Iterate through h taking top 4 bits each time and outputting ASCII of hex
    // digit for those 4 bits
    for (int i = 0; i < 2; i++) {
        cur_digit = h >> 4;

        if(i == 0 && colour_code & 1) puts(red_start);
        if(i == 1 && colour_code & 2) puts(red_start);

        if (cur_digit < 10)
            putchar('0' + cur_digit);
        else
            putchar('A' - 10 + cur_digit);

        puts(color_reset);

        h <<= 4;
    }
}

// Print a uint32_t as a decimal number using putchar()
void putdec(uint32_t n) {
    char buffer[11]; // Maximum number of digits in a uint32_t is 10 (4294967295)
    int i = 0;

    // Handle the case when n is 0 separately
    if (n == 0) {
        putchar('0');
        return;
    }

    // Store each digit in reverse order in the buffer
    while (n != 0) {
        buffer[i] = '0' + (n % 10);
        i += 1;
        n /= 10;
    }

    // Print the digits in correct order
    while (i > 0) {
        i -= 1;
        putchar(buffer[i]);
    }
}

void print_char_array_in_hex_little_endian(const char *name, uint8_t *array, int len) {
    puts(name);
    puts(" = ");
    for (int i = 0; i < len; i++) {
        puthex_char(array[i],0);
        puts(" ");
    }
    puts("\n");
}

void print_char_array_in_hex_with_name(const char *name, uint8_t *array, int len) {
    puts(name);
    puts(" = ");
    for (int i = len - 1; i >= 0; i--) {
        if((len - 1 - i) % 4 == 0 && i != len - 1) {
            puts( "| ");
        }
        puthex_char(array[i],0);
        puts(" ");
    }
    puts("\n");
}

void print_char_array_in_hex(uint8_t *array, int len) {
    for (int i = len - 1; i >= 0; i--) {
        if((len - 1 - i) % 4 == 0 && i != len - 1) {
            puts( "| ");
        }
        puthex_char(array[i],0);
        puts(" ");
    }
}

void print_state_share_in_hex(share_t *share, uint16_t colour_code) {

    for (int i = 8 - 1; i >= 0; i--) {
        if(i == 3) {
            puts( "| ");
        }
        puthex_char(((uint8_t *)share)[i],colour_code);
        colour_code = colour_code >> 2;
        puts(" ");
    }
}

void my_printf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    char buffer[1024];
    char *buf_ptr = buffer;
    const char *fmt_ptr = format;
    int int_val;
    char char_val;
    char *str_val;

    while (*fmt_ptr != '\0') {
        if (*fmt_ptr == '%') {
            fmt_ptr++;
            switch (*fmt_ptr) {
                case 'd':
                    int_val = va_arg(args, int);
                    buf_ptr += sprintf(buf_ptr, "%d", int_val);
                    break;
                case 'c':
                    char_val = (char) va_arg(args, int);  // char is promoted to int in va_arg
                    *buf_ptr++ = char_val;
                    break;
                case 's':
                    str_val = va_arg(args, char*);
                    while (*str_val) {
                        *buf_ptr++ = *str_val++;
                    }
                    break;
                default:
                    *buf_ptr++ = '%';
                    *buf_ptr++ = *fmt_ptr;
                    break;
            }
        } else {
            *buf_ptr++ = *fmt_ptr;
        }
        fmt_ptr++;
    }

    *buf_ptr = '\0';  // Null-terminate the buffer
    puts(buffer);     // Print the final buffer
    va_end(args);
}

// Convert a 64 bit delta between two values to a colour_code for colouring the hex characters of the newer of the two values that made the delta.
// Essentially the 64 bit of data is compressed to 16 bits by taking 4 bits of the original and doing an OR between all 4 to end up with 1 bit for the output.
// During the process the output is also reversed to facilitate easier printing of the hex characters.
uint16_t convert_delta_to_colour_code(uint64_t d) {
    uint16_t colour_code = 0;
    for(int i = 0; i < 16; i++) {
        colour_code = colour_code << 1;
        if(d & 0b1111) colour_code = colour_code | 0x0001;
        d = d >> 4;
    }
    return colour_code;
}

void print_state(const char *name, ascon_state_t *s) {
    puts("[");
    puts(name);
    puts("]: {\n");
    for (int i = 0; i < 6; i++) {
        my_printf("  x%d:\n", i);
        uint64_t sum = 0;
        for(int share = 0; share < NUM_SHARES_KEY; share++) {
            my_printf("    share %d = ", share);
            uint64_t share_delta = *((uint64_t*)&s->x[i].s[share]) ^ *((uint64_t*)&old_state.x[i].s[share]);
            uint16_t colour_code = convert_delta_to_colour_code(share_delta);
            print_state_share_in_hex(&s->x[i].s[share], colour_code);
            puts("\n");
            uint64_t temp = s->x[i].s[share].w[1];
            temp = temp << 32 | s->x[i].s[share].w[0];
            sum ^= temp;
        }
        my_printf("        sum = ");
//        print_char_array_in_hex("sum", (uint8_t *) &sum, 8);
        uint64_t sum_delta = sum ^ old_sums[i];
        uint16_t colour_code = convert_delta_to_colour_code(sum_delta);
        print_state_share_in_hex((share_t *) &sum, colour_code);
        puts("\n");
//        print_char_array_in_hex("delta", (uint8_t*)&delta, 8);
//        print_char_array_in_hex_little_endian("delta", (uint8_t*)&delta, 8);
//        print_char_array_in_hex("colour_code", (uint8_t*)&colour_code, 2);
//        print_char_array_in_hex_little_endian("colour_code", (uint8_t*)&colour_code, 2);
        old_sums[i] = sum;
    }
    puts("}\n");
    old_state = *s;
}

// Print the shares of data, such as key, nonce, ad etc. len is in bytes as usual.
void print_data_shares_in_hex(mask_npub_uint32_t *s, int len) {

    for(int share = 0; share < NUM_SHARES_KEY; share++) {
        my_printf("share %d: ", share);
        for(int i = len - 1; i >= 0; i--) {
            uint32_t t = s[i].shares[share];
            print_char_array_in_hex((uint8_t*)&t, 4);
            if(i != 0) puts("| ");
        }

        puts("\n");
    }
}


void generate_shares_with_print(uint32_t *s, int num_shares, const uint8_t *data,
                                uint64_t len) {
    uint32_t rnd0, rnd1, i;
    uint64_t rnd;


    my_printf("Generating shares!\n\nNUM_WORDS(len) = %d\n|-------------------------------|\n", NUM_WORDS(len));



    /* generate random shares */
    my_printf("|-Generating random numbers:-|\n\n");
    for (i = 0; i < NUM_WORDS(len); i += 2) {
        my_printf("Iteration %d:\n\n", i);

        s[(i + 0) * num_shares + 0] = 0;
        s[(i + 1) * num_shares + 0] = 0;
        for (int d = 1; d < num_shares; ++d) {
            my_printf("  Share %d:\n\n", d);
            RND(rnd0);
            RND(rnd1);
            //my_printf("  rnd0: %d, rnd1: %d\n", rnd0, rnd1);
            print_char_array_in_hex_with_name("rnd0", (uint8_t *) &rnd0, 4);
            print_char_array_in_hex_with_name("rnd1", (uint8_t *) &rnd1, 4);

            s[(i + 0) * num_shares + d] = rnd0;
            s[(i + 1) * num_shares + d] = rnd1;
#if ASCON_EXTERN_BI
            s[(i + 0) * num_shares + 0] ^= ROR32(rnd0, ROT(d));
            s[(i + 1) * num_shares + 0] ^= ROR32(rnd1, ROT(d));
#else
            rnd = ROR64((uint64_t) rnd1 << 32 | rnd0, ROT(2 * d));
            print_char_array_in_hex_with_name("rnd ", (uint8_t *) &rnd, 8);
            uint64_t test = (uint64_t) rnd1 << 32 | rnd0;
            uint64_t test2 = ROR64((uint64_t) rnd1 << 32 | rnd0, 10);
            print_char_array_in_hex_with_name("test", (uint8_t *) &test, 8);
            print_char_array_in_hex_with_name("test2", (uint8_t *) &test2, 8);
            s[(i + 0) * num_shares + 0] ^= (uint32_t) rnd;
            s[(i + 1) * num_shares + 0] ^= (uint32_t) (rnd >> 32);
            print_char_array_in_hex_with_name("s[(i + 0) * num_shares + 0]", (uint8_t *) &s[(i + 0) * num_shares + 0],
                                              4);
            print_char_array_in_hex_with_name("s[(i + 1) * num_shares + 0]", (uint8_t *) &s[(i + 1) * num_shares + 0],
                                              4);
            uint64_t test4 = 65;
            print_char_array_in_hex_with_name("test4", (uint8_t *) &test4, 8);
            uint64_t deadbeef = 0xCAFEABCDDEADBEEF;
            puts("\n");
            print_char_array_in_hex_with_name("deadbeef", (uint8_t *) &deadbeef, 8);
#endif
        }
    }
    /* mask complete words */
    my_printf("|-Masking words:-|\n\n");
    for (i = 0; i < len / 8; ++i) {
        my_printf("Iteration %d:\n\n", i);
        uint64_t x;
        memcpy(&x, data + i * 8, 8);
        print_char_array_in_hex_with_name("x", (uint8_t *) &x, 8);
        x = U64BIG(x);
        print_char_array_in_hex_with_name("x", (uint8_t *) &x, 8);

#if ASCON_EXTERN_BI
        x = TOBI(x);
#endif
        s[(2 * i + 0) * num_shares + 0] ^= (uint32_t) x;
        s[(2 * i + 1) * num_shares + 0] ^= (uint32_t) (x >> 32);
        print_char_array_in_hex_with_name("s[(2 * i + 0) * num_shares + 0]",
                                          (uint8_t *) &s[(2 * i + 0) * num_shares + 0], 4);
        print_char_array_in_hex_with_name("s[(2 * i + 1) * num_shares + 0]",
                                          (uint8_t *) &s[(2 * i + 1) * num_shares + 0], 4);

    }
    /* mask remaining bytes */
    if ((len / 8 * 8) != len) {
        uint64_t x = 0;
        for (i = (len / 8) * 8; i < len; ++i) {
            x ^= (uint64_t) data[i] << ((i % 8) * 8);
        }
        x = U64BIG(x);
#if ASCON_EXTERN_BI
        x = TOBI(x);
#endif
        s[(2 * (len / 8) + 0) * num_shares + 0] ^= (uint32_t) x;
        s[(2 * (len / 8) + 1) * num_shares + 0] ^= (uint32_t) (x >> 32);
    }
}


