#pragma once
#pragma warning(disable : 4702)

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "F4SE/F4SE.h"
#include "RE/Fallout.h"

#include <AutoTOML.hpp>
#include <spdlog/sinks/basic_file_sink.h>
#include <xbyak/xbyak.h>

#define DLLEXPORT extern "C" [[maybe_unused]] __declspec(dllexport)

using namespace std::literals;

namespace logger = F4SE::log;

namespace stl
{
	using namespace F4SE::stl;

	void asm_replace(std::uintptr_t a_from, std::size_t a_size, std::uintptr_t a_to);
}

// clang-format off
#include "Version.h"
#include "Settings.h"
// clang-format on

DLLEXPORT constinit auto F4SEPlugin_Version = []() noexcept {
	F4SE::PluginVersionData data{};

	data.PluginVersion(Version::MAJOR);
	data.PluginName(Version::PROJECT);
	// data.AuthorName();
	data.UsesAddressLibrary(true);
	data.UsesSigScanning(false);
	data.IsLayoutDependent(true);
	data.HasNoStructUse(false);
	data.CompatibleVersions({ F4SE::RUNTIME_LATEST });

	return data;
}();
