extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Query(const F4SE::QueryInterface* a_F4SE, F4SE::PluginInfo* a_info)
{
	// Create log
	auto logPath = logger::log_directory();
	if (!logPath)
	{
		return false;
	}

	*logPath /= "BakaScrapHeap.log"sv;
	auto logSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(logPath->string(), true);
	auto log = std::make_shared<spdlog::logger>("plugin_log"s, std::move(logSink));

	// Set default log traits
	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%m/%d/%Y - %T] [%^%l%$] %v"s);
	spdlog::flush_on(spdlog::level::debug);

	// Set log level from ini
	Settings::Load();
	if (*Settings::EnableDebugLogging)
	{
		spdlog::set_level(spdlog::level::debug);
	}
	else
	{
		spdlog::set_level(spdlog::level::info);
	}

	// Initial messages
	logger::info("{} log opened."sv, "BakaScrapHeap"sv);
	logger::debug("Debug logging enabled."sv);

	// Initialize PluginInfo
	a_info->infoVersion = F4SE::PluginInfo::kVersion;
	a_info->name = "BakaScrapHeap";
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
		logger::critical("Unsupported runtime v{}, marking as incompatible."sv, ver.string());
		return false;
	}

	return true;
}

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Load(const F4SE::LoadInterface* a_F4SE)
{
	// Initialize F4SE
	F4SE::Init(a_F4SE);

	// Patch ScrapHeap
	static auto GetScrapHeapMaxSize = REL::ID(126418);
	switch (*Settings::ScrapHeapMult)
	{
	case 1:
		logger::info("Leaving ScrapHeap unpatched at 0x04000000 (~70mb)."sv);
		break;
	case 2:
		REL::SafeWriteT(GetScrapHeapMaxSize.address() + 4, static_cast<std::uint8_t>(0x80));
		logger::info("Patching ScrapHeap to 0x08000000 (~130mb)."sv);
		break;
	case 3:
		REL::SafeWriteT(GetScrapHeapMaxSize.address() + 4, static_cast<std::uint8_t>(0xC0));
		logger::info("Patching ScrapHeap to 0x0C000000 (~200mb)."sv);
		break;
	case 4:
		REL::SafeWriteT(GetScrapHeapMaxSize.address() + 4, static_cast<std::uint8_t>(0xFF));
		logger::info("Patching ScrapHeap to 0x0FF00000 (~270mb)."sv);
		break;
	default:
		logger::warn("iScrapHeapMult invalid: value must be between 1 and 4, is {}."sv, *Settings::ScrapHeapMult);
		return false;
	}

	// Finish load
	logger::info("Plugin loaded successfully."sv);
	return true;
}