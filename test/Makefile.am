noinst_PROGRAMS = \
		testObsCode.out \
		testAntenna.out \
		testAntex.out \
                testNavRnx.out

MCXXFLAGS = \
	-std=c++17 \
	-g \
	-pg \
	-Wall \
	-Wextra \
	-Werror \
	-pedantic \
	-W \
	-Wshadow \
	-Wdisabled-optimization \
	-DDEBUG

AM_LIBS = -lggdatetime

testObsCode_out_SOURCES   = test_gnssobs.cpp
testObsCode_out_CXXFLAGS  = $(MCXXFLAGS) -I$(top_srcdir)/src 
testObsCode_out_LDADD     = $(top_srcdir)/src/libgnss.la $(AM_LIBS)

testAntenna_out_SOURCES   = test_antenna.cpp
testAntenna_out_CXXFLAGS  = $(MCXXFLAGS) -I$(top_srcdir)/src 
testAntenna_out_LDADD     = $(top_srcdir)/src/libgnss.la $(AM_LIBS)

testAntex_out_SOURCES   = test_antex.cpp
testAntex_out_CXXFLAGS  = $(MCXXFLAGS) -I$(top_srcdir)/src 
testAntex_out_LDADD     = $(top_srcdir)/src/libgnss.la $(AM_LIBS)

testNavRnx_out_SOURCES   = test_navrnx.cpp
testNavRnx_out_CXXFLAGS  = $(MCXXFLAGS) -I$(top_srcdir)/src 
testNavRnx_out_LDADD     = $(top_srcdir)/src/libgnss.la $(AM_LIBS)
