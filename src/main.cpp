namespace Config
{
	namespace General
	{
		static REX::INI::U32 iScrapHeapMult{ "General"sv, "iScrapHeapMult"sv, 2u };
	}

	static void Load()
	{
		const auto ini = REX::INI::SettingStore::GetSingleton();
		ini->Init("Data/F4SE/plugins/BakaScrapHeap.ini",
		          "Data/F4SE/plugins/BakaScrapHeapCustom.ini");
		ini->Load();
	}
}

namespace Hooks
{
	class hkMaxMemory :
		public REX::Singleton<hkMaxMemory>
	{
	private:
		static std::uint32_t QMaxMemory()
		{
			return MaxMemory;
		}

		inline static REL::Hook _QMaxMemory0{ REL::ID(2228905), 0x056, QMaxMemory };
		inline static REL::Hook _QMaxMemory1{ REL::ID(2228905), 0x0A2, QMaxMemory };
		inline static REL::Hook _QMaxMemory2{ REL::ID(2267850), 0x057, QMaxMemory };
		inline static REL::Hook _QMaxMemory3{ REL::ID(2275922), 0x1AA, QMaxMemory };
		inline static REL::Hook _QMaxMemory4{ REL::ID(2275922), 0x1EC, QMaxMemory };
		inline static REL::Hook _QMaxMemory5{ REL::ID(2275943), 0x1AA, QMaxMemory };
		inline static REL::Hook _QMaxMemory6{ REL::ID(2275943), 0x1EC, QMaxMemory };

	public:
		inline static std::uint32_t MaxMemory{ 0x04000000u };
	};

	static void Install()
	{
		Config::Load();

		switch (Config::General::iScrapHeapMult)
		{
		case 1:
			REX::INFO("ScrapHeap set to 0x04000000 (~070mb)"sv);
			break;
		case 2:
			REX::INFO("ScrapHeap set to 0x08000000 (~130mb)"sv);
			hkMaxMemory::MaxMemory = 0x08000000u;
			break;
		case 3:
			REX::INFO("ScrapHeap set to 0x0C000000 (~200mb)"sv);
			hkMaxMemory::MaxMemory = 0x0C000000u;
			break;
		case 4:
			REX::INFO("ScrapHeap set to 0x0FF00000 (~270mb)"sv);
			hkMaxMemory::MaxMemory = 0x0FF00000u;
			break;
		default:
			REX::WARN("iScrapHeapMult invalid: value must be between 1 and 4."sv);
			break;
		}
	}
}

namespace
{
	void MessageCallback(F4SE::MessagingInterface::Message* a_msg)
	{
		switch (a_msg->type)
		{
		case F4SE::MessagingInterface::kPostLoad:
			Hooks::Install();
			break;
		default:
			break;
		}
	}
}

F4SE_PLUGIN_LOAD(const F4SE::LoadInterface* a_f4se)
{
	F4SE::Init(a_f4se, { .trampoline = true, .trampolineSize = 14 });
	F4SE::GetMessagingInterface()->RegisterListener(MessageCallback);
	return true;
}
