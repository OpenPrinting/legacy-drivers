# Makefile to download Ghostscript 9.22 and build the cp50 driver for verification.

GS_VERSION = 9.22
GS_TARBALL = ghostscript-$(GS_VERSION).tar.gz
GS_URL = https://github.com/ArtifexSoftware/ghostpdl-downloads/releases/download/gs922/$(GS_TARBALL)
GS_DIR = ghostscript-$(GS_VERSION)

.PHONY: all build clean test

all: build

$(GS_TARBALL):
	@echo "Downloading Ghostscript $(GS_VERSION)..."
	wget -c $(GS_URL)

$(GS_DIR): $(GS_TARBALL)
	@echo "Extracting Ghostscript $(GS_VERSION)..."
	tar -xzf $(GS_TARBALL)

build: $(GS_DIR)
	@echo "Building Ghostscript $(GS_VERSION) with cp50 driver..."
	cd $(GS_DIR) && ./configure && $(MAKE)

test: build
	@echo "Testing cp50 driver..."
	./$(GS_DIR)/bin/gs -dBATCH -dNOPAUSE -dSAFER -sDEVICE=cp50 -sOutputFile=test_out.cp50 ./$(GS_DIR)/examples/tiger.eps
	@echo "Test completed successfully: test_out.cp50 generated."

clean:
	rm -rf $(GS_DIR) test_out.cp50

distclean: clean
	rm -f $(GS_TARBALL)
