#pragma once

namespace Settings
{
	namespace
	{
		using bSetting = AutoTOML::bSetting;
		using iSetting = AutoTOML::iSetting;
		using ISetting = AutoTOML::ISetting;
	}

	namespace General
	{
		inline bSetting EnableDebugLogging{ "General"s, "EnableDebugLogging"s, false };
		inline iSetting ScrapHeapMult{ "General"s, "ScrapHeapMult"s, 1 };
	}

	inline void Load()
	{
		try
		{
			const auto table = toml::parse_file(
				fmt::format(FMT_STRING("Data/F4SE/Plugins/{:s}.toml"sv), Version::PROJECT));
			for (const auto& setting : ISetting::get_settings())
			{
				setting->load(table);
			}
		}
		catch (const toml::parse_error& e)
		{
			std::ostringstream ss;
			ss
				<< "Error parsing file \'" << *e.source().path << "\':\n"
				<< '\t' << e.description() << '\n'
				<< "\t\t(" << e.source().begin << ')';
			logger::error(FMT_STRING("{:s}"sv), ss.str());
			stl::report_and_fail("Failed to load settings."sv);
		}
		catch (const std::exception& e)
		{
			stl::report_and_fail(e.what());
		}
		catch (...)
		{
			stl::report_and_fail("Unknown failure."sv);
		}
	}
}
