
add_library(secp256k1 STATIC 

src/secp256k1.c

)

include_directories(src)

add_definitions(-DUSE_NUM_NONE -DUSE_FIELD_10X26 -DUSE_FIELD_INV_BUILTIN -DUSE_SCALAR_8X32 -DUSE_SCALAR_INV_BUILTIN -DENABLE_MODULE_ECDH -DENABLE_MODULE_RECOVERY)
target_include_directories(secp256k1 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

