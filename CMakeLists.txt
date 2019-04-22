cmake_minimum_required (VERSION 3.9)

if (NOT DEFINED CMAKE_BUILD_TYPE)
    set (CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

add_subdirectory ("cstructures")

if (CSTRUCTURES_BENCHMARKS)
    #set_property (TARGET benchmark PROPERTY POSITION_INDEPENDENT_CODE ${CSTRUCTURES_PIC})
    add_subdirectory ("thirdparty/benchmark")
endif ()

if (CSTRUCTURES_TESTS)
    add_subdirectory ("thirdparty/googletest")
    set_property (TARGET gmock PROPERTY POSITION_INDEPENDENT_CODE ${CSTRUCTURES_PIC})
    set_property (TARGET gtest PROPERTY POSITION_INDEPENDENT_CODE ${CSTRUCTURES_PIC})
endif ()
