##################################################################
#
# Coverage Unit Test build recipe
#
# This CMake file contains the recipe for building the sample_radio unit tests.
# It is invoked from the parent directory when unit tests are enabled.
#
##################################################################

#
#
# NOTE on the subdirectory structures here:
#
# - "inc" provides local header files shared between the coveragetest,
#    wrappers, and overrides source code units
# - "coveragetest" contains source code for the actual unit test cases
#    The primary objective is to get line/path coverage on the FSW 
#    code units.
#

add_cfe_coverage_stubs(sample_radio-internal stubs/sample_radio_device_stubs.c stubs/libuart_stubs.c) 
target_link_libraries(coverage-sample_radio-internal-stubs ut_core_api_stubs ut_assert)
target_include_directories(coverage-sample_radio-internal-stubs PUBLIC $<TARGET_PROPERTY:sample_radio,INCLUDE_DIRECTORIES>)

 
# Use the UT assert public API, and allow direct
# inclusion of source files that are normally private
include_directories(${PROJECT_SOURCE_DIR}/fsw/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)
include_directories(${hwlib_MISSION_DIR}/fsw/public_inc)
include_directories(${PROJECT_SOURCE_DIR}/../../../../fsw/osal/ut_assert/inc)


# Add a coverage test executable called "sample_radio-ALL" that 
# covers all of the functions in sample_radio_app.  
#
# Also note in a more complex app/lib the coverage test can also
# be broken down into smaller units (in which case one should use
# a unique suffix other than "ALL" for each unit).  For example,
# OSAL implements a separate coverage test per source unit.
add_cfe_coverage_test(sample_radio ALL 
    "coveragetest/coveragetest_sample_radio_app.c"
    "../src/sample_radio_app.c"
)

add_cfe_coverage_test(sample_radio DEVICE 
    "coveragetest/coveragetest_sample_radio_device.c"
    "../../shared/sample_radio_device.c"
    "stubs/libuart_stubs.c"
)

add_cfe_coverage_dependency(sample_radio ALL sample_radio-internal)


# The sample_radio uses library functions provided by sample_radio_lib so must be linked
# with the sample_radio_lib stub library (this is mainly just an example of how this 
# can be done).
#add_cfe_coverage_dependency(sample_radio ALL sample_radio_lib)

