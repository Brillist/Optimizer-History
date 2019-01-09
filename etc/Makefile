###############################################################################
### Makefile for interior (abstract) build directories ########################
###############################################################################

ifeq ($(SRCROOT),)
	SRCROOT = ..
endif

###############################################################################

default:	all

###############################################################################

include $(SRCROOT)/makefiles/Makefile.common

###############################################################################

all:
	@if [ "$(SUBDIRS)" != "" ]; then \
		for dir in $(SUBDIRS); do \
			cd $${dir} ; \
			if ! $(MAKE) all ; then break ; fi ; \
			cd .. ; \
		done ; \
	fi

###############################################################################

clean:
	@$(CMD_CLEAN)
	@if [ "$(SUBDIRS)" != "" ]; then \
		for dir in $(SUBDIRS); do \
			cd $${dir} ; \
			if ! $(MAKE) clean ; then break ; fi ; \
			cd .. ; \
		done ; \
	fi

###############################################################################

depend:
	@if [ "$(SUBDIRS)" != "" ]; then \
		for dir in $(SUBDIRS); do \
			cd $${dir} ; \
			if [ -f Makefile ]; then \
				if ! $(MAKE) depend; then \
					break ; \
				fi ; \
			fi ; \
			cd .. ; \
		done ; \
	fi

###############################################################################

distclean:
	@$(CMD_CLEAN)
	@$(CMD_DISTCLEAN)
	@if [ "$(SUBDIRS)" != "" ]; then \
		for dir in $(SUBDIRS); do \
			cd $${dir} ; \
			if ! $(MAKE) distclean ; then break ; fi ; \
			cd .. ; \
		done ; \
	fi

###############################################################################

install:	all
	@for dir in $(SUBDIRS); do \
		cd $${dir} ; \
		if ! $(MAKE) install ; then break ; fi ; \
		cd .. ; \
	done
