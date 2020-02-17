lib_LTLIBRARIES = libgnss.la

## Only use the following flags for debuging purposes
libgnss_la_CXXFLAGS = \
	-std=c++17 \
	-g \
	-pg \
	-Wall \
	-Wextra \
	-Werror \
	-pedantic \
	-W \
	-Wshadow \
	-Winline \
	-Wdisabled-optimization \
	-DDEBUG

dist_include_HEADERS = \
	satsys.hpp \
	satellite.hpp \
	gnssobs.hpp \
        gnssobsrv.hpp \
	antenna.hpp \
        bern_utils.hpp \
        antenna_pcv.hpp \
	antex.hpp \
        navrnx.hpp

dist_libgnss_la_SOURCES = \
	satsys.cpp \
        gnssobs.cpp \
        gnssobsrv.cpp \
        bern_utils.cpp \
	antenna.cpp \
        antenna_pcv.cpp \
	antex.cpp \
        navrnx.cpp \
	gpsnav.cpp \
	glonav.cpp
