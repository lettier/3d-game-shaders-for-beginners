#.rst:
# Findpanda3d
# --------------
#
# FindPanda3D.cmake
#
# Author: Younguk Kim (bluekyu)
# Date  : 2017-11-20
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module defines the following variables::
#
#   panda3d_FOUND      - True if panda3d has been found and can be used
#
# This module reads hints about search locations from variables::
#
#   panda3d_ROOT       - Preferred installation prefix
#   panda3d_ROOT_DEBUG     - Preferred Debug installation prefix (Panda3D optimization level 1-2)
#   panda3d_ROOT_RELEASE   - Preferred Release installation prefix (Panda3D optimization level 3-4)
#
# The following `IMPORTED` targets are also defined::
#
#   panda3d::panda3d  - Target for all Panda3D libraries.
#   panda3d::<C>    - Target for specific Panda3D component.

cmake_minimum_required(VERSION 3.8)

function(_panda3d_find_include)
  set(panda3d_configurations "DEBUG" "RELEASE")

  foreach(configuration ${panda3d_configurations})
    find_path(panda3d_INCLUDE_DIR_${configuration}
      NAMES "pandaFramework.h"
      HINTS "${panda3d_ROOT_${configuration}}/include"
    )
  endforeach()

  if(panda3d_INCLUDE_DIR_RELEASE)
    set(panda3d_INCLUDE_DIR ${panda3d_INCLUDE_DIR_RELEASE} PARENT_SCOPE)
  else()
    set(panda3d_INCLUDE_DIR ${panda3d_INCLUDE_DIR_DEBUG} PARENT_SCOPE)
  endif()
endfunction()

function(_panda3d_get_dependencies component_name ret)
  set(panda3d_DEPENDENCIES_p3dtoolconfig      p3dtool)
  set(panda3d_DEPENDENCIES_p3interrogatedb    p3dtoolconfig)
  set(panda3d_DEPENDENCIES_pandaexpress       p3dtoolconfig)
  set(panda3d_DEPENDENCIES_panda              pandaexpress)
  set(panda3d_DEPENDENCIES_p3vision           panda)
  set(panda3d_DEPENDENCIES_p3rocket           panda)
  set(panda3d_DEPENDENCIES_p3awesomium        panda)
  set(panda3d_DEPENDENCIES_pandaskel          panda)
  set(panda3d_DEPENDENCIES_pandafx            panda)
  set(panda3d_DEPENDENCIES_p3vrpn             panda)
  set(panda3d_DEPENDENCIES_p3ffmpeg           panda)
  set(panda3d_DEPENDENCIES_p3fmod_audio       panda)
  set(panda3d_DEPENDENCIES_p3openal_audio     panda)
  set(panda3d_DEPENDENCIES_p3windisplay       panda)
  set(panda3d_DEPENDENCIES_pandadx9           p3windisplay)
  set(panda3d_DEPENDENCIES_p3framework        panda)
  set(panda3d_DEPENDENCIES_pandaegg           panda)
  if(WIN32)
    set(panda3d_DEPENDENCIES_pandagl          p3windisplay)
  else()
    set(panda3d_DEPENDENCIES_pandagl          panda)
  endif()
  set(panda3d_DEPENDENCIES_pandagles          panda)
  set(panda3d_DEPENDENCIES_pandagles2         panda)
  set(panda3d_DEPENDENCIES_pandaode           panda)
  set(panda3d_DEPENDENCIES_pandabullet        panda)
  set(panda3d_DEPENDENCIES_pandaphysx         panda)
  set(panda3d_DEPENDENCIES_pandaphysics       panda)
  set(panda3d_DEPENDENCIES_pandaspeedtree     panda)
  set(panda3d_DEPENDENCIES_p3android          panda)
  if(WIN32)
    set(panda3d_DEPENDENCIES_p3android        panda p3windisplay)
  else()
    set(panda3d_DEPENDENCIES_p3android        panda)
  endif()
  set(panda3d_DEPENDENCIES_p3direct           panda)
  set(panda3d_DEPENDENCIES_p3dpython          p3interrogatedb panda)
  set(panda3d_DEPENDENCIES_p3assimp           panda)
  set(panda3d_DEPENDENCIES_p3ptloader         pandaegg)
  set(panda3d_DEPENDENCIES_p3mayaloader2013   p3ptloader)
  set(panda3d_DEPENDENCIES_p3mayaloader20135  p3ptloader)
  set(panda3d_DEPENDENCIES_p3mayaloader2014   p3ptloader)
  set(panda3d_DEPENDENCIES_p3mayaloader2015   p3ptloader)
  set(panda3d_DEPENDENCIES_p3mayaloader2016   p3ptloader)
  set(panda3d_DEPENDENCIES_p3mayaloader20165  p3ptloader)
  set(panda3d_DEPENDENCIES_p3mayaloader2017   p3ptloader)
  set(panda3d_DEPENDENCIES_pandaai            panda)

  set(${ret} ${panda3d_DEPENDENCIES_${component_name}} PARENT_SCOPE)
