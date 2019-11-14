cmake_minimum_required(VERSION 3.2)

if(DEFINED ENV{TRAVIS_BUILD_NUMBER})
    project(bulk VERSION 0.0.$ENV{TRAVIS_BUILD_NUMBER})
else()
    project(bulk VERSION 0.0.1)
endif()

find_package(Threads REQUIRED)
find_package(GTest   REQUIRED)
find_package(Boost   REQUIRED)

add_executable(bulk       sources/bulk.cpp)
add_executable(bulk_gtest tests/bulk_gtest.cpp)

set_target_properties(bulk PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)

target_include_directories(bulk_gtest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/sources
    ${Boost_INCLUDE_DIRS}
)

target_link_libraries(bulk_gtest
    ${GTEST_BOTH_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${Boost_LIBRARIES}
)

if (MSVC)
    target_compile_options(bulk PRIVATE
        /W4 -O3
    )
    target_compile_options(bulk_gtest PRIVATE
        /W4 -O3
    )
else ()
    target_compile_options(bulk PRIVATE
        -Wall -Wextra -pedantic -Werror -O3
    )
    target_compile_options(bulk_gtest PRIVATE
        -Wall -Wextra -pedantic -Werror -O3
    )
endif()

install(TARGETS bulk RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT bulk@mail.ru)

include(CPack)

enable_testing()

add_test(bulk_gtest bulk_gtest)

add_custom_command(
    TARGET bulk_gtest
    POST_BUILD 
    COMMAND ./bulk_gtest
    COMMENT "Run bulk test..."
)