option(BUILD_SHARED_LIBS        "Build shared libraries"		ON)

option(MEMUTILZ_BUILD_APPS      "Build the main executable"     ON)
option(MEMUTILZ_BUILD_PLUGINS   "Build available plugins"		ON)
option(MEMUTILZ_BUILD_DOCS		"Build doxygen documentation"	ON)

option(MEMUTILZ_WARNINGS_AS_ERRORS  "Treat compiler warnings as errors" OFF)

option(MEMUTILZ_ENABLE_ASAN     "Enable Address Sanitizer"            OFF)
option(MEMUTILZ_ENABLE_UBSAN    "Enable Undefined Behavior Sanitizer" OFF)
option(MEMUTILZ_ENABLE_COVERAGE "Enable code coverage"                OFF)
option(MEMUTILZ_ENABLE_IPO		"Enable Interprocedural Optimization (LTO) when supported" ON)

option(MEMUTILZ_USE_LOCAL_DEPS  "Use local copies of third-party dependencies" ON)