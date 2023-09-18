#pragma once
#include <string>
#include <memory>
#include "ext/lsignal/lsignal.h"
#include "ext/alg_tracker.h"
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

extern alg::tracker t;

extern bt::config g_config;

extern bt::url_pipeline g_pipeline;

/**
 * @brief Simple global app event. Args:
 * 0 - event name
 * 1, 2 - extras depending on event
*/
extern lsignal::signal<void(const std::string&, const std::string&, const std::string&)> app_event;

/**
 * @brief Even on URL being opened after a rule match or explicit pick.
 * Args:
 * 0 - url being opened
*/
extern lsignal::signal<void(const bt::url_payload&, const bt::browser_match_result&)> open_on_match_event;
