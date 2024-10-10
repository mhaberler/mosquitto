include ../../config.mk

.PHONY : all binary check clean reallyclean test install uninstall

PLUGIN_NAME=mosquitto_theengs_decoder

PLUGIN_INCLUDES=-I/home/mah/src/decoder/include/shared	-I/home/mah/src/decoder/src/arduino_json/src
PLUGIN_LIBS=/home/mah/src/decoder/build/libdecoder.a
 
all : binary

binary : ${PLUGIN_NAME}.so

${PLUGIN_NAME}.so : ${PLUGIN_NAME}.cpp
	$(CROSS_COMPILE)$(CXX) $(PLUGIN_CPPFLAGS) $(PLUGIN_INCLUDES) $(PLUGIN_CFLAGS) $(PLUGIN_LDFLAGS) -fPIC -shared $<  $(PLUGIN_LIBS) -lstdc++ -o $@

reallyclean : clean
clean:
	-rm -f *.o ${PLUGIN_NAME}.so *.gcda *.gcno

check: test
test:

install: ${PLUGIN_NAME}.so
	# Don't install, these are examples only.
	#$(INSTALL) -d "${DESTDIR}$(libdir)"
	#$(INSTALL) ${STRIP_OPTS} ${PLUGIN_NAME}.so "${DESTDIR}${libdir}/${PLUGIN_NAME}.so"

uninstall :
	-rm -f "${DESTDIR}${libdir}/${PLUGIN_NAME}.so"
