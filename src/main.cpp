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
	// Create log
	auto logPath = logger::log_directory();
	if (!logPath)
	{
		return false;
	}

	*logPath /= fmt::format(FMT_STRING("{}.log"), Version::PROJECT);
	auto logSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logPath->string(), true);
	auto log = std::make_shared<spdlog::logger>("plugin_log"s, std::move(logSink));

	// Set default log traits
	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%m/%d/%Y - %T] [%^%l%$] %v"s);
	spdlog::flush_on(spdlog::level::debug);

	// Set log level from ini
	Settings::Load();
#ifdef NDEBUG
	if (*Settings::EnableDebugLogging)
#else
	if (true)
#endif
	{
		spdlog::set_level(spdlog::level::debug);
	}
	else
	{
		spdlog::set_level(spdlog::level::info);
	}

	// Initial messages
	logger::info(FMT_STRING("{} v{} log opened."), Version::PROJECT, Version::NAME);
	logger::debug("Debug logging enabled."sv);

	// Initialize PluginInfo
	a_info->infoVersion = F4SE::PluginInfo::kVersion;
	a_info->name = Version::PROJECT.data();
	a_info->version = Version::MAJOR;

	// Check if we're being loaded in the CK.
	if (a_F4SE->IsEditor())
	{
		logger::critical("Loaded in editor, marking as incompatible."sv);
		return false;
	}

	// Check if we're being loaded by a supported version of the game.
	const auto ver = a_F4SE->RuntimeVersion();
	if (ver < F4SE::RUNTIME_1_10_163)
	{
		logger::critical(FMT_STRING("Unsupported runtime v{}, marking as incompatible."), ver.string());
		return false;
	}

	return true;
}

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Load(const F4SE::LoadInterface* a_F4SE)
{
	// Initialize F4SE
	F4SE::Init(a_F4SE);

	// Patch ScrapHeap::QMaxMemory
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
