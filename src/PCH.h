#pragma once

#include "F4SE/F4SE.h"
#include "RE/Fallout.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <INIReader.h>


#define DLLEXPORT __declspec(dllexport)

namespace logger = F4SE::log;
using namespace std::literals;


// Config
namespace Plugin
{
	static constexpr auto NAME = "BakaScrapHeap";
	static constexpr auto LOG = "BakaScrapHeap.log";
	static constexpr auto CONFIG = "Data/F4SE/plugins/BakaScrapHeap.ini";
	static constexpr auto VERSION = 1;
}