endfunction()

function(_panda3d_add_library component_name)
  if(TARGET panda3d::${component_name})
    return()
  endif()

  if(WIN32)
    set(panda3d_lib_prefix "lib")
  else()
    set(panda3d_lib_prefix "")
  endif()
  set(panda3d_lib_suffix_RELEASE "")
  set(panda3d_lib_suffix_DEBUG "_d")
  set(panda3d_configurations "DEBUG" "RELEASE")

  foreach(configuration ${panda3d_configurations})
    find_library(panda3d_${component_name}_LIBRARY_${configuration}
      NAMES ${panda3d_lib_prefix}${component_name}${panda3d_lib_suffix_${configuration}}
      HINTS "${panda3d_ROOT_${configuration}}"
      PATH_SUFFIXES "lib"
    )
  endforeach()

  if(EXISTS "${panda3d_${component_name}_LIBRARY_RELEASE}")
    if(NOT TARGET panda3d::${component_name})
      add_library(panda3d::${component_name} UNKNOWN IMPORTED)
    endif()

    set_property(TARGET panda3d::${component_name} APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
    set_property(TARGET panda3d::${component_name} APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
    set_target_properties(panda3d::${component_name} PROPERTIES
      IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
      IMPORTED_LOCATION "${panda3d_${component_name}_LIBRARY_RELEASE}"

      IMPORTED_LINK_INTERFACE_LANGUAGES_${configuration} "CXX"
      IMPORTED_LOCATION_RELEASE "${panda3d_${component_name}_LIBRARY_RELEASE}"
      IMPORTED_LOCATION_RELWITHDEBINFO "${panda3d_${component_name}_LIBRARY_RELEASE}"
      IMPORTED_LOCATION_MINSIZEREL "${panda3d_${component_name}_LIBRARY_RELEASE}"
    )
  endif()

  if(EXISTS "${panda3d_${component_name}_LIBRARY_DEBUG}")
    if(NOT TARGET panda3d::${component_name})
      add_library(panda3d::${component_name} UNKNOWN IMPORTED)
    endif()

    set_property(TARGET panda3d::${component_name} APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
    set_target_properties(panda3d::${component_name} PROPERTIES
      IMPORTED_LINK_INTERFACE_LANGUAGES_${configuration} "CXX"
      IMPORTED_LOCATION_DEBUG "${panda3d_${component_name}_LIBRARY_DEBUG}"
    )

    if(NOT EXISTS "${panda3d_${component_name}_LIBRARY_RELEASE}")
      set_target_properties(panda3d::${component_name} PROPERTIES
        IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
        IMPORTED_LOCATION "${panda3d_${component_name}_LIBRARY_DEBUG}"
      )
    endif()
  endif()

  if(TARGET panda3d::${component_name})
    if(panda3d_INCLUDE_DIR_DEBUG AND panda3d_INCLUDE_DIR_RELEASE)
      set_target_properties(panda3d::${component_name} PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES $<IF:$<CONFIG:DEBUG>,${panda3d_INCLUDE_DIR_DEBUG},${panda3d_INCLUDE_DIR_RELEASE}>
      )
    else()
      set_target_properties(panda3d::${component_name} PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES ${panda3d_INCLUDE_DIR}
      )
    endif()

    if(${component_name} STREQUAL "p3dtool")
      set_target_properties(panda3d::${component_name} PROPERTIES INTERFACE_COMPILE_FEATURES "cxx_std_11")
      if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
        include(CMakeFindDependencyMacro)
        find_dependency(Threads REQUIRED)
        target_link_libraries(panda3d::${component_name} INTERFACE Threads::Threads)
      endif()
    elseif(${component_name} STREQUAL "panda")
      if((${CMAKE_SYSTEM_NAME} MATCHES "Windows") OR (${CMAKE_SYSTEM_NAME} MATCHES "Linux"))
        # check if Eigen is used
        foreach(configuration ${panda3d_configurations})
          if(panda3d_INCLUDE_DIR_${configuration})
            file(STRINGS "${panda3d_INCLUDE_DIR_${configuration}}/dtool_config.h"
              panda3d_dtool_HAVE_EIGEN_${configuration}
              REGEX "#define HAVE_EIGEN 1"
            )
          endif()
        endforeach()
        if(panda3d_dtool_HAVE_EIGEN_DEBUG OR panda3d_dtool_HAVE_EIGEN_RELEASE)
          include(CMakeFindDependencyMacro)
          find_dependency(Eigen3 REQUIRED)
          target_link_libraries(panda3d::${component_name} INTERFACE
            $<$<AND:$<BOOL:${panda3d_dtool_HAVE_EIGEN_DEBUG}>,$<CONFIG:DEBUG>>:Eigen3::Eigen>
            $<$<AND:$<BOOL:${panda3d_dtool_HAVE_EIGEN_RELEASE}>,$<CONFIG:RELEASE>>:Eigen3::Eigen>
          )
        endif()
      endif()
    endif()

    _panda3d_get_dependencies(${component_name} panda3d_comp_deps_names)
    foreach(deps_name ${panda3d_comp_deps_names})
      if(NOT TARGET panda3d::${deps_name})
        _panda3d_add_library(${deps_name})
      endif()
      set_property(TARGET panda3d::${component_name} APPEND
        PROPERTY INTERFACE_LINK_LIBRARIES panda3d::${deps_name}
      )
    endforeach()

    set(_panda3d_new_found_components ${_panda3d_new_found_components} panda3d::${component_name} PARENT_SCOPE)

    # Make variables changeable to the advanced user
    mark_as_advanced(
      panda3d_${component_name}_LIBRARY_DEBUG
      panda3d_${component_name}_LIBRARY_RELEASE
    )
  endif()
endfunction()

# =================================================================================================

if(panda3d_ROOT AND NOT panda3d_ROOT_RELEASE)
  set(panda3d_ROOT_RELEASE "${panda3d_ROOT}")
endif()

if(panda3d_ROOT AND NOT panda3d_ROOT_DEBUG)
  set(panda3d_ROOT_DEBUG "${panda3d_ROOT}")
endif()

set(panda3d_ROOT "${panda3d_ROOT}" CACHE PATH "Hint for finding panda3d root directory")
set(panda3d_ROOT_DEBUG "${panda3d_ROOT_DEBUG}" CACHE PATH "Hint for finding panda3d-debug root directory")
set(panda3d_ROOT_RELEASE "${panda3d_ROOT_RELEASE}" CACHE PATH "Hint for finding panda3d-release root directory")

_panda3d_find_include()

# Set panda3d_FOUND
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(panda3d
  FOUND_VAR panda3d_FOUND
  REQUIRED_VARS panda3d_INCLUDE_DIR
)

# =================================================================================================

if(panda3d_FOUND)
  # create targets of found components
  foreach(component_name ${panda3d_FIND_COMPONENTS})
    if(NOT TARGET panda3d::${component_name})
      _panda3d_add_library(${component_name})
    endif()
  endforeach()

  if(_panda3d_new_found_components)
    # create interface target
    if(NOT TARGET panda3d::panda3d)
      add_library(panda3d::panda3d INTERFACE IMPORTED)
    endif()

    set_property(TARGET panda3d::panda3d APPEND
      PROPERTY INTERFACE_LINK_LIBRARIES ${_panda3d_new_found_components}
    )
  endif()

  mark_as_advanced(
    panda3d_INCLUDE_DIR
    panda3d_INCLUDE_DIR_DEBUG
    panda3d_INCLUDE_DIR_RELEASE
    panda3d_ROOT_DEBUG
    panda3d_ROOT_RELEASE
  )
endif()
