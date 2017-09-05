MNAME=astparser

if COND_MINGWCC
QPP=qpp_native
else
QPP=qpp
endif

dummy:
	echo "Build started!"

QC_AstParser.cpp: QC_AstParser.qpp
	$(QPP) -V $<

QC_AstTree.cpp: QC_AstTree.qpp
	$(QPP) -V $<

QC_AstTreeSearcher.cpp: QC_AstTreeSearcher.qpp
	$(QPP) -V $<

GENERATED_SOURCES = QC_AstParser.cpp QC_AstTree.cpp QC_AstTreeSearcher.cpp
CLEANFILES = $(GENERATED_SOURCES)
ast_parser.hpp: ast_parser.cpp
ast_parser.cpp: ast_parser.ypp
	bison --defines=ast_parser.hpp --output=$@ $<
ast_scanner.cpp: ast_scanner.lpp
	flex -o $@ $<

if COND_SINGLE_COMPILATION_UNIT
ASTPARSER_SOURCES = astparser_scu.cpp
astparser_scu.cpp: $(GENERATED_SOURCES) ast_parser.hpp ast_parser.cpp ast_scanner.cpp
else
ASTPARSER_SOURCES = astparser-module.cpp AstParser.cpp AstParserHolder.cpp \
	AstPrinter.cpp AstTreeHolder.cpp AstTreePrinter.cpp AstTreeSearcher.cpp \
	queries/FindMatchingSymbolsQuery.cpp queries/FindNodeQuery.cpp \
	queries/FindNodeAndParentsQuery.cpp queries/FindReferencesQuery.cpp \
	queries/FindSymbolInfoQuery.cpp queries/FindSymbolsQuery.cpp \
	queries/GetNodesInfoQuery.cpp queries/SymbolInfoFixes.cpp \
	ast_parser.cpp ast_scanner.cpp
nodist_astparser_la_SOURCES = $(GENERATED_SOURCES)
endif

lib_LTLIBRARIES = astparser.la
astparser_la_SOURCES = $(ASTPARSER_SOURCES)
astparser_la_LDFLAGS = -module -avoid-version ${ASTPARSER_LIBS} ${MODULE_LDFLAGS}

ASTPARSER_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/modules/astparser/src

AM_CPPFLAGS=$(QORE_CPPFLAGS) $(ASTPARSER_CPPFLAGS)

if COND_PROFILE
AM_CXXFLAGS=-g -pg
else
if COND_DEBUG
AM_CXXFLAGS=-g
endif
endif

if COND_MINGWCC
RM_OTHER=$(DESTDIR)$(libdir)/$(MNAME).dll.a
endif

install-exec-hook:
	test -d $(MODULE_DIR) || mkdir -p $(MODULE_DIR)
	mv $(DESTDIR)$(libdir)/$(MNAME).$(MODULE_SUFFIX) $(MODULE_DIR)/$(MNAME)-api-$(MODULE_API).qmod
	$(RM) $(DESTDIR)$(libdir)/$(MNAME).la $(DESTDIR)$(libdir)/$(MNAME).a $(RM_OTHER)

clean-local:
	$(RM) -r $(GENERATED_SOURCES)
	$(RM) ast_parser.hpp ast_parser.cpp ast_scanner.cpp
	$(RM) *.qtest

uninstall-local:
	$(RM) $(MODULE_DIR)/$(MNAME)-api-$(MODULE_API).qmod