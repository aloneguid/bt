#pragma once
#include "app/url_pipeline.h"
#include "app/config.h"
#include "app/script_site.h"
#include "../common/ext/alg_tracker.h"

#define ProtoName "BrowserTamerHTM"

#define PdfProtoName "BrowserTamerPDF"

#define SSHProtoName "BrowserTamerSSH"

#define CustomProtoName "x-bt"

#define ContributeUrl "https://github.com/aloneguid/bt#contributing"

#define ArgSplitter "|"

#define PortableMarkerName ".portable"

#define LuaRulePrefix "rule_"

#define LuaPipelinePrefix "ppl_"

extern alg::tracker t;

extern bt::config g_config;

extern bt::url_pipeline g_pipeline;

extern bt::script_site g_script;