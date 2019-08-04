# Author: Younguk Kim (bluekyu)
# Date  : 2017-07-19

cmake_minimum_required(VERSION 3.8)

# windows_add_longpath_manifest function.
#
# This function genereates "longpath.manifest" file if it does not exist.
# And then, it adds the manifest file to given target as source.
# The manifest file has "longPathAware" tag for Win32 application.
# see https://msdn.microsoft.com/en-us/library/windows/desktop/aa365247(v=vs.85).aspx
#
# windows_add_longpath_manifest(<target>)
# @param  target    Target variable
function(windows_add_longpath_manifest target_)
  set(manifest_file_path "${CMAKE_BINARY_DIR}/src/windows/longpath.manifest")

  if(NOT EXISTS ${manifest_file_path})
    file(WRITE ${manifest_file_path}
      "<assembly xmlns=\"urn:schemas-microsoft-com:asm.v1\" manifestVersion=\"1.0\">\n"
      "  <application xmlns=\"urn:schemas-microsoft-com:asm.v3\">\n"
      "  <windowsSettings>\n"
      "    <longPathAware xmlns=\"http://schemas.microsoft.com/SMI/2016/WindowsSettings\">true</longPathAware>\n"
      "  </windowsSettings>\n"
      "  </application>\n"
      "</assembly>\n"
    )
    message(STATUS "\"${manifest_file_path}\" file was created.")
  endif()

  target_sources(${target_} PRIVATE ${manifest_file_path})
endfunction()

# windows_add_delay_load function.
#
# This function add "/DELAYLOAD" option to link flags.
#
# windows_add_delay_load(TARGET <target> FILES <dll_names> CONFIGURATIONS <config>)
# @param  TARGET        Target variable
# @param  FILES         The list of name of dlls.
# @param  IMPORTED_TARGETS  Imported targets to delay dlls.
# @param  CONFIGURATIONS    Configuration. ex) Release, Debug
function(windows_add_delay_load)
  cmake_parse_arguments(ARG "" "TARGET" "FILES;IMPORTED_TARGETS;CONFIGURATIONS" ${ARGN})

  # process FILES & CONFIGURATIONS
  set(flag "")
  foreach(file_name ${ARG_FILES})
    set(flag "${flag} /DELAYLOAD:${file_name}")
  endforeach()

  if(ARG_CONFIGURATIONS)
    set(_imported_target_config ${ARG_CONFIGURATIONS})
    foreach(config ${ARG_CONFIGURATIONS})
      string(TOUPPER ${config} config_upper)
      set(flag_${config_upper} ${flag})
    endforeach()
    unset(flag)
  else()
    if(CMAKE_BUILD_TYPE)
      set(_imported_target_config "${CMAKE_BUILD_TYPE}")
    else()
      set(_imported_target_config ${CMAKE_CONFIGURATION_TYPES})
    endif()
  endif()

  foreach(config ${_imported_target_config})
    string(TOUPPER ${config} config_upper)
    set(imported_target_config "${imported_target_config};${config_upper}")
  endforeach()

  # process IMPORTED_TARGETS
  foreach(target ${ARG_IMPORTED_TARGETS})
    get_target_property(target_type ${target} TYPE)
    if(${target_type} STREQUAL "INTERFACE_LIBRARY")
      get_target_property(imported_libname ${target} IMPORTED_LIBNAME)
      if(imported_libname)
        set(flag "${flag} /DELAYLOAD:${imported_libname}${CMAKE_SHARED_LIBRARY_SUFFIX}")
      else()
        foreach(config_upper ${imported_target_config})
          get_target_property(imported_libname ${target} IMPORTED_LIBNAME_${config_upper})
          if(imported_libname)
            set(flag_${config_upper} "${flag_${config_upper}} /DELAYLOAD:${imported_libname}${CMAKE_SHARED_LIBRARY_SUFFIX}")
          endif()
        endforeach()
      endif()
    else()
      get_target_property(imported_location ${target} IMPORTED_LOCATION)
      if(imported_location)
        get_filename_component(imported_libname ${imported_location} NAME_WE)
        set(flag "${flag} /DELAYLOAD:${imported_libname}${CMAKE_SHARED_LIBRARY_SUFFIX}")
      else()
        foreach(config_upper ${imported_target_config})
          get_target_property(imported_location ${target} IMPORTED_LOCATION_${config_upper})
          if(imported_location)
            get_filename_component(imported_libname ${imported_location} NAME_WE)
            set(flag_${config_upper} "${flag_${config_upper}} /DELAYLOAD:${imported_libname}${CMAKE_SHARED_LIBRARY_SUFFIX}")
          endif()
        endforeach()
      endif()
    endif()
  endforeach()

  if(flag)
    if(ARG_CONFIGURATIONS)
      foreach(config_upper ${imported_target_config})
        set_property(TARGET ${ARG_TARGET} APPEND_STRING PROPERTY LINK_FLAGS_${config_upper} "${flag} ")
      endforeach()
    else()
      set_property(TARGET ${ARG_TARGET} APPEND_STRING PROPERTY LINK_FLAGS "${flag} ")
    endif()
  endif()

  foreach(config_upper ${imported_target_config})
    if(flag_${config_upper})
      set_property(TARGET ${ARG_TARGET} APPEND_STRING PROPERTY LINK_FLAGS_${config_upper} "${flag_${config_upper}} ")
    endif()
  endforeach()
endfunction()
