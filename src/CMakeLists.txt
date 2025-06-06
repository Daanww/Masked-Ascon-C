cmake_minimum_required(VERSION 3.10)
project(protected_bi32_rv32i)

add_executable(protected_bi32_rv32i aead.c constants.c crypto_aead.c interleave.c permutations.c printstate.c print_simulation.c shares.c
        demo.h)

include_directories(../tests)
