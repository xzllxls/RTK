#
# General SWIG configuration
#
if ( WRAP_LUA OR WRAP_PYTHON OR WRAP_JAVA OR WRAP_CSHARP OR WRAP_TCL OR WRAP_R OR WRAP_RUBY )
  
  find_package ( SWIG 2 REQUIRED )
  include (UseSWIGLocal)

  set_source_files_properties ( SimpleRTK.i PROPERTIES CPLUSPLUS ON )

  file(GLOB SWIG_EXTRA_DEPS
    "${SimpleRTK_SOURCE_DIR}/Code/Common/include/*.h"
    "${SimpleRTK_SOURCE_DIR}/Code/IO/include/*.h")

  # make a manual list of dependencies for the Swig.i files
  list( APPEND SWIG_EXTRA_DEPS "${SimpleRTK_BINARY_DIR}/Code/BasicFilters/include/SimpleRTKBasicFiltersGeneratedHeaders.i"
   "${SimpleRTK_BINARY_DIR}/Code/BasicFilters/include/SimpleRTKBasicFiltersGeneratedHeaders.h"
    ${SimpleRTKBasicFiltersGeneratedHeader}
     )

  # check if uint64_t is the same as unsigned long
  try_compile(SRTK_ULONG_SAME_AS_UINT64
    ${SimpleRTK_BINARY_DIR}/CMakeTmp
    ${SimpleRTK_SOURCE_DIR}/CMake/same_uint64_ulong.cxx )

  # when "-DSWIGWORDSIZE64" is defined SWIG used unsigned long for uint64_t types
  if(${SRTK_ULONG_SAME_AS_UINT64} )
    set ( CMAKE_SWIG_GLOBAL_FLAGS "-DSWIGWORDSIZE64" )
  endif()

endif()

#
# lua SWIG configuration
#
if ( WRAP_LUA )
  option ( USE_SYSTEM_LUA "Use a system provided lua" OFF )

  if ( USE_SYSTEM_LUA )
    find_package ( Lua51 REQUIRED )
    include_directories ( ${LUA_INCLUDE_DIR} )
    set ( LUA_LIB ${LUA_LIBRARIES} )
  else()
    set ( LUA_LIB lua5 )
    include_directories ( ${SimpleRTK_SOURCE_DIR}/Utilities/lua-5.1.5/src )
  endif()

  # Run swig
  set(CMAKE_SWIG_FLAGS ${CMAKE_SWIG_GLOBAL_FLAGS})
  set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR})
  set(SWIG_MODULE_SimpleRTKLua_EXTRA_DEPS ${SWIG_EXTRA_DEPS}
    ${CMAKE_CURRENT_SOURCE_DIR}/Lua.i )
  SWIG_module_initialize ( SimpleRTKLua lua )
  SWIG_add_source_to_module ( SimpleRTKLua swig_generated_source SimpleRTK.i ${SWIG_EXTRA_DEPS} )
  set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/SimpleRTKLUA_wrap.cxx PROPERTIES COMPILE_FLAGS "-w" )

  add_executable ( SimpleRTKLua SimpleRTKLuaMain.cxx SimpleRTKLUA_wrap.cxx )
  target_link_libraries ( SimpleRTKLua ${SimpleRTK_LIBRARIES} ${LUA_LIB} )
  srtk_strip_target( SimpleRTKLua )

endif()

