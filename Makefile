# Makefile for MPIP	-*-Makefile-*-
# Please see license in doc/UserGuide.html
# Makefile.  Generated from Makefile.in by configure.
# $Id: Makefile.in 498 2013-07-18 22:12:41Z chcham $

srcdir=.
prefix=/work/bin/mpiPviz
exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include
slibdir=${exec_prefix}/lib
bindir=${exec_prefix}/bin
datarootdir=${prefix}/share
datadir=${datarootdir}
libunwind_loc=/work/bin/libunwind


PACKAGE_TARNAME=mpip
docdir=${datarootdir}/doc/${PACKAGE_TARNAME}

include Defs.mak

.PHONY: default all clean api API shared proto cleanobjs test testing \
        add_binutils_objs add_libunwind_objs TAGS cleanobjs distclean \
        lint indent proto install merged-install install-api install-bin \
        install-all

default: ${C_TARGET} ${FORTRAN_TARGET} ${DEMANGLE_TARGET}

all: ${C_TARGET} ${FORTRAN_TARGET} ${DEMANGLE_TARGET} API test
	@echo All done.

PC_LOOKUP_FILE=pc_lookup.c
PC_LOOKUP_OBJ = $(PC_LOOKUP_FILE:.c=.o)

SRCS =	diag_msgs.c \
	mpiP-hash.c \
	glob.c \
	wrappers.c \
	wrappers_special.c \
	mpiPi.c \
	util.c \
	lookup.c \
	report.c \
	pcontrol.c \
	mpiP-API.c \
	record_stack.c \
	${PC_LOOKUP_FILE}

API_SRCS =	diag_msgs_api.c \
	mpiP-hash.c \
	glob.c \
	mpiPi.c \
	util.c \
	record_stack.c \
	mpiP-API.c \
	${PC_LOOKUP_FILE}

OBJS  = $(SRCS:.c=.o)
FOBJS = $(OBJS)

API_OBJS = $(API_SRCS:.c=.o)

FORTRAN_GETARG_OBJ_FILE=get_fortran_arg.o
FOBJS += $(FORTRAN_GETARG_OBJ_FILE)

clean::
	rm -f $(FORTRAN_GETARG_OBJ_FILE)


#  If using BFD and need a separate demangling library,
#  defensively rebuild pc_lookup.o for each target
ifeq ($(ENABLE_BFD),yes)
ifneq ($(DEMANGLE_TARGET),)

${C_TARGET} ${FORTRAN_TARGET} :: 
	rm -f $(PC_LOOKUP_OBJ)
	$(MAKE) $(PC_LOOKUP_OBJ)

${DEMANGLE_TARGET} :: 
	rm -f $(PC_LOOKUP_OBJ)
	$(MAKE) CPPFLAGS="${CPPFLAGS} ${DEMANGLE_FLAG}" $(PC_LOOKUP_OBJ)

ifeq ($(ENABLE_API_ONLY),no)
${API_TARGET}:: 
	rm -f $(PC_LOOKUP_OBJ)
	$(MAKE) $(PC_LOOKUP_OBJ)
endif
endif
endif

#  If using Fortran iargc/getarg intrinsics to get command-line arguments,
#  defensively rebuild util.o for each target
ifeq ($(USE_GETARG),true)
${C_TARGET} ${DEMANGLE_TARGET} :: 
	rm -f util.o
	$(MAKE) util.o

${FORTRAN_TARGET} ::
	rm -f util.o
	$(MAKE) util.o CPPFLAGS="${CPPFLAGS} ${FORTRAN_FLAG}"

ifeq ($(ENABLE_API_ONLY),no)
${API_TARGET}:: 
	rm -f util.o
	$(MAKE) util.o
endif
endif


${C_TARGET}:: ${OBJS} 
	${AR} ruv $@ ${OBJS}
	${RANLIB} $@

${FORTRAN_TARGET}:: ${FOBJS} 
	${AR} ruv $@ ${FOBJS}
	${RANLIB} $@

${DEMANGLE_TARGET}:: ${OBJS} 
	${AR} ruv $@ ${OBJS}
	${RANLIB} $@
	rm -f pc_lookup.o

api API: ${API_TARGET}

${API_TARGET}:: ${API_OBJS} 
	${AR} ruv $@ ${API_OBJS}
	${RANLIB} $@


ifeq ($(OS), Linux)
SHARED_C_TARGET = $(C_TARGET:.a=.so)
SHARED_FORTRAN_TARGET = $(FORTRAN_TARGET:.a=.so)
SHARED_DEMANGLE_TARGET = $(DEMANGLE_TARGET:.a=.so)

