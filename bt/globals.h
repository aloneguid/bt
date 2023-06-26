#pragma once
#include <string>
#include <vector>
#include <chrono>
#include "app/config.h"
#include "ext/lsignal/lsignal.h"

const std::string Win32ClassName("BTWindow");
const std::string AppGuid("026741D2-FF77-462B-AD70-4140697C8AE1");

const std::string AppDescription("Redirects open URLs to a browser of your choice.");

const std::string ProtoName = "BrowserTamerHTM";

const std::string PdfProtoName = "BrowserTamerPDF";

const std::string CustomProtoName = "x-bt";

const std::string HomeUrl = "https://www.aloneguid.uk/projects/bt/";

const std::string ChromeExtensionUrl = "https://chrome.google.com/webstore/detail/browser-tamer/oggcljknmiiomjekepdoindjcpnpglnd";

const std::string EdgeExtensionUrl = "https://microsoftedge.microsoft.com/addons/detail/browser-tamer/gofjagaghddmjloaecpnldjmjlplicin";

const std::string VersionCheckUrl = "https://www.aloneguid.uk/projects/bt/bin/latest.txt";

const std::string CoffeePageUrl = "https://www.buymeacoffee.com/alonecoffee";

const std::string ArgSplitter = "|";

static std::chrono::steady_clock::time_point last_useful_activity = std::chrono::steady_clock::now();

const size_t AutoshudownIntervalMinutes = 30;

extern lsignal::signal<void(const std::string&, const std::string&, const std::string&)> app_event;