#
# python SWIG configuration
#
if ( WRAP_PYTHON )
  find_package ( PythonLibs REQUIRED )
  find_package ( PythonInterp REQUIRED )
  include_directories ( ${PYTHON_INCLUDE_DIR} ${SimpleRTK_SOURCE_DIR}/Wrapping )

  option ( SimpleRTK_PYTHON_THREADS "Enable threaded python usage by unlocking the GIL." OFF )
  mark_as_advanced( SimpleRTK_PYTHON_THREADS )
  option ( SimpleRTK_PYTHON_WHEEL "Add building of python wheels to the dist target." OFF )
  mark_as_advanced( SimpleRTK_PYTHON_WHEEL )


  # Run swig
  set(CMAKE_SWIG_FLAGS ${CMAKE_SWIG_GLOBAL_FLAGS} -features autodoc=1 -keyword )
  if( SimpleRTK_PYTHON_THREADS )
    set(CMAKE_SWIG_FLAGS ${CMAKE_SWIG_FLAGS} -threads)
  endif()
  set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR})
  set(SWIG_MODULE_SimpleRTK_EXTRA_DEPS ${SWIG_EXTRA_DEPS}
    ${CMAKE_CURRENT_SOURCE_DIR}/Python.i )
  SWIG_add_module ( SimpleRTK python
    SimpleRTK.i
    srtkPyCommand.cxx )
  SWIG_link_libraries ( SimpleRTK ${SimpleRTK_LIBRARIES} ${PYTHON_LIBRARIES})
  set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/SimpleRTKPYTHON_wrap.cxx PROPERTIES COMPILE_FLAGS "-w")
  srtk_strip_target( ${SWIG_MODULE_SimpleRTK_TARGET_NAME} )

  set(SWIG_MODULE_SimpleRTKPython_TARGET_NAME "${SWIG_MODULE_SimpleRTK_TARGET_NAME}")

  # Installation
  set( SIMPLERTK_PYTHON_PACKAGE_DIR "${SimpleRTK_BINARY_DIR}/Wrapping" )
  file( TO_NATIVE_PATH "${SIMPLERTK_PYTHON_PACKAGE_DIR}" SIMPLERTK_PYTHON_PACKAGE_DIR )
  get_target_property( SWIG_MODULE_SimpleRTKPython_TARGET_LOCATION ${SWIG_MODULE_SimpleRTKPython_TARGET_NAME} OUTPUT_NAME )
  add_custom_command(
    TARGET ${SWIG_MODULE_SimpleRTKPython_TARGET_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "$<TARGET_FILE:${SWIG_MODULE_SimpleRTKPython_TARGET_NAME}>" "${CMAKE_CURRENT_BINARY_DIR}"
  )
  get_target_property( SIMPLERTK_RELATIVE_BINARY_MODULE ${SWIG_MODULE_SimpleRTKPython_TARGET_NAME} LOCATION )
  get_filename_component( SIMPLERTK_RELATIVE_BINARY_MODULE "${SIMPLERTK_RELATIVE_BINARY_MODULE}" NAME )


  if ( SimpleRTK_DOC_FILES )
    # create a python list for the import documents to include in
    # packaging

    # This string is targed for setup.py. It will be passed through
    # the build-time configuration script and as a command line
    # argument. This sequence is having portability issues with
    # quote. So Windows needs separate handling from Unix-like
    # platforms.
    if( WIN32 )
      set( _q "'" )
    else()
      set( _q "\\'")
    endif()

    # specially handle the first element
    list( GET SimpleRTK_DOC_FILES 0 d )
    file(TO_NATIVE_PATH "${d}" d )
    set( SimpleRTK_DOC_FILES_AS_LIST "[r${_q}${d}${_q}")
    set( _doc_list "${SimpleRTK_DOC_FILES}" )
    list( REMOVE_AT _doc_list 0 )

    foreach( d ${_doc_list} )
      file(TO_NATIVE_PATH "${d}" d )
      set( SimpleRTK_DOC_FILES_AS_LIST "${SimpleRTK_DOC_FILES_AS_LIST},r${_q}${d}${_q}")
    endforeach()
    set( SimpleRTK_DOC_FILES_AS_LIST "${SimpleRTK_DOC_FILES_AS_LIST}]")

  endif()

  include( srtkConfigureFileBuildtime )
  configure_file_buildtime( "${CMAKE_CURRENT_SOURCE_DIR}/PythonPackage/setup.py.in"
    "${CMAKE_CURRENT_BINARY_DIR}/PythonPackage/setup.py" )

  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/PythonPackage/ez_setup.py"
    "${CMAKE_CURRENT_BINARY_DIR}/PythonPackage/ez_setup.py"
    COPYONLY )

  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/PythonPackage/__init__.py"
    "${CMAKE_CURRENT_BINARY_DIR}/__init__.py"
    COPYONLY )


  #
  # Setup Python Virtual Enviroment for testing and packaging
  #
  set( PythonVirtualenvHome "${SimpleRTK_BINARY_DIR}/Testing/Installation/PythonVirtualenv" )

  # virtualenv places the python executable in different
  # locations. Also note than on windows installations where python is
  # installed only for a single user the may be a missing dll issue.
  if( WIN32 )
    set( VIRTUAL_PYTHON_EXECUTABLE "${PythonVirtualenvHome}/Scripts/python" CACHE INTERNAL "Python executable in virtual enviroment" FORCE )
  else()
    set( VIRTUAL_PYTHON_EXECUTABLE "${PythonVirtualenvHome}/bin/python" CACHE INTERNAL "Python executable in virtual enviroment" FORCE )
  endif()

  # configure a scripts which creates the virtualenv and installs numpy
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/PythonVirtualEnvInstall.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/PythonVirtualEnvInstall.cmake"
    @ONLY )

  set( PythonVirtualEnv_ALL "" )
  if ( BUILD_TESTING )
    set( PythonVirtualEnv_ALL "ALL" )
  endif()

  add_custom_target( PythonVirtualEnv ${PythonVirtualEnv_ALL}
    DEPENDS "${VIRTUAL_PYTHON_EXECUTABLE}"
    SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/PythonVirtualEnvInstall.cmake.in )

  add_custom_command( OUTPUT "${VIRTUAL_PYTHON_EXECUTABLE}"
    COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/PythonVirtualEnvInstall.cmake"
    DEPENDS
      "${SWIG_MODULE_SimpleRTKPython_TARGET_NAME}"
      "${CMAKE_CURRENT_BINARY_DIR}/PythonVirtualEnvInstall.cmake"
      ConfigureFileBuildtime
      "${CMAKE_CURRENT_BINARY_DIR}/PythonPackage/ez_setup.py"
    COMMENT "Creating python virtual enviroment..."
    )

