
# Eric's cheat sheet for running configure

DOXYGEN=/home/erik/doxygen/doxygen-1.8.7/bin/doxygen ./configure --with-logging --prefix=$IDIR CPPFLAGS=-I$IDIR/include LDFLAGS=-L$IDIR/lib CXXFLAGS=-Wno-unused-local-typedefs
#./configure --without-log4cxx --prefix=$IDIR CPPFLAGS=-I$IDIR/include LDFLAGS=-L$IDIR/lib CXXFLAGS=-Wno-unused-local-typedefs

