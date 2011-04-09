project(kaccessible)

#find_package(Qt4 REQUIRED)
#find_package(KDE4 REQUIRED)
#find_package(X11 REQUIRED)
#include(${QT_USE_FILE})

##################################################################

include_directories(X11_INCLUDE_DIR ${KDE4_INCLUDES})
set(kaccessiblebridge_SRCS kaccessiblebridge.cpp)
kde4_add_plugin(kaccessiblebridge ${kaccessiblebridge_SRCS})
target_link_libraries(kaccessiblebridge ${QT_LIBRARIES} ${KDE4_KDEUI_LIBS} ${X11_LIBRARIES})
#install(TARGETS kaccessiblebridge DESTINATION ${PLUGIN_INSTALL_DIR})
#install(TARGETS kaccessiblebridge DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/accessible)
install(TARGETS kaccessiblebridge DESTINATION ${PLUGIN_INSTALL_DIR}/plugins/accessiblebridge)

##################################################################

#include(PkgConfigGetVar)
#include(DBusMacros)
include(UsePkgConfig)
MACRO(PKGCONFIG_GETVAR _package _var _output_variable)
  SET(${_output_variable})
  IF(PKGCONFIG_EXECUTABLE)
    EXEC_PROGRAM(${PKGCONFIG_EXECUTABLE} ARGS ${_package} --exists RETURN_VALUE _return_VALUE OUTPUT_VARIABLE _pkgconfigDevNull )
    IF(NOT _return_VALUE)
      EXEC_PROGRAM(${PKGCONFIG_EXECUTABLE} ARGS ${_package} --variable ${_var} OUTPUT_VARIABLE ${_output_variable} )
    ENDIF(NOT _return_VALUE)
  ENDIF(PKGCONFIG_EXECUTABLE)
ENDMACRO(PKGCONFIG_GETVAR _package _var _output_variable)
macro(dbus_add_activation_service _sources)
    PKGCONFIG_GETVAR(dbus-1 session_bus_services_dir _install_dir)
    foreach (_i ${_sources})
        get_filename_component(_service_file ${_i} ABSOLUTE)
        string(REGEX REPLACE "\\.service.*$" ".service" _output_file ${_i})
        set(_target ${CMAKE_CURRENT_BINARY_DIR}/${_output_file})
        configure_file(${_service_file} ${_target})
        install(FILES ${_target} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/dbus-1/services )
    endforeach (_i ${ARGN})
endmacro(dbus_add_activation_service _sources)

dbus_add_activation_service(org.kde.kaccessible.service.in)

##################################################################

# set(kded_kdeaccessiblekded_SRCS kdeaccessiblekded.cpp)
# #QT4_GENERATE_DBUS_INTERFACE(dbus/dbusapi_screens.h org.kde.Kephal.Screens.xml)
# #QT4_ADD_DBUS_ADAPTOR (kded_kdeaccessiblekded_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Kephal.Screens.xml dbus/dbusapi_screens.h DBusAPIScreens)
# #QT4_GENERATE_DBUS_INTERFACE(dbus/dbusapi_outputs.h org.kde.Kephal.Outputs.xml)
# #QT4_ADD_DBUS_ADAPTOR (kded_kdeaccessiblekded_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Kephal.Outputs.xml dbus/dbusapi_outputs.h DBusAPIOutputs)
# #QT4_GENERATE_DBUS_INTERFACE(dbus/dbusapi_configurations.h org.kde.Kephal.Configurations.xml)
# #QT4_ADD_DBUS_ADAPTOR (kded_kdeaccessiblekded_SRCS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Kephal.Configurations.xml dbus/dbusapi_configurations.h DBusAPIConfigurations)
#
# kde4_add_plugin(kded_kdeaccessiblekded ${kded_kdeaccessiblekded_SRCS})
# target_link_libraries(kded_kdeaccessiblekded ${KDE4_KDECORE_LIBS} ${KDE4_KDEUI_LIBS} ${QT_QTCORE_LIBRARY} ${QT_QTDBUS_LIBRARY} ${QT_QTXML_LIBRARY} ${X11_LIBRARIES} ${X11_Xrandr_LIB})
# install(TARGETS kded_kdeaccessiblekded DESTINATION ${PLUGIN_INSTALL_DIR})
# install(FILES kdeaccessiblekded.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kded)

##################################################################

macro_optional_find_package(Speechd)
macro_log_feature(SPEECHD_FOUND "speechd" "Speech Dispatcher provides a high-level device independent layer for speech synthesis" "http://www.freebsoft.org/speechd" FALSE "" "Speech dispatcher is an optional dependency for kaccessible.")

if(SPEECHD_FOUND)
  set(SPEECH_LIB ${SPEECHD_LIBRARIES})
  add_definitions(-DSPEECHD_FOUND)
endif(SPEECHD_FOUND)

set(kaccessibleapp_SRCS kaccessibleapp.cpp)
qt4_wrap_cpp(kaccessibleapp_SRCS kaccessibleapp.h)
add_executable(kaccessibleapp ${kaccessibleapp_SRCS})
#INCLUDE_DIRECTORIES(. .. ${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(kaccessibleapp ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${KDE4_KDEUI_LIBS} ${QT_QTDBUS_LIBRARY} ${SPEECH_LIB})
install(TARGETS kaccessibleapp RUNTIME DESTINATION ${LIBEXEC_INSTALL_DIR})
