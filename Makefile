CXX=clang++-20

INCLUDES=-I${TT_METAL_HOME}/tt_metal/api -I${TT_METAL_HOME}/tt_metal/api/tt-metalium -I${TT_METAL_HOME}/build_Release/include -I${TT_METAL_HOME}/build_Release/include/metalium-thirdparty -I${TT_METAL_HOME}/build_Release/include/umd/device/device/api -I${TT_METAL_HOME}/tt_metal/hostdevcommon/api/ -I${TT_METAL_HOME}/tt_metal/third_party/tracy/public -I${TT_METAL_HOME}/build_Release/include/fmt -I${HOME}/tools/boost_1_89_0

CFLAGS=${INCLUDES} -O3 -Wno-int-to-pointer-cast -mavx2 -fPIC -DFMT_HEADER_ONLY -DSPDLOG_FMT_EXTERNAL -fvisibility-inlines-hidden -fno-lto -DARCH_WORMHOLE -DDISABLE_ISSUE_3487_FIX -Werror -Wdelete-non-virtual-dtor -Wreturn-type -Wswitch -Wuninitialized -Wno-unused-parameter -Wsometimes-uninitialized -Wno-c++11-narrowing -Wno-c++23-extensions -Wno-error=local-type-template-args -Wno-delete-non-abstract-non-virtual-dtor -Wno-c99-designator -Wno-shift-op-parentheses -Wno-non-c-typedef-for-linkage -Wno-deprecated-this-capture -Wno-deprecated-volatile -Wno-deprecated-builtins -Wno-deprecated-declarations -std=c++20

LINKER=clang++-20
LFLAGS= -rdynamic \
        -L${TT_METAL_HOME}/build_Release/lib -ltt_metal \
	    -Wl,-rpath,${TT_METAL_HOME}/build_Release/lib \
        -ldl -lstdc++fs -pthread -lm -ldevice \
        -L${HOME}/tools/boost_1_89_0/stage/lib

# -L : Link-time search path
# -Wl,-rpath : Runtime search path
 
all:
	${CXX} ${CFLAGS} -c host.cpp
	${LINKER} host.o ../utils-tt/tt-utils.o -o executable ${LFLAGS}
