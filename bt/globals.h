#pragma once
#include <string>
#include <memory>
#include "app/browser.h"
#include "app/url_pipeline.h"
#include "app/config.h"

const std::string Win32ClassName("BTWindow");
const std::string AppGuid("026741D2-FF77-462B-AD70-4140697C8AE1");

const std::string AppDescription("Redirects open URLs to a browser of your choice.");

const std::string ProtoName = "BrowserTamerHTM";

const std::string PdfProtoName = "BrowserTamerPDF";

const std::string CustomProtoName = "x-bt";

const std::string ContributeUrl = "https://github.com/aloneguid/bt#contributing";

const std::string ArgSplitter = "|";

const std::string PortableMarkerName = ".portable";

//extern alg::tracker t;

extern bt::config g_config; // todo: pass by reference, do not make it global

extern bt::url_pipeline g_pipeline;