#pragma once
#include "app/url_pipeline.h"
#include "app/script_site.h"
#include "state.h"
#include "common/config.hpp"

#define ProtoNameDev "BrowserTamerHTMDev"
#define ProtoNameRelease "BrowserTamerHTM"

#if _DEBUG
#define ProtoName ProtoNameDev
#else
#define ProtoName ProtoNameRelease
#endif

//#define PdfProtoName "BrowserTamerPDF"

#define CustomProtoName "x-bt"

#define ArgSplitter "|"

#define PortableMarkerName ".portable"

#define APP_SHORT_NAME "bt"

#if _DEBUG
#define APP_LONG_NAME "Browser Tamer Dev"
#else
#define APP_LONG_NAME "Browser Tamer"
#endif

#define CONFIG_NAME "Browser Tamer"

#define APP_URL "https://www.aloneguid.uk/projects/bt/"

#define APP_TEST_URL "https://www.aloneguid.uk/other/bt-test-page/"

#define APP_GITHUB_URL "https://github.com/aloneguid/bt"

#define APP_GITHUB_RELEASES_URL "https://github.com/aloneguid/bt/releases"

#define APP_DOCS_URL "https://www.aloneguid.uk/projects/bt/"

#define APP_HELP_BASE_URL "https://www.aloneguid.uk/projects/bt/"

#define APP_BUYMEACOFFEE_URL "https://www.buymeacoffee.com/alonecoffee"

#define APP_REG_DESCRIPTION "Redirects open URLs to a browser of your choice."

#define APP_VERSION "6.2.3"

extern bt::url_pipeline g_pipeline;

extern bt::script_site g_script;

extern bt::state g_state;

extern grey::common::config<bt::state> g_config;
