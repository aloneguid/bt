#pragma once
#include <string>
#include "app/url_pipeline.h"
#include "app/config.h"
#include "app/script_site.h"
#include "../common/ext/alg_tracker.h"

const std::string ProtoName = "BrowserTamerHTM";

const std::string PdfProtoName = "BrowserTamerPDF";

const std::string CustomProtoName = "x-bt";

const std::string ContributeUrl = "https://github.com/aloneguid/bt#contributing";

const std::string ArgSplitter = "|";

constexpr std::string_view PortableMarkerName = ".portable";

constexpr std::string_view LuaRulePrefix = "rule_";

constexpr std::string_view LuaPipelinePrefix = "ppl_";

extern alg::tracker t;

extern bt::config g_config;

extern bt::url_pipeline g_pipeline;

extern bt::script_site g_script;