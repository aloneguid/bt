#pragma once
#include <string>
#include "app/url_pipeline.h"
#include "app/config.h"
#include "../common/ext/alg_tracker.h"

const std::string AppDescription("Redirects open URLs to a browser of your choice.");

const std::string ProtoName = "BrowserTamerHTM";

const std::string PdfProtoName = "BrowserTamerPDF";

const std::string CustomProtoName = "x-bt";

const std::string ContributeUrl = "https://github.com/aloneguid/bt#contributing";

const std::string ArgSplitter = "|";

const std::string PortableMarkerName = ".portable";

extern alg::tracker t;

extern bt::config g_config;

extern bt::url_pipeline g_pipeline;