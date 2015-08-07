# Try to find nglib/netgen
# Once done this will define
#
# NGLIB_INCLUDE_DIR   - where the nglib include directory can be found
# NGLIB_LIBRARIES     - Link this to use nglib
# NGLIB_LIBRARY_DIRS  - path to the library
# NETGEN_INCLUDE_DIRS - where the netgen include directories can be found
# NETGEN_DEFINITIONS  - set additional difinitions switches
#
# NETGEN_VERSION_5    - set to true if the found library got the version 5
# NETGEN_WITH_OCC     - set to true if the found netgen fas compiled with OpenCasCade
#                       support
# NETGEN_WITH_PARALLEL_THREADS - set to true if the found netgen fas compiled with
#                       parallel threads
#
# The module also uses/alters the value for next variables:
# NETGEN_PRIVATE_HEADERS_PATHS - list of search paths for NETGEN_INCLUDE_DIRS
#
# See also: http://git.salome-platform.org/gitweb/?p=NETGENPLUGIN_SRC.git;a=summary

find_package( PackageHandleStandardArgs REQUIRED )
find_package( OpenCasCade )
find_package( PkgConfig )

include( CMakePushCheckState )
include( CheckCXXSourceCompiles )

# nglib

# Some distributions like Fedora provide pkg-config file for netgen. Try it first.
pkg_check_modules ( NGLIB netgen-mesher netgen QUIET )

# fallback to manual find
if( NOT NGLIB_FOUND )
    find_path( NGLIB_INCLUDE_DIR nglib.h
        PATH_SUFFIXES netgen-mesher netgen nglib
    )
    find_library( NGLIB_LIBRARIES nglib )
    get_filename_component( NGLIB_LIBRARY_DIRS ${NGLIB_LIBRARIES} DIRECTORY CACHE )
endif( )

mark_as_advanced( NGLIB_INCLUDE_DIR )
mark_as_advanced( NGLIB_LIBRARIES )

find_package_handle_standard_args (NGLIB REQUIRED_VARS NGLIB_INCLUDE_DIR NGLIB_LIBRARIES)

# netgen private headers
list( APPEND NETGEN_PRIVATE_HEADERS_PATHS "${NGLIB_INCLUDE_DIR}" )
foreach( _prefix ${CMAKE_SYSTEM_PREFIX_PATH})
    list( APPEND NETGEN_PRIVATE_HEADERS_PATHS "${_prefix}/share/netgen/libsrc" )
endforeach( )
list( APPEND NETGEN_PRIVATE_HEADERS_PATHS "${NGLIB_INCLUDE_DIR}/private" )
list( APPEND NETGEN_PRIVATE_HEADERS_PATHS "${NGLIB_INCLUDE_DIR}/netgen" )
list( REMOVE_DUPLICATES NETGEN_PRIVATE_HEADERS_PATHS )

set( NETGEN_PRIVATE_HEADERS_PATHS ${NETGEN_PRIVATE_HEADERS_PATHS}
    CACHE "A semicolon separated list of files where cmake should search for netgen's private headers" STRING )

find_path( NETGEN_DIR_csg     csg.hpp        PATHS ${NETGEN_PRIVATE_HEADERS_PATHS} PATH_SUFFIXES csg     )
find_path( NETGEN_DIR_general array.hpp      PATHS ${NETGEN_PRIVATE_HEADERS_PATHS} PATH_SUFFIXES general )
find_path( NETGEN_DIR_geom2d  geom2dmesh.hpp PATHS ${NETGEN_PRIVATE_HEADERS_PATHS} PATH_SUFFIXES geom2d  )
find_path( NETGEN_DIR_gprim   gprim.hpp      PATHS ${NETGEN_PRIVATE_HEADERS_PATHS} PATH_SUFFIXES gprim   )
find_path( NETGEN_DIR_linalg  linalg.hpp     PATHS ${NETGEN_PRIVATE_HEADERS_PATHS} PATH_SUFFIXES linalg  )
find_path( NETGEN_DIR_meshing meshing.hpp    PATHS ${NETGEN_PRIVATE_HEADERS_PATHS} PATH_SUFFIXES meshing )
find_path( NETGEN_DIR_occ     occgeom.hpp    PATHS ${NETGEN_PRIVATE_HEADERS_PATHS} PATH_SUFFIXES occ     )
find_path( NETGEN_DIR_stlgeom stlgeom.hpp    PATHS ${NETGEN_PRIVATE_HEADERS_PATHS} PATH_SUFFIXES stlgeom )

foreach( _dir    csg general geom2d gprim linalg meshing occ stlgeom )
    mark_as_advanced( NETGEN_DIR_${_dir} )
    if( NOT "${NETGEN_DIR_${_dir}}" STREQUAL "NETGEN_DIR_${_dir}-NOTFOUND")
        list( APPEND NETGEN_INCLUDE_DIRS ${NETGEN_DIR_${_dir}} )
    else ( )
        message( WARNING "-- Netgen private headers ${_dir} not found" )
    endif( )
endforeach ( )

if( NOT MSVC )
    # Check some features provoded by the library

    # Some needed libraris for tests

    find_library( NETGEN_LIBRARY_mesh NAMES mesh ngmesh PATHS ${NGLIB_LIBRARY_DIRS})
    find_library( NETGEN_LIBRARY_occ  NAMES occ  ngocc PATHS ${NGLIB_LIBRARY_DIRS})
    mark_as_advanced( NETGEN_LIBRARY_mesh NETGEN_LIBRARY_occ)

    cmake_push_check_state()

    # Check if NETGEN provides API version 5

    set( CMAKE_REQUIRED_INCLUDES  ${NETGEN_DIR_general} ${NETGEN_DIR_meshing} ${NETGEN_DIR_linalg} )
    if( NOT "${NETGEN_LIBRARY_occ}" STREQUAL "NETGEN_LIBRARY_occ-NOTFOUND" )
        set( CMAKE_REQUIRED_LIBRARIES ${NGLIB_LIBRARIES} ${NETGEN_LIBRARY_mesh} )
    else( )
        set( CMAKE_REQUIRED_LIBRARIES ${NGLIB_LIBRARIES} )
    endif( )

    set( CMAKE_REQUIRED_DEFINITIONS ${NETGEN_DEFINITIONS} )
    check_cxx_source_compiles (
        " #include <meshing.hpp>
        int main() { netgen::Mesh* mesh; mesh->CalcLocalH(0.); } "
        NETGEN_VERSION_5
        )

    cmake_reset_check_state()

    # Check if NETGEN was built with parthreads

    set( CMAKE_REQUIRED_INCLUDES  ${NETGEN_DIR_general} )
    set( CMAKE_REQUIRED_DEFINITIONS )
    check_cxx_source_compiles (
        " #include <myadt.hpp>
        int main() { netgen::NgMutex foo; } "
        NETGEN_WITH_PARALLEL_THREADS
        )

    if( NOT NETGEN_WITH_PARALLEL_THREADS )
        list( APPEND NETGEN_DEFINITIONS "-DNO_PARALLEL_THREADS")
    endif( )

    cmake_reset_check_state()

    # Check if NETGEN build with opencascade

    set( CMAKE_REQUIRED_INCLUDES
        ${NETGEN_DIR_occ} ${NETGEN_DIR_general} ${NETGEN_DIR_meshing} ${NETGEN_DIR_linalg}
        ${OCC_INCLUDE_DIR}
        )

    # Get full paths for OCC libraris because CMAKE_REQUIRED_LIBRARIES supposes to get such
    foreach( _lib ${OCC_LIBRARIES})
        find_library( OCC_LIBRARY_${_lib}  ${_lib} HINTS ${OCC_LIBRARY_DIR} )
        if( NOT "${OCC_LIBRARY_${_lib}}" STREQUAL "OCC_LIBRARY_${_lib}-NOTFOUND")
            list(APPEND _OCC_LIBRARIES_FULL_PATHS ${OCC_LIBRARY_${_lib}})
        endif()
    endforeach ()

    if( NETGEN_VERSION_5 )
        set( CMAKE_REQUIRED_LIBRARIES ${NETGEN_LIBRARY_mesh} ${NETGEN_LIBRARY_occ})
    endif( )

    set( CMAKE_REQUIRED_LIBRARIES
        ${CMAKE_REQUIRED_LIBRARIES}
        ${NGLIB_LIBRARIES}
        ${_OCC_LIBRARIES_FULL_PATHS}
        )
    set( CMAKE_REQUIRED_DEFINITIONS ${NETGEN_DEFINITIONS} -DOCCGEOMETRY )

    check_cxx_source_compiles (
        "#include <occgeom.hpp>
        int main() { netgen::OCCGeometry foo; }"
        NETGEN_WITH_OCC
        )

    unset( _OCC_LIBRARIES_FULL_PATHS )

    if( NETGEN_WITH_OCC )
        list( APPEND NETGEN_DEFINITIONS "-DOCCGEOMETRY" )
    endif( )

    cmake_pop_check_state( )
else( NOT MSVC )
    # if some one willing to fix up autoconfiguration with MSVC fill free
    message( STATUS "-- nglib features check skipped due to msvc compiller" )
    set ( NETGEN_VERSION_5 TRUE CACHE "true if netgen library version is 5" INTERNAL )
    set ( NETGEN_WITH_OCC  TRUE CACHE "netgen is build with opencascade support" INTERNAL )
    set ( NETGEN_WITH_PARALLEL FALSE CACHE "true if netgen is build without parallel threads support" INTERNAL )

    set( NETGEN_DEFINITIONS -DNO_PARALLEL_THREADS -DOCCGEOMETRY )
endif( NOT MSVC )

set( NETGEN_DEFINITIONS ${NETGEN_DEFINITIONS} CACHE
    "Additional definitions switches required to build against private headers" INTERNAL )