shared: ${SHARED_C_TARGET}
install::
	if [ -x ${SHARED_C_TARGET} ] ; then \
	  mkdir -p ${DESTDIR}${slibdir} ; \
	  ${INSTALL} ${SHARED_C_TARGET} ${DESTDIR}${slibdir} ; \
	fi
clean::
	rm -f ${SHARED_C_TARGET}

${SHARED_C_TARGET}: CFLAGS += -fpic -DPIC
${SHARED_C_TARGET}: cleanobjs ${OBJS}
	${CC} -shared -o $@ ${OBJS} ${LDFLAGS} ${LIBS}

endif


clean :: 
	rm -f ${API_OBJS}

ifeq (${ARCH}, x86_64)
BINUTILS_LIB_DIR=lib64
else
BINUTILS_LIB_DIR=lib
endif

BFD_LIBS = libbfd.a libiberty.a
ifeq (${OS}, AIX)
BFD_LIBS := ${BFD_LIBS} libintl.a
endif

add_binutils_objs:
	-rm -rf mpip_temp_obj_dir
	-mkdir mpip_temp_obj_dir
	cd mpip_temp_obj_dir; \
	for lib in ${BFD_LIBS} ; do \
	   ar -x ${BIN_TYPE_FLAG} ${BINUTILS_DIR}/${BINUTILS_LIB_DIR}/$${lib} ; \
	done ; \
	if [ "x$(ENABLE_API_ONLY)" = "xno" ] ; then \
	  ar -q ${BIN_TYPE_FLAG} ../${C_TARGET} *.o ; \
	  if [ "x${FORTRAN_TARGET}" != "x" ]; then \
	    ar -q ${BIN_TYPE_FLAG} ../${FORTRAN_TARGET} *.o ; fi ; \
	  if [ "x${DEMANGLE_TARGET}" != "x" ]; then \
	    ar -q ${BIN_TYPE_FLAG} ../${DEMANGLE_TARGET} *.o ; fi ; \
	fi ; \
	if [ "x${API_TARGET}" != "x" ]; then \
	  ar -q ${BIN_TYPE_FLAG} ../${API_TARGET} *.o ; fi ; \
	cd .. ; \
rm -rf mpip_temp_obj_dir

