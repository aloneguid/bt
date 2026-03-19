#pragma once
#include "app/url_pipeline.h"
#include "app/config.h"
#include "app/script_site.h"

#define ProtoName "BrowserTamerHTM"

#define PdfProtoName "BrowserTamerPDF"

#define CustomProtoName "x-bt"

#define ArgSplitter "|"

#define PortableMarkerName ".portable"

#define LuaRulePrefix "rule_"

#define LuaPipelinePrefix "ppl_"

#define APP_SHORT_NAME "bt"

#define APP_LONG_NAME "Browser Tamer"

#define APP_URL "https://www.aloneguid.uk/projects/bt/"

#define APP_TEST_URL "https://www.aloneguid.uk/other/bt-test-page/"

#define APP_GITHUB_URL "https://github.com/aloneguid/bt"

#define APP_GITHUB_RELEASES_URL "https://github.com/aloneguid/bt/releases"

#define APP_DOCS_URL "https://www.aloneguid.uk/projects/bt/"

#define APP_HELP_BASE_URL "https://www.aloneguid.uk/projects/bt/"

#define APP_BUYMEACOFFEE_URL "https://www.buymeacoffee.com/alonecoffee"

#define APP_REG_DESCRIPTION "Redirects open URLs to a browser of your choice."

#define APP_VERSION "5.6.0"

extern bt::config g_config;

extern bt::url_pipeline g_pipeline;

extern bt::script_site g_script;
