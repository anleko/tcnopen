#//
#// $Id$
#//
#// DESCRIPTION    trdp Makefile
#//
#// AUTHOR         Bernd Loehr, NewTec GmbH
#//
#// All rights reserved. Reproduction, modification, use or disclosure
#// to third parties without express authority is forbidden.
#// Copyright Bombardier Transportation GmbH, Germany, 2012.
#//

# Preliminary: Currently, posix support only

TARGET_VOS = posix
TARGET_FLAG = POSIX

# --------------------------------------------------------------------

VOS_PATH = src/vos/$(TARGET_VOS)

vpath %.c src/common src/example src/vos/common $(VOS_PATH) test
vpath %.h src/api src/vos/api src/common src/vos/common

INCPATH = src/api
VOS_INCPATH = src/vos/api -I src/common
BUILD_PATH = bld/$(TARGET_VOS)

ifdef TARGET
DIR_PATH=$(TARGET)/
CROSS=$(TARGET)-

CC=$(CROSS)gcc
AR=$(CROSS)ar
LD=$(CROSS)ld
STRIP=$(CROSS)strip
else
CC = $(GNUPATH)gcc
AR = $(GNUPATH)ar
LD = $(GNUPATH)ld
STRIP = $(GNUPATH)strip
endif

ECHO = echo
RM = rm -f
MD = mkdir -p
CP = cp


ifeq ($(MD_SUPPORT),1)
CFLAGS += -D__USE_BSD -D_DARWIN_C_SOURCE -D_XOPEN_SOURCE=500 -pthread -D$(TARGET_FLAG)  -fPIC -Wall -DMD_SUPPORT=1
else
CFLAGS += -D__USE_BSD -D_DARWIN_C_SOURCE -D_XOPEN_SOURCE=500 -pthread -D$(TARGET_FLAG)  -fPIC -Wall -DMD_SUPPORT=0
endif

SUBDIRS	= src
INCLUDES = -I $(INCPATH) -I $(VOS_INCPATH) -I $(VOS_PATH)
OUTDIR = $(BUILD_PATH)

LDFLAGS = -L $(OUTDIR) 

ifeq ($(DEBUG),1)
CFLAGS += -g -O -DDEBUG
LDFLAGS += -g
# Display the strip command and do not execut it
STRIP = $(ECHO) "do NOT strip: " 
else
CFLAGS += -Os  -DNO_DEBUG
endif

VOS_OBJS = vos_utils.o vos_sock.o vos_mem.o vos_thread.o
TRDP_OBJS = trdp_pdcom.o trdp_utils.o trdp_if.o trdp_stats.o $(VOS_OBJS)

ifeq ($(MD_SUPPORT),1)
TRDP_OBJS += trdp_mdcom.o
endif

all:		outdir libtrdp demo

libtrdp:	outdir $(OUTDIR)/libtrdp.a
demo:		outdir $(OUTDIR)/receiveSelect $(OUTDIR)/cmdlineSelect $(OUTDIR)/receivePolling $(OUTDIR)/sendHello
test:		outdir $(OUTDIR)/test_server $(OUTDIR)/test_client1
doc:		doc/latex/refman.pdf

$(OUTDIR)/trdp_if.o:	trdp_if.c
			$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OUTDIR)/%.o: %.c %.h trdp_if_light.h trdp_types.h vos_types.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@


$(OUTDIR)/libtrdp.a:	$(addprefix $(OUTDIR)/,$(notdir $(TRDP_OBJS)))
			@$(ECHO) ' ### Building the lib $(@F)'
			$(RM) $@
			$(AR) cq $@ $^


$(OUTDIR)/receiveSelect:  echoSelect.c  $(OUTDIR)/libtrdp.a 
			@$(ECHO) ' ### Building application $(@F)'
			$(CC) $(SUBDIRS)/example/echoSelect.c \
				$(CFLAGS) $(INCLUDES) -o $@\
				-ltrdp \
			    $(LDFLAGS) \
			    
			$(STRIP) $@

			
$(OUTDIR)/cmdlineSelect:  echoSelect.c  $(OUTDIR)/libtrdp.a 
			@$(ECHO) ' ### Building application $(@F)'
			$(CC) $(SUBDIRS)/example/echoSelectcmdline.c \
				$(CFLAGS) $(INCLUDES) -o $@\
				-ltrdp \
			    $(LDFLAGS) \
			    
			$(STRIP) $@
			
$(OUTDIR)/receivePolling:  echoPolling.c  $(OUTDIR)/libtrdp.a 
			@$(ECHO) ' ### Building application $(@F)'
			$(CC) $(SUBDIRS)/example/echoPolling.c \
				$(CFLAGS) $(INCLUDES) -o $@\
				-ltrdp \
			    $(LDFLAGS) \
			    
			$(STRIP) $@

$(OUTDIR)/sendHello:   sendHello.c  $(OUTDIR)/libtrdp.a 
			@$(ECHO) ' ### Building application $(@F)'
			$(CC) $(SUBDIRS)/example/sendHello.c \
			    -ltrdp \
			    $(LDFLAGS) $(CFLAGS) $(INCLUDES) \
			    -o $@
			$(STRIP) $@

$(OUTDIR)/test_server:   test_server_main.c  $(OUTDIR)/libtrdp.a 
			@$(ECHO) ' ### Building test server application $(@F)'
			$(CC) test/test_server_main.c \
			    test/test_general.c \
			    -ltrdp \
			    $(LDFLAGS) $(CFLAGS) $(INCLUDES) \
			    -o $@
			$(STRIP) $@

$(OUTDIR)/test_client1:   test_client_main1.c  $(OUTDIR)/libtrdp.a 
			@$(ECHO) ' ### Building test client application $(@F)'
			$(CC) test/test_client_main1.c \
			    test/test_general.c \
			    -ltrdp \
			    $(LDFLAGS) $(CFLAGS) $(INCLUDES) \
			    -o $@
			$(STRIP) $@

outdir:
		$(MD) $(OUTDIR)


doc/latex/refman.pdf: Doxyfile trdp_if_light.h trdp_types.h
			@$(ECHO) ' ### Making the PDF document'
			doxygen Doxyfile
			make -C doc/latex
			$(CP) doc/latex/refman.pdf doc



help:
	@echo " " >&2
	@echo "BUILD TARGETS FOR TRDP" >&2
	@echo "Edit the paths in the top part of this Makefile to suit your environment." >&2
	@echo "Then call 'make' or 'make all' to build everything." >&2
	@echo "To build debug binaries, append 'DEBUG=TRUE' to the make command " >&2
	@echo "To include message data support, append 'MD_SUPPORT=TRUE' to the make command " >&2
	@echo " " >&2
	@echo "Other builds:" >&2
	@echo "  * make demo      - build the sample applications" >&2
	@echo "  * make test      - build the test server application" >&2
	@echo "  * make clean     - remove all binaries and objects of the current target" >&2
	@echo "  * make libtrdp	  - build the static library, only" >&2
	@echo " " >&2
	@echo "  * make doc	      - build the documentation (refman.pdf)" >&2
	@echo "                   - (needs doxygen installed in executable path)" >&2
	@echo " " >&2

#########################################################################


clean:
	$(RM) -r $(OUTDIR)/*
	$(RM) -r doc/latex/*


#########################################################################
