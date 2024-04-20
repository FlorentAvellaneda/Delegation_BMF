PROGRAMS=iteress

ifneq (, $(findstring Windows, $(OS)))
WINDOWS=1
endif

CFLAGS+=-Wall -ansi -pedantic -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast
ifeq ($(WINDOWS), 1)
CC=gcc
endif

FOPT=-O3 -fomit-frame-pointer

FI686=-march=native
FX86_64=-march=native
FSPARC=-mcpu=native
ARCH=$(shell uname -m)
ifeq (, $(ARCH))
ARCH=i686
endif
ifeq ($(ARCH), i686)
FARCH=$(FI686)
endif
ifeq ($(ARCH), x86_64)
FARCH=$(FX86_64)
endif
ifeq ($(ARCH), sparc64)
FARCH=$(FSPARC)
endif

ifeq ($(WINDOWS), 1)
PLATFORM=windows
else
PLATFORM=$(shell uname -s | tr '[:upper:]' '[:lower:]')
endif

COMPILE=$(CC) $(CFLAGS) $(FOPT) $(FARCH) -o $@ $(filter %.c,$^)
COMPILE_debug=$(CC) $(CFLAGS) $(FARCH) -g -o $@ $(filter %.c,$^)
COMPILE_static=$(CC) $(CFLAGS) $(FOPT) $(FARCH) -static -o $@ $(filter %.c,$^)
ifeq ($(WINDOWS), 1)
STRIP=strip $@.exe
else
STRIP=strip $@
endif

.PHONY: all debug static clean

all: $(PROGRAMS)

debug: $(addsuffix -debug,$(PROGRAMS))

static: $(addsuffix -static,$(PROGRAMS))

iteress: %: %.c
	$(COMPILE)
	$(STRIP)

iteress-debug: %-debug: %.c
	$(COMPILE_debug)

iteress-static: %-static: %.c
	$(subst $@,$@-$(PLATFORM)-$(ARCH),$(COMPILE_static))
	$(subst $@,$@-$(PLATFORM)-$(ARCH),$(STRIP))

clean:
	rm -f $(PROGRAMS) $(addsuffix -debug,$(PROGRAMS)) $(addsuffix -static-$(PLATFORM)-$(ARCH),$(PROGRAMS)) $(addsuffix .exe,$(PROGRAMS) $(addsuffix -debug,$(PROGRAMS)) $(addsuffix -static-$(PLATFORM)-$(ARCH),$(PROGRAMS)))
