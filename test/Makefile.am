noinst_PROGRAMS = \
		testAntenna.out \
		testAntex.out

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

testAntenna_out_SOURCES   = test_antenna.cpp
testAntenna_out_CXXFLAGS  = $(MCXXFLAGS) -I$(top_srcdir)/src -L$(top_srcdir)/src
testAntenna_out_LDADD     = $(top_srcdir)/src/libgnss.la
testAntex_out_SOURCES   = test_antex.cpp
testAntex_out_CXXFLAGS  = $(MCXXFLAGS) -I$(top_srcdir)/src -L$(top_srcdir)/src
testAntex_out_LDADD     = $(top_srcdir)/src/libgnss.la
