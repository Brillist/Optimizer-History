###############################################################################
### Makefile for package directory ############################################
###############################################################################

ifeq ($(SRCROOT),)
    SRCROOT = ..
endif

###############################################################################

all:		objs

###############################################################################

include $(SRCROOT)/makefiles/Makefile.build
include $(SRCROOT)/makefiles/Makefile.common

###############################################################################

clean:
	@$(CMD_CLEAN)

###############################################################################

depend:
	@rm -f .depend*
	@if ls *.cpp >/dev/null 2>&1; then \
		$(CC) $(CC_OPTS) -M *.cpp > .depend; \
	fi

###############################################################################

distclean:	clean
	@$(CMD_DISTCLEAN)

###############################################################################

objs:		$(OBJS)

###############################################################################
### override/extend with local setup ##########################################
###############################################################################

ifeq ($(SRCROOT)/makefiles/Makefile.package.local,\
      $(wildcard $(SRCROOT)/makefiles/Makefile.package.local))
    include $(SRCROOT)/makefiles/Makefile.package.local
endif
