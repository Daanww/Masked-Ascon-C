#!/bin/bash



#copying the custom CMakeLists and gcc_toolchain.cmake
cp -f src/CMakeLists_Custom.txt ibex-demo-system/sw/c/CMakeLists.txt
cp -f src/gcc_toolchain_Custom.cmake ibex-demo-system/sw/c/gcc_toolchain.cmake

# 2 shares
cp -f src/api_2_shares.h src/api.h

rm -rf ibex-demo-system/sw/c/demo/simpleserial-ascon
cp -r simpleserial-ascon ibex-demo-system/sw/c/demo/
cp simpleserial-ascon/CMakeLists_demo.txt ibex-demo-system/sw/c/demo/CMakeLists.txt
cp -t ibex-demo-system/sw/c/demo/simpleserial-ascon/ src/*.c
cp -t ibex-demo-system/sw/c/demo/simpleserial-ascon/ src/*.h
mkdir -p ibex-demo-system/sw/c/build
rm -r ibex-demo-system/sw/c/build/*
pushd ibex-demo-system/sw/c/build
cmake -DCMAKE_BUILD_TYPE=Release -DSIM_CTRL_OUTPUT:BOOL=ON -DRANDOM_SEED=37 ..
make -j $(nproc)
popd
pushd ibex-demo-system/
# ./build/lowrisc_ibex_demo_system_0/sim-verilator/Vibex_demo_system \
#./build/lowrisc_ibex_demo_system_0/sim-verilator/Vtop_verilator \
#  -t --meminit=ram,./sw/c/build/ascon/protected_bi32_rv32i
#  -t --meminit=ram,./sw/c/build/ascon/demo/simpleserial-ascon/simpleserial-ascon
#  -t --meminit=ram,./sw/c/build/ascon/demo/simpleserial-aes/simpleserial-aes
popd

echo "simulate"
# 2 shares ------------------------------------------------------------------------------------------------------------------------
echo "2 shares"

echo -e "|-- ibex_demo_system.log --|\n "
cat ibex-demo-system/ibex_demo_system.log
echo -e "\n|--      End of log      --|"

