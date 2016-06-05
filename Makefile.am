
SUBDIRS = rp rpxl xlsdk Examples Docs

EXTRA_DIST = \
    Announce.txt \
    Authors.txt \
    autogen.sh \
    Contributors.txt \
    LICENSE.TXT \
    NEWS.txt \
    reposit_vc9.sln \
    rplib_vc9.vcproj \
    README.txt \
    TODO.txt

.PHONY: docs docs-clean
docs:
	$(MAKE) -C Docs docs-html
docs-clean:
	$(MAKE) -C Docs docs-clean

