cmake_minimum_required(VERSION 3.2)

if(DEFINED ENV{TRAVIS_BUILD_NUMBER})
    project(bulkmt VERSION 0.0.$ENV{TRAVIS_BUILD_NUMBER})
else()
    project(bulkmt VERSION 0.0.1)
endif()

find_package(Threads REQUIRED)

add_executable(bulkmt sources/bulk.cpp)

set_target_properties(bulkmt PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
)

target_link_libraries(bulkmt
    PRIVATE
        Threads::Threads
)

if (MSVC)
    target_compile_options(bulkmt PRIVATE
        /W4 -O3
    )
else ()
    target_compile_options(bulkmt PRIVATE
        -Wall -Wextra -pedantic -Werror -O3
    )
endif()

install(TARGETS bulkmt RUNTIME DESTINATION bin)

set(CPACK_GENERATOR DEB)

set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT bulkmt@mail.ru)

include(CPack)
