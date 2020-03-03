noinst_PROGRAMS = \
		testObsCode.out \
		testAntenna.out \
		testAntex.out \
                testBernSatellit.out \
                testNavRnxG.out \
                testNavRnxR.out \
                testGloNavJ12.out

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

AM_LIBS = -lggdatetime -lggeodesy

testObsCode_out_SOURCES   = test_gnssobs.cpp
testObsCode_out_CXXFLAGS  = $(MCXXFLAGS) -I$(top_srcdir)/src 
testObsCode_out_LDADD     = $(top_srcdir)/src/libgnss.la $(AM_LIBS)

testAntenna_out_SOURCES   = test_antenna.cpp
testAntenna_out_CXXFLAGS  = $(MCXXFLAGS) -I$(top_srcdir)/src 
testAntenna_out_LDADD     = $(top_srcdir)/src/libgnss.la $(AM_LIBS)

testBernSatellit_out_SOURCES   = test_bern_SATELLIT.cpp
testBernSatellit_out_CXXFLAGS  = $(MCXXFLAGS) -I$(top_srcdir)/src 
testBernSatellit_out_LDADD     = $(top_srcdir)/src/libgnss.la $(AM_LIBS)

testAntex_out_SOURCES   = test_antex.cpp
testAntex_out_CXXFLAGS  = $(MCXXFLAGS) -I$(top_srcdir)/src 
testAntex_out_LDADD     = $(top_srcdir)/src/libgnss.la $(AM_LIBS)

testNavRnxG_out_SOURCES   = test_navrnx_G.cpp
testNavRnxG_out_CXXFLAGS  = $(MCXXFLAGS) -I$(top_srcdir)/src 
testNavRnxG_out_LDADD     = $(top_srcdir)/src/libgnss.la $(AM_LIBS)

testNavRnxR_out_SOURCES   = test_navrnx_R.cpp
testNavRnxR_out_CXXFLAGS  = $(MCXXFLAGS) -I$(top_srcdir)/src 
testNavRnxR_out_LDADD     = $(top_srcdir)/src/libgnss.la $(AM_LIBS)

testGloNavJ12_out_SOURCES   = testGloNavJ12.cpp
testGloNavJ12_out_CXXFLAGS  = $(MCXXFLAGS) -I$(top_srcdir)/src 
testGloNavJ12_out_LDADD     = $(top_srcdir)/src/libgnss.la $(AM_LIBS)
