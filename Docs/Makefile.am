
EXTRA_DIST = \
    docs.vcproj \
    Doxyfile \
    Makefile.vc

DOXYGEN_INPUT := $(wildcard pages/*.dox)

all: docs-html

clean: docs-clean

.PHONY: docs-html docs-clean

docs-html: .time-stamp

docs-clean:
	rm -rf html
	rm -f .time-stamp

.time-stamp: Doxyfile $(DOXYGEN_INPUT)
	$(DOXYGEN) Doxyfile
	mkdir -p html/images
	cp -p images/* html/images
	touch .time-stamp

