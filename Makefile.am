SUBDIRS = po lib third_party src doc test

ACLOCAL_AMFLAGS = -I m4 --install
RST2HTML = @RST2HTML@

EXTRA_DIST = config.rpath \
	     tools/script-helper \
	     tools/import-po \
	     tools/build_test.sh \
	     packaging/scripts/makerelease \
	     packaging/scripts/mingw-config packaging/scripts/mingw-release packaging/scripts/mingw-build-memo \
	     packaging/scripts/android-config packaging/scripts/android-release \
	     packaging/macos/makerelease-osx.mk \
	     packaging/macos/DS_Store packaging/macos/etc/paths.d/aria2c packaging/macos/etc/manpaths.d/aria2 \
	     packaging/docker/Dockerfile.mingw \
	     packaging/docker/Dockerfile.android \
	     packaging/legacy/Dockerfile.raspberrypi \
	     packaging/legacy/travis.yml \
	     packaging/android/generic-config packaging/android/openssl-config packaging/android/zlib-config \
	     doc/requirements.txt \
	     examples/libaria2ex.cc examples/libaria2wx.cc

dist_doc_DATA = README README.rst README.html

.PHONY: clang-format

if HAVE_RST2HTML
README.html: README.rst
	$(RST2HTML)  $< > $@
else # !HAVE_RST2HTML
README.html: README.rst
	@echo Warning: rst2html not installed. Placing only a dummy file for $@
	echo "<!doctype html>" > $@
	echo "<p>Readme has not been built!.<br>Instead see <a href='README.rst'>README.rst</a></p>" >> $@
endif # !HAVE_RST2HTML

dist_noinst_DATA = LICENSE.OpenSSL

# Format source files using clang-format.  Don't format source files
# under third_party directory since we are not responsible for their coding
# style.
clang-format:
	CLANGFORMAT=`git config --get clangformat.binary`; \
	test -z $${CLANGFORMAT} && CLANGFORMAT="clang-format"; \
	$${CLANGFORMAT} -i $(top_srcdir)/src/*.{c,cc,h} $(top_srcdir)/src/includes/aria2/*.h \
	$(top_srcdir)/examples/*.cc $(top_srcdir)/test/*.{cc,h}
