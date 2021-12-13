namespace
{
	void InitializeLog()
	{
#ifndef NDEBUG
		auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
		auto path = logger::log_directory();
		if (!path)
		{
			stl::report_and_fail("Failed to find standard logging directory"sv);
		}

		*path /= fmt::format(FMT_STRING("{}.log"), Plugin::NAME);
		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

#ifndef NDEBUG
		const auto level = spdlog::level::trace;
#else
		const auto level =
			*Settings::EnableDebugLogging ? spdlog::level::trace : spdlog::level::info;
#endif

		auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
		log->set_level(level);
		log->flush_on(level);

		spdlog::set_default_logger(std::move(log));
		spdlog::set_pattern("[%m/%d/%Y - %T] [%^%l%$] %v"s);
	}
}

namespace ScrapHeap
{
	std::uint32_t MaxMemory{ 0x04000000 };
	std::uint32_t QMaxMemory()
	{
		return MaxMemory;
	}
}

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Query(const F4SE::QueryInterface* a_F4SE, F4SE::PluginInfo* a_info)
{
	a_info->infoVersion = F4SE::PluginInfo::kVersion;
	a_info->name = Plugin::NAME.data();
	a_info->version = Plugin::VERSION[0];

	const auto rtv = a_F4SE->RuntimeVersion();
	if (rtv < F4SE::RUNTIME_LATEST)
	{
		stl::report_and_fail(
			fmt::format(FMT_STRING("{} does not support runtime v{}."sv), Plugin::NAME, rtv.string()));
	}

	return true;
}

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Load(const F4SE::LoadInterface* a_F4SE)
{
	Settings::Load();

	InitializeLog();
	logger::info(FMT_STRING("{} v{} log opened."sv), Plugin::NAME, Plugin::VERSION.string());
	logger::debug("Debug logging enabled."sv);

	F4SE::Init(a_F4SE);

	REL::Relocation<std::uintptr_t> target{ REL::ID(126418) };
	stl::asm_replace(target.address(), 0x10, reinterpret_cast<std::uintptr_t>(ScrapHeap::QMaxMemory));

	// Apply Config
	switch (*Settings::ScrapHeapMult)
	{
		case 1:
			logger::info("ScrapHeap default [0x04000000 (~70mb)]"sv);
			break;
		case 2:
			logger::info("ScrapHeap patched [0x08000000 (~130mb)]"sv);
			ScrapHeap::MaxMemory = 0x08000000;
			break;
		case 3:
			logger::info("ScrapHeap patched [0x0C000000 (~200mb)]"sv);
			ScrapHeap::MaxMemory = 0x0C000000;
			break;
		case 4:
			logger::info("ScrapHeap patched [0x0FF00000 (~270mb)]"sv);
			ScrapHeap::MaxMemory = 0x0FF00000;
			break;
		default:
			logger::warn(FMT_STRING("ScrapHeapMult invalid: value must be between 1 and 4, is {:d}."), *Settings::ScrapHeapMult);
			break;
	}

	// Finish load
	logger::info("Plugin loaded successfully."sv);
	return true;
}
