include_directories(${mosquitto_SOURCE_DIR} ${mosquitto_SOURCE_DIR}/include
			${OPENSSL_INCLUDE_DIR} ${STDBOOL_H_PATH} ${STDINT_H_PATH})
link_directories(${mosquitto_SOURCE_DIR})

add_library(mosquitto_theengs_decoder MODULE mosquitto_theengs_decoder.c)
set_target_properties(mosquitto_theengs_decoder PROPERTIES
	POSITION_INDEPENDENT_CODE 1
)
set_target_properties(mosquitto_theengs_decoder PROPERTIES PREFIX "")
if(WIN32)
	target_link_libraries(mosquitto_theengs_decoder mosquitto)
endif(WIN32)

# Don't install, these are example plugins only.
#install(TARGETS mosquitto_theengs_decoder RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
