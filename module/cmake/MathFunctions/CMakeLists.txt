


#add_library(MathFunctions mysqrt.cpp)
add_library(MathFunctions SHARED mysqrt.cpp)
add_library(MathFunctions-static STATIC mysqrt.cpp)
set_target_properties (MathFunctions-static PROPERTIES OUTPUT_NAME "MathFunctions")


#SET_TARGET_PROPERTIES(hello_static PROPERTIES OUTPUT_NAME "hello")
#SET_TARGET_PROPERTIES(hello PROPERTIES CLEAN_DIRECT_OUTPUT 1)
#SET_TARGET_PROPERTIES(hello_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)


install (TARGETS MathFunctions DESTINATION bin)
install (FILES MathFunctions.h DESTINATION include)
