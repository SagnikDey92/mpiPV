# Makefile for MPIP	-*-Makefile-*-
# Please see license in doc/UserGuide.html
# Defs.mak.  Generated from Defs.mak.in by configure.
# $Id: Defs.mak.in 369 2006-10-05 19:21:12Z chcham $

SHELL	= /bin/sh
CC	= mpicc
CXX	= mpicxx
FC	= /usr/bin/gfortran
AR	= ar
RANLIB	= ranlib
PYTHON  = python

CFLAGS = -g -O2
FFLAGS = 

USE_GETARG   = false
USE_LIBDWARF = no

ifneq (-g,$(findstring -g,$(CFLAGS)))
CFLAGS += -g
endif
ifneq (-g,$(findstring -g,$(FFLAGS)))
FFLAGS += -g
endif

INSTALL = /usr/bin/install -c
INSTALL_PROGRAM = ${INSTALL}
INSTALL_DATA = ${INSTALL} -m 644

LIBS          = -ldl -lm -L/work/bin/binutils/lib  -lbfd  -lz -liberty -lunwind
LDFLAGS       = -L/work/bin/libunwind/lib -lunwind -L/work/bin/binutils/lib -lbfd -liberty -L/work/bin/mpich-3.1.4-install/lib
F77_SYMBOLS   = symbol_
BINUTILS_DIR  = /work/bin/binutils
BIN_TYPE_FLAG = 

CANONICAL_BUILD     = x86_64-unknown-linux-gnu
CANONICAL_TARGET    = x86_64-unknown-linux-gnu
TARGET_OS           = linux-gnu
TARGET_CPU          = x86_64


ifneq ( $(srcdir), "." )
CPPFLAGS   += -I.
endif
CPPFLAGS    = -I$(srcdir) -I. -I/work/bin/libunwind/include -I/work/bin/binutils/include -I/work/bin/mpich-3.1.4-install/include -I/work/bin/binutils/include

# check if we're *really* cross-compiling
ifeq (${CANONICAL_TARGET},${CANONICAL_BUILD})
OS	= $(shell uname)
ARCH    = $(shell uname -m)
else
OS      = ${TARGET_OS}
ARCH    = ${TARGET_CPU}
endif

ifeq ($(OS),UNICOS/mp)
  OS    = UNICOS_mp
endif

ifeq ($(OS),OSF1)
  LIBS += -lexc
endif

ifeq ($(OS),Linux)
  ifeq ($(ARCH),i686)
    CPPFLAGS += -DIA32
  endif
  ifeq ($(ARCH),alpha)
    CPPFLAGS += -Dalpha
  endif
  ifeq ($(ARCH),x86_64)
    CPPFLAGS += -DX86_64
  endif
  ifeq ($(ARCH),ppc64)
    CPPFLAGS += -Dppc64
  endif

endif

ifeq (${OS},catamount)
OS      = Catamount
ifeq (${ARCH},x86_64)
  CPPFLAGS  += -DX86_64
endif
endif

C_TARGET = libmpiP.a
API_TARGET = libmpiPapi.a
MPIPLIB  = mpiP
MPIPFLIB = mpiP

BUILD_FLIB=false
ifeq ($(USE_GETARG),true)
  BUILD_FLIB=true
  FORTRAN_FLAG = -DUSE_GETARG
endif
ifneq ($(OS),Linux)
  BUILD_FLIB=true
endif

ifeq ($(BUILD_FLIB),true)
FORTRAN_TARGET = libmpiPg77.a
MPIPFLIB       = mpiPg77
FORTRAN_FLAG   := $(FORTRAN_FLAG) -DGNU_Fortran
F77_VENDOR     = GNU
endif

DEMANGLE_TARGET =
DO_DEMANGLE = false
ENABLE_BFD = yes
MPIPCXXLIB  = mpiP

ifeq ($(ENABLE_BFD),yes)

ifeq ($(DO_DEMANGLE),GNU)
  CPPFLAGS += -DDEMANGLE_$(DO_DEMANGLE)
endif
ifeq ($(DO_DEMANGLE),IBM)
  DEMANGLE_FLAG = -DDEMANGLE_$(DO_DEMANGLE)
  DEMANGLE_TARGET = libmpiPdmg.a
  MPIPCXXLIB = mpiPdmg
  CPPFLAGS := -I/usr/include $(CPPFLAGS)
endif
ifeq ($(DO_DEMANGLE),Compaq)
  DEMANGLE_FLAG = -DDEMANGLE_$(DO_DEMANGLE)
  DEMANGLE_TARGET = libmpiPdmg.a
  MPIPCXXLIB = mpiPdmg
  CPPFLAGS := -I/usr/include $(CPPFLAGS)
  CXXLIBS += -lmld
endif

endif


ifneq ($(ARCH),ppc64)
  CPPFLAGS+= -D${OS}
endif
LFLAGS	+=
LIBS	+=


ENABLE_API_ONLY = no
##### EOF