add_libunwind_objs:
	-mkdir mpip_temp_obj_dir
	cd mpip_temp_obj_dir ; \
	cp ${libunwind_loc} ./${C_TARGET} ; \
	ar x ../${C_TARGET} ; \
	ar -q ${BIN_TYPE_FLAG} ./${C_TARGET} ./*.o ; \
	mv ./${C_TARGET} .. ; \
	if [ "${FORTRAN_TARGET}" ]; then \
	  rm *.o ; \
	  cp ${libunwind_loc} ./${FORTRAN_TARGET} ; \
	  ar x ../${FORTRAN_TARGET} ; \
	  ar -q ${BIN_TYPE_FLAG} ./${FORTRAN_TARGET} ./*.o ; \
	  mv ./${FORTRAN_TARGET} .. ; fi ; \
	if [ "${DEMANGLE_TARGET}" ]; then \
	  rm *.o ; \
	  cp ${libunwind_loc} ./${DEMANGLE_TARGET} ; \
	  ar x ../${DEMANGLE_TARGET} ; \
	  ar -q ${BIN_TYPE_FLAG} ./${DEMANGLE_TARGET} ./*.o ; \
	  mv ./${DEMANGLE_TARGET} .. ; fi ; \
	cd .. ; \
	rm -rf mpip_temp_obj_dir

TAGS: ${SRCS}
	etags $^

test testing:
	$(MAKE) -C testing

clean::
	rm -f TAGS tags
	$(MAKE) -C testing clean

include $(srcdir)/Rules.mak

ENABLE_FORTRAN_XLATE = yes
ifeq ($(ENABLE_FORTRAN_XLATE), yes)
  MAKE_WRAPPERS_ARGS += --xlate
endif

ENABLE_FORTRAN_WEAK_SYMS = no
ifeq ($(ENABLE_FORTRAN_WEAK_SYMS), yes)
  MAKE_WRAPPERS_ARGS += --weak
endif

USE_SETJMP = no
ifeq ($(USE_SETJMP), yes)
  MAKE_WRAPPERS_ARGS += --usesetjmp
endif

wrappers.c symbols.h lookup.c mpiPi_def.h: mpi.protos.txt make-wrappers.py
	$(PYTHON) $(srcdir)/make-wrappers.py $(MAKE_WRAPPERS_ARGS) --arch=$(ARCH) --f77symbol $(F77_SYMBOLS) $^

ifeq ($(ENABLE_API_ONLY), no)
${OBJS}: mpiPconfig.h mpiPi.h mpiPi_def.h mpiPi_proto.h symbols.h mpip_timers.h
else
${API_OBJS} : mpiPconfig.h mpiPi.h mpiPi_proto.h mpip_timers.h
endif

install:: ${C_TARGET} ${DEMANGLE_TARGET} ${FORTRAN_TARGET} 
	mkdir -p ${DESTDIR}${libdir} ${DESTDIR}${docdir}
	${INSTALL} ${C_TARGET} ${DESTDIR}${libdir}/${C_TARGET}
	if test "x${DEMANGLE_TARGET}" != "x" ; then ${INSTALL} ${DEMANGLE_TARGET} ${DESTDIR}${libdir}/${DEMANGLE_TARGET} ; fi
	if test "x${FORTRAN_TARGET}" != "x" ; then ${INSTALL} ${FORTRAN_TARGET} ${DESTDIR}${libdir}/${FORTRAN_TARGET} ; fi
	${INSTALL} doc/*txt doc/*html doc/README ${DESTDIR}${docdir} ; \

install-api: API
	mkdir -p ${DESTDIR}${libdir} ${DESTDIR}${includedir}/mpip_timers 
	if test "x${API_TARGET}" != "x" ; then ${INSTALL} ${API_TARGET} ${DESTDIR}${libdir}/${API_TARGET} ; fi
	${INSTALL} mpiP-API.h mpip_timers.h ${DESTDIR}${includedir}
	${INSTALL} mpip_timers/*h ${DESTDIR}${includedir}/mpip_timers

install-bin:
	mkdir -p ${DESTDIR}${bindir}
	${INSTALL} bin/*mpip* ${DESTDIR}${bindir} ; \

install-all: install install-api install-bin 

merged-install: add_binutils_objs add_libunwind_objs install

uninstall:
	if [ ${prefix} != `pwd` -a ${prefix} != "." ] ; then \
	rm -f ${DESTDIR}${bindir}/mpip-insert-src ${DESTDIR}${bindir}/mpirun-mpip ${DESTDIR}${bindir}/srun-mpip ; \
	rm -f ${DESTDIR}${libdir}/${C_TARGET} ; \
	if test "x${DEMANGLE_TARGET}" != "x" ; then rm -f ${DESTDIR}${libdir}/${DEMANGLE_TARGET} ; fi ; \
	if test "x${FORTRAN_TARGET}" != "x" ; then rm -f ${DESTDIR}${libdir}/${FORTRAN_TARGET} ; fi ; \
	if test "x${SHARED_C_TARGET}" != "x" && test -e ${DESTDIR}${slibdir}/${SHARED_C_TARGET} ; then rm -f ${DESTDIR}${slibdir}/${SHARED_C_TARGET} ; fi ; \
	if test "x${SHARED_FORTRAN_TARGET}" != "x" && test -e ${DESTDIR}${slibdir}/${SHARED_FORTRAN_TARGET} ; then rm -f ${DESTDIR}${slibdir}/${SHARED_FORTRAN_TARGET} ; fi ; \
	if test "x${SHARED_DEMANGLE_TARGET}" != "x" && test -e ${DESTDIR}${slibdir}/${SHARED_DEMANGLE_TARGET} ; then rm -f ${DESTDIR}${slibdir}/${SHARED_DEMANGLE_TARGET} ; fi ; \
	rm -rf ${DESTDIR}${docdir} ; \
	rm -rf ${DESTDIR}${includedir}/mpip_timers.h ${DESTDIR}${includedir}/mpip_timers ; \
	rm -f ${DESTDIR}${includedir}/mpiP-API.h ; \
	fi

proto:
	cextract -DCEXTRACT $(CPPFLAGS) *.c | sed -e '/find_address/d' -e '/Dwarf/d' > mpiPi_proto.h

clean::
	rm -f wrappers.c symbols.h lookup.c mpiPi_def.h

clean::
	rm -f ${FORTRAN_TARGET} ${DEMANGLE_TARGET} ${API_TARGET}

cleanobjs:
	rm -f ${FOBJS} ${API_OBJS}

MPIINC	= -I/usr/lpp/ppe.poe/include -I /usr/lib/mpi/mpi_intel/include
LINT = gcc -Wall
lint: ${SRCS}
	${LINT} -Dlint -I. ${CFLAGS} ${CPPFLAGS} ${MPIINC} -c ${SRCS}

distclean: clean
	make -C testing clean
	rm -f Makefile Defs.mak Check.mak config.log config.status lookup.c mpiPconfig.h mpiPi_def.h symbols.h wrappers.c testing/Makefile libmpiP* mpi.protos.txt mpiPi.h get_fortran_arg.f mpip_timers/aix_local.h site.exp site.bak testing.sum

include Check.mak

indent:
	indent -gnu *.c *.h *.h.in mpip_timers/*.h mpip_timers/*.h.in

##### EOF
