# Find all subdirectories inside src/, excluding src/bash/
SUBDIRS := $(filter-out src/bash/ src/python src/zig/, $(wildcard src/*/))

# Default target: build all subdirectories
.PHONY: all $(SUBDIRS) clean

all: $(SUBDIRS)

# Rule to dive into each directory and run make
$(SUBDIRS):
	$(MAKE) -C $@

# Propagate 'make clean' to all subdirectories
clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done
