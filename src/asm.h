#ifndef ASM_H_
#define ASM_H_

#ifndef __GNUC__
#define __asm__ asm
#endif

//#define LDR(rd, ptr, offset) rd = *((int*)((char*)ptr + offset))

#define LDR(rd, ptr, offset) \
  __asm__ volatile("lw %0, %2(%1)\n\t" : "=r"(rd) : "r"(ptr), "ri"(offset))

//#define STR(rd, ptr, offset) *((int*)((char*)ptr + offset)) = rd

#define STR(rd, ptr, offset)                                                \
  __asm__ volatile("sw %0, %2(%1)\n\t" ::"r"(rd), "r"(ptr), "ri"(offset) \
                   : "memory")


//#define MOV(rd, imm) rd = imm

#define MOV(rd, imm) __asm__ volatile("addi %0, zero, %1\n\t" : "=r"(rd) : "i"(imm))

#define ROR(rd, rn, imm) rd = (rn >> imm) | (rn << (-imm & 31))

/*
#define ROR(rd, rn, imm) \
    do {                 \
        volatile int tmp;\
        __asm__ volatile(\
            "srli %[rd_], %[rn_], (%[imm_] & 31) # & 31 because gnu assembler does not implicitly truncate the shift amount \n\t"  \
            "slli %[tmp_], %[rn_], (-%[imm_] & 31) \n\t" \
            "or   %[rd_], %[rd_], %[tmp_] \n\t"\
            : [rd_] "+r"(rd), [rn_] "+r"(rn), [tmp_] "+r"(tmp) :  [imm_] "i"(imm));\
    } while (0)
*/

/*
#define ROR(rd, rn, imm) \
    do {                 \
    rd = (rn >> (imm & 31)) | (rn << (-imm & 31)); \
    rn = (rn << (-imm & 31)); \
    } while (0)
*/

/*
#define EOR_ROR(rd, rn, rm, imm) \
    do {                         \
        ROR(rd, rm, imm);        \
        rd ^= rn;               \
    } while (0)
*/
/*
#define EOR_ROR(rd, rn, rm, imm) \
    do {                         \
        ROR(rd, rm, imm);        \
        __asm__ volatile(\
            "xor %[rd_], %[rd_], %[rn_] \n\t"  \
            : [rd_] "+r"(rd) : [rn_] "r"(rn));\
    } while (0)
*/

#define EOR_ROR(rd, rn, rm, imm) rd = rn ^ ((rm >> imm) | (rm <<(-imm & 31)))
/*
#define EOR_AND_ROR(ce, ae, be, imm, tmp) \
        __asm__ volatile(                 \
            "srli %[tmp_], %[be_], (%[i1_] & 31) # & 31 because gnu assembler does not implicitly truncate the shift amount \n\t"  \
            "slli %[tmp_], %[rn_], (-%[i1_] & 31) \n\t" \
            "or   %[rd_], %[rd_], %[tmp_] \n\t"                              \
            "and %[tmp_], %[ae_], %[tmp_] \n\t"  \
            "xor %[ce_], %[tmp_], %[ce_] \n\t"    \
            : [ce_] "+r"(ce), [tmp_] "=r"(tmp) : [ae_] "r"(ae), [be_] "r"(be), [i1_] "i"(ROT(imm)))\
*/


#define EOR_AND_ROR(ce, ae, be, imm, tmp) \
    do {                                  \
        ROR(tmp, be, ROT(imm));                \
        __asm__ volatile(                       \
            "and %[tmp_], %[ae_], %[tmp_] \n\t"  \
            "xor %[ce_], %[tmp_], %[ce_] \n\t"    \
            : [ce_] "+r"(ce), [tmp_] "+r"(tmp) : [ae_] "r"(ae));\
    } while (0)

#define EOR_BIC_ROR(ce, ae, be, imm, tmp) \
    do {                                  \
        ROR(tmp, be, ROT(imm));                \
        __asm__ volatile(                 \
            "xori %[tmp_], %[tmp_], -1 \n\t"     \
            "and %[tmp_], %[ae_], %[tmp_] \n\t"  \
            "xor %[ce_], %[tmp_], %[ce_] \n\t"    \
            : [ce_] "+r"(ce), [tmp_] "+r"(tmp) : [ae_] "r"(ae));\
    } while (0)

#define EOR_ORR_ROR(ce, ae, be, imm, tmp) \
    do {                                  \
        ROR(tmp, be, ROT(imm));                \
        __asm__ volatile(                       \
            "or  %[tmp_], %[ae_], %[tmp_] \n\t"  \
            "xor %[ce_], %[tmp_], %[ce_] \n\t"    \
            : [ce_] "+r"(ce), [tmp_] "+r"(tmp) : [ae_] "r"(ae));\
    } while (0)


#endif  // ASM_H_
