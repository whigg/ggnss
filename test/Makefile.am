noinst_PROGRAMS = \
		testObsCode.out \
		testObsType.out \
		testAntenna.out \
		testAntex.out \
                testBernSatellit.out \
                testNavRnxG.out \
                testNavRnxR.out \
                testNavRnxE.out \
                testNavRnxC.out \
                testGloNavJ12.out \
                testNavRnx.out \
                testObsRnx.out \
		testSp3.out \
                pprnx.out

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

testObsType_out_SOURCES   = test_gnssobsrv.cpp
testObsType_out_CXXFLAGS  = $(MCXXFLAGS) -I$(top_srcdir)/src 
testObsType_out_LDADD     = $(top_srcdir)/src/libgnss.la $(AM_LIBS)

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

testNavRnxE_out_SOURCES   = test_navrnx_E.cpp
testNavRnxE_out_CXXFLAGS  = $(MCXXFLAGS) -I$(top_srcdir)/src 
testNavRnxE_out_LDADD     = $(top_srcdir)/src/libgnss.la $(AM_LIBS)

testNavRnxC_out_SOURCES   = test_navrnx_C.cpp
testNavRnxC_out_CXXFLAGS  = $(MCXXFLAGS) -I$(top_srcdir)/src 
testNavRnxC_out_LDADD     = $(top_srcdir)/src/libgnss.la $(AM_LIBS)

testNavRnx_out_SOURCES   = test_navrnx.cpp
testNavRnx_out_CXXFLAGS  = $(MCXXFLAGS) -I$(top_srcdir)/src 
testNavRnx_out_LDADD     = $(top_srcdir)/src/libgnss.la $(AM_LIBS)

testGloNavJ12_out_SOURCES   = testGloNavJ12.cpp
testGloNavJ12_out_CXXFLAGS  = $(MCXXFLAGS) -I$(top_srcdir)/src 
testGloNavJ12_out_LDADD     = $(top_srcdir)/src/libgnss.la $(AM_LIBS)

testObsRnx_out_SOURCES   = test_obsrnx.cpp
testObsRnx_out_CXXFLAGS  = $(MCXXFLAGS) -I$(top_srcdir)/src 
testObsRnx_out_LDADD     = $(top_srcdir)/src/libgnss.la $(AM_LIBS)

testSp3_out_SOURCES   = test_sp3.cpp
testSp3_out_CXXFLAGS  = $(MCXXFLAGS) -I$(top_srcdir)/src 
testSp3_out_LDADD     = $(top_srcdir)/src/libgnss.la $(AM_LIBS)

pprnx_out_SOURCES   = pp_rnx.cpp
pprnx_out_CXXFLAGS  = $(MCXXFLAGS) -I$(top_srcdir)/src 
pprnx_out_LDADD     = $(top_srcdir)/src/libgnss.la $(AM_LIBS)
