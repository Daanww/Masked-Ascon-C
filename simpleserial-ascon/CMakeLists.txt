add_definitions(-DSS_VER=1 -DHAL_TYPE=HAL_ibex)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../../vendor/newae/crypto)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../../vendor/newae/hal)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../../vendor/newae/simpleserial)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../common)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../../../src)

add_library(simpleserial
${CMAKE_CURRENT_SOURCE_DIR}/../../../../vendor/newae/simpleserial/simpleserial.c
${CMAKE_CURRENT_SOURCE_DIR}/../../../../vendor/newae/hal/ibex/ibex_hal.c
)

add_executable(simpleserial-ascon simpleserial-ascon.c aead.c constants.c crypto_aead.c interleave.c permutations.c printstate.c print_simulation.c shares.c)

target_link_libraries(simpleserial-ascon common simpleserial)