endif()

#
# ruby SWIG configuration
#
if ( WRAP_RUBY )
  find_package ( Ruby REQUIRED )
  include_directories ( ${RUBY_INCLUDE_DIRS} )

  # Run swig
  set(CMAKE_SWIG_FLAGS -autorename -module simplertk ${CMAKE_SWIG_GLOBAL_FLAGS})
  set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR})
  set(SWIG_MODULE_simplertk_EXTRA_DEPS ${SWIG_EXTRA_DEPS}
     ${CMAKE_CURRENT_SOURCE_DIR}/Ruby.i)

  SWIG_add_module ( simplertk ruby SimpleRTK.i SimpleRTKRUBY_wrap.cxx )
  SWIG_link_libraries ( simplertk ${SimpleRTK_LIBRARIES} ${RUBY_LIBRARY})
  set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/SimpleRTKRUBY_wrap.cxx  PROPERTIES COMPILE_FLAGS "-w")
  srtk_strip_target( ${SWIG_MODULE_simplertk_TARGET_NAME} )

endif()

#
# JAVA SWIG configuration
#
if ( WRAP_JAVA )
  find_package ( Java REQUIRED )
  find_package ( JNI REQUIRED )
  include_directories ( ${JAVA_INCLUDE_PATH} ${JNI_INCLUDE_DIRS} )

  find_program(Java_JAVADOC_EXECUTABLE
    NAMES javadoc
    HINTS ${_JAVA_HINTS}
    PATHS ${_JAVA_PATHS}
    )

  # Make sure the nested directory structure exists
  set(JAVA_SOURCE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/)
  set(JAVA_BINARY_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/build)
  file(MAKE_DIRECTORY ${JAVA_SOURCE_DIRECTORY})
  file(MAKE_DIRECTORY ${JAVA_BINARY_DIRECTORY})

  # Nicely write the bridge code in org/rtk/simple
  set(CMAKE_SWIG_OUTDIR ${JAVA_SOURCE_DIRECTORY}/org/rtk/simple/)
  set(CMAKE_SWIG_FLAGS -package "org.rtk.simple" ${CMAKE_SWIG_GLOBAL_FLAGS})
  set(SWIG_MODULE_SimpleRTKJava_EXTRA_DEPS ${SWIG_EXTRA_DEPS}
    ${CMAKE_CURRENT_SOURCE_DIR}/Java.i )
  SWIG_add_module ( SimpleRTKJava java SimpleRTK.i SimpleRTKJAVA_wrap.cxx)
  SWIG_link_libraries(SimpleRTKJava ${SimpleRTK_LIBRARIES})
  set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/SimpleRTKJAVA_wrap.cxx  PROPERTIES COMPILE_FLAGS "-w")
  srtk_strip_target( ${SWIG_MODULE_SimpleRTKJava_TARGET_NAME} )

  # Add target for org.rtk.simple.jar
  add_custom_target(org_rtk_simple_jar ALL DEPENDS ${JAR_FILE})
  set(JAVA_SOURCE_CODE ${JAVA_SOURCE_DIRECTORY}/org/rtk/simple/*.java)

  # Add custom command and target to compile the generated files and put them in a jar file
  # Make sure the commands depend on the output library from SWIG
  add_custom_command(
    OUTPUT ${JAR_FILE}
    COMMENT "Creating jar file..."
    COMMAND ${Java_JAVAC_EXECUTABLE} -d ${JAVA_BINARY_DIRECTORY} ${JAVA_SOURCE_CODE}
    COMMAND ${Java_JAR_EXECUTABLE} cf ${CMAKE_CURRENT_BINARY_DIR}/${JAR_FILE} -C ${JAVA_BINARY_DIRECTORY} org
    COMMAND ${Java_JAVADOC_EXECUTABLE} -quiet -d ${JAVA_BINARY_DIRECTORY}/javadoc -sourcepath ${JAVA_SOURCE_DIRECTORY} org.rtk.simple
    COMMAND ${Java_JAR_EXECUTABLE} cf ${CMAKE_CURRENT_BINARY_DIR}/${JAVADOC_FILE} -C ${JAVA_BINARY_DIRECTORY}/javadoc org
    COMMAND ${Java_JAR_EXECUTABLE} cf ${CMAKE_CURRENT_BINARY_DIR}/${JAVA_SOURCE_FILE} org
    DEPENDS ${SWIG_MODULE_SimpleRTKJava_TARGET_NAME}
    )

  # Get the location of the extension directory
  string(REGEX REPLACE "include"  "jre/lib/ext" JAVA_EXTENSION_DIR ${JAVA_INCLUDE_PATH} )

  # TODO: add new target to install simplertk java
  # Add the install target
  # install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${JAR_FILE} DESTINATION ${JAVA_EXTENSION_DIR})

endif()

#
# C# SWIG configuration
#
if ( WRAP_CSHARP )

  # Find C#
  find_package( CSharp REQUIRED )
  include( ${CSHARP_USE_FILE} )

  # CSharp version requirements: http://msdn.microsoft.com/en-us/library/system.reflection.assemblyversionattribute.aspx
  # major.minor[.build[.revision]] where all components are 16-bit unsigned integers
  set(_build 0)
  if(DEFINED SimpleRTK_VERSION_POST)
    math(EXPR _build "${SimpleRTK_VERSION_POST}")
  elseif(DEFINED SimpleRTK_VERSION_DEV)
    math(EXPR _build "32768+${SimpleRTK_VERSION_DEV}")
  endif()
  if(_build GREATER 65535)
    message(WARNING "CSharp build component overflowed, setting to 65535 instead of ${_build}.")
    set(_build 65535)
  endif()

  set(_revision 0)
  if(DEFINED SimpleRTK_VERSION_PATCH)
    math(EXPR _revision "${SimpleRTK_VERSION_PATCH}<<8")
    if(DEFINED SimpleRTK_VERSION_TWEAK)
      math(EXPR _revision "_revision+${SimpleRTK_VERSION_TWEAK}")
    endif()
  endif()
  if(_revision GREATER 65535)
    message(WARNING "CSharp revision component overflowed, setting to 65535 instead of ${_revision}.")
    set(_revision 65535)
  endif()

  set(SimpleRTK_VERSION_CSHARP_AssemblyVersion "${SimpleRTK_VERSION_MAJOR}.${SimpleRTK_VERSION_MINOR}.${_build}.${_revision}")

  # Make sure the nested directory structure exists
  set(CSHARP_SOURCE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/CSharpSources CACHE INTERNAL "")
  set(CSHARP_BINARY_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/CSharpBinaries CACHE INTERNAL "")
  file(MAKE_DIRECTORY ${CSHARP_SOURCE_DIRECTORY})
  file(MAKE_DIRECTORY ${CSHARP_BINARY_DIRECTORY})

  # Create swig target
  set(CMAKE_SWIG_OUTDIR ${CSHARP_SOURCE_DIRECTORY})
  if ( UNIX )
    set(CMAKE_SWIG_FLAGS -dllimport \"libSimpleRTKCSharpNative\")
  else ()
    set(CMAKE_SWIG_FLAGS -dllimport \"SimpleRTKCSharpNative\")
  endif ()
  set(CMAKE_SWIG_FLAGS -namespace \"rtk.simple\" ${CMAKE_SWIG_GLOBAL_FLAGS} ${CMAKE_SWIG_FLAGS})
  set(SWIG_MODULE_SimpleRTKCSharpNative_EXTRA_DEPS ${SWIG_EXTRA_DEPS}
    ${CMAKE_CURRENT_SOURCE_DIR}/CSharp.i
    ${CMAKE_CURRENT_SOURCE_DIR}/CSharpTypemapHelper.i )
  swig_add_module(SimpleRTKCSharpNative csharp SimpleRTK.i)
  swig_link_libraries(SimpleRTKCSharpNative ${SimpleRTK_LIBRARIES})
  set_target_properties(${SWIG_MODULE_SimpleRTKCSharpNative_TARGET_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CSHARP_BINARY_DIRECTORY})
  if ( UNIX )
    set_target_properties(${SWIG_MODULE_SimpleRTKCSharpNative_TARGET_NAME} PROPERTIES PREFIX "lib")
    set_target_properties(${SWIG_MODULE_SimpleRTKCSharpNative_TARGET_NAME} PROPERTIES SUFFIX ".so")
  else ()
    set_target_properties(${SWIG_MODULE_SimpleRTKCSharpNative_TARGET_NAME} PROPERTIES PREFIX "")
    set_target_properties(${SWIG_MODULE_SimpleRTKCSharpNative_TARGET_NAME} PROPERTIES SUFFIX ".dll")
    foreach ( CMAKE_CONFIGURATION_TYPE ${CMAKE_CONFIGURATION_TYPES} )
      string(TOUPPER ${CMAKE_CONFIGURATION_TYPE} CMAKE_CONFIGURATION_TYPE)
      set_target_properties(${SWIG_MODULE_SimpleRTKCSharpNative_TARGET_NAME}
        PROPERTIES LIBRARY_OUTPUT_DIRECTORY_${CMAKE_CONFIGURATION_TYPE} "${CSHARP_BINARY_DIRECTORY}")
      set_target_properties(${SWIG_MODULE_SimpleRTKCSharpNative_TARGET_NAME}
        PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${CMAKE_CONFIGURATION_TYPE} "${CSHARP_BINARY_DIRECTORY}")
    endforeach()
  endif()
  srtk_strip_target( ${SWIG_MODULE_SimpleRTKCSharpNative_TARGET_NAME} )

  # Configure AssemblyInfo.cs
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/AssemblyInfo.cs.in
    ${CSHARP_SOURCE_DIRECTORY}/AssemblyInfo.cs
    @ONLY
  )

  # Add managed wrapper
  csharp_add_library(
    SimpleRTKCSharpManaged
    ${CSHARP_SOURCE_DIRECTORY}/*.cs
  )
  add_dependencies(SimpleRTKCSharpManaged ${SWIG_MODULE_SimpleRTKCSharpNative_TARGET_NAME})

endif()

#
# TCL SWIG configuration
#
if ( WRAP_TCL )
  find_package ( TCL REQUIRED )
  include_directories ( ${TCL_INCLUDE_PATH} )

  # Run swig
  set(CMAKE_SWIG_FLAGS "-nosafe" ${CMAKE_SWIG_GLOBAL_FLAGS})
  set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR})
  set(SWIG_MODULE_SimpleRTKTCL_EXTRA_DEPS ${SWIG_EXTRA_DEPS} ${CMAKE_CURRENT_SOURCE_DIR}/Tcl.i )
  set(SWIG_MODULE_SimpleRTKTcl_EXTRA_DEPS  ${SWIG_MODULE_SimpleRTKTCL_EXTRA_DEPS})
#  SWIG_add_module ( SimpleRTKTcl tcl SimpleRTK.i SimpleRTKTCL_wrap.cxx )
#  SWIG_link_libraries ( SimpleRTKTcl ${SimpleRTK_LIBRARIES} ${TCL_LIBRARY})
#  set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/SimpleRTKTCL_wrap.cxx  PROPERTIES COMPILE_FLAGS "-w")
#  add_executable ( SimpleRTKTclsh SimpleRTKTCL_wrap.cxx )
#  target_link_libraries ( SimpleRTKTclsh ${SimpleRTK_LIBRARIES} ${TCL_LIBRARY})

  SWIG_module_initialize ( SimpleRTKTCL tcl )
  SWIG_add_source_to_module ( SimpleRTKTCL swig_generated_source SimpleRTK.i ${SWIG_EXTRA_DEPS} )
  set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/SimpleRTKTCL_wrap.cxx PROPERTIES COMPILE_FLAGS "-w" )
  add_executable ( SimpleRTKTclsh SimpleRTKTCL_wrap.cxx )
  target_link_libraries ( SimpleRTKTclsh ${SimpleRTK_LIBRARIES} ${TCL_LIBRARY} )
  srtk_strip_target( SimpleRTKTclsh )

endif()

#
# R SWIG configuration
#
if ( WRAP_R )
  find_package ( R REQUIRED )
  include_directories ( ${R_INCLUDE_DIR} )

  # Run swig
  set(CMAKE_SWIG_FLAGS ${CMAKE_SWIG_GLOBAL_FLAGS})
  set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR})
  set(SWIG_MODULE_SimpleRTK_EXTRA_DEPS ${SWIG_EXTRA_DEPS}
    ${CMAKE_CURRENT_SOURCE_DIR}/R.i )
  SWIG_add_module ( SimpleRTK r SimpleRTK.i SimpleRTKR_wrap.cxx )
  SWIG_link_libraries ( SimpleRTK ${SimpleRTK_LIBRARIES} )

  # on some platforms the r libraries are not required at link time...
  if(R_LIBRARIES)
    SWIG_link_libraries ( SimpleRTK ${R_LIBRARIES} )
  endif()

  set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/SimpleRTKR_wrap.cxx PROPERTIES COMPILE_FLAGS "-w")

  get_target_property( SIMPLERTKR_BINARY_MODULE ${SWIG_MODULE_SimpleRTK_TARGET_NAME} LOCATION )
  file(TO_NATIVE_PATH "${SIMPLERTKR_BINARY_MODULE}" SIMPLERTKR_NATIVE_BINARY_MODULE )


  add_dependencies( ${SWIG_MODULE_SimpleRTK_TARGET_NAME} ConfigureFileBuildtime )

  # set the output directory for the R library to the binary packaging location
  set_target_properties( ${SWIG_MODULE_SimpleRTK_TARGET_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Rpackaging/SimpleRTK/src/)

  srtk_strip_target( ${SWIG_MODULE_SimpleRTK_TARGET_NAME} )

  # copy the R files a binary package
  add_custom_command( TARGET ${SWIG_MODULE_SimpleRTK_TARGET_NAME}
      PRE_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy_directory ${SimpleRTK_SOURCE_DIR}/Wrapping/Rpackaging ${CMAKE_CURRENT_BINARY_DIR}/Rpackaging
      COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/Rpackaging/SimpleRTK/data/
  )
  add_custom_command( TARGET ${SWIG_MODULE_SimpleRTK_TARGET_NAME}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/RLib
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/SimpleRTK.R ${CMAKE_CURRENT_BINARY_DIR}/Rpackaging/SimpleRTK/R/
      # copy sample images
      COMMAND ${CMAKE_COMMAND} -E copy ${SimpleRTK_BINARY_DIR}/ExternalData/Testing/Data/Input/cthead1.png  ${CMAKE_CURRENT_BINARY_DIR}/Rpackaging/SimpleRTK/data/
      COMMAND ${CMAKE_COMMAND} -E copy ${SimpleRTK_BINARY_DIR}/ExternalData/Testing/Data/Input/cthead1-Float.mha  ${CMAKE_CURRENT_BINARY_DIR}/Rpackaging/SimpleRTK/data/
      # install for running tests and create binary package
      COMMAND ${R_COMMAND} CMD INSTALL --build ${CMAKE_CURRENT_BINARY_DIR}/Rpackaging/SimpleRTK --library=${CMAKE_CURRENT_BINARY_DIR}/RLib

      COMMENT "Installing R package for testing and building binary version for distribution"
       )

endif()

# Packaging for distribution
add_subdirectory(dist)
