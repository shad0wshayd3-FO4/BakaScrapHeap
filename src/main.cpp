namespace stl
{
	namespace detail
	{
		struct asm_patch :
			Xbyak::CodeGenerator
		{
			asm_patch(std::uintptr_t a_dst)
			{
				Xbyak::Label dst;

				jmp(ptr[rip + dst]);

				L(dst);
				dq(a_dst);
			}
		};

		static void asm_jump(std::uintptr_t a_from, [[maybe_unused]] std::size_t a_size, std::uintptr_t a_to)
		{
			asm_patch p{ a_to };
			p.ready();
			assert(p.getSize() <= a_size);
			REL::safe_write(
				a_from,
				std::span{ p.getCode<const std::byte*>(), p.getSize() });
		}
	}

	static void asm_replace(std::uintptr_t a_from, std::size_t a_size, std::uintptr_t a_to)
	{
		REL::safe_fill(a_from, REL::INT3, a_size);
		detail::asm_jump(a_from, a_size, a_to);
	}
}

namespace Config
{
	namespace General
	{
		static REX::INI::U32 iScrapHeapMult{ "General"sv, "iScrapHeapMult"sv, 2 };
	}

	static void Load()
	{
		const auto ini = REX::INI::SettingStore::GetSingleton();
		ini->Init("Data/F4SE/plugins/BakaScrapHeap.ini",
		          "Data/F4SE/plugins/BakaScrapHeapCustom.ini");
		ini->Load();
	}
}

class ScrapHeap
{
public:
	static void Install()
	{
		static REL::Relocation<std::uintptr_t> target{ REL::ID(2228361) };
		stl::asm_replace(target.address(), 0x10, reinterpret_cast<std::uintptr_t>(QMaxMemory));

		auto iScrapHeapMult = Config::General::iScrapHeapMult.GetValue();
		switch (iScrapHeapMult)
		{
		case 1:
			F4SE::log::info("ScrapHeap default [0x04000000 (~70mb)]"sv);
			break;
		case 2:
			F4SE::log::info("ScrapHeap patched [0x08000000 (~130mb)]"sv);
			MaxMemory = 0x08000000;
			break;
		case 3:
			F4SE::log::info("ScrapHeap patched [0x0C000000 (~200mb)]"sv);
			MaxMemory = 0x0C000000;
			break;
		case 4:
			F4SE::log::info("ScrapHeap patched [0x0FF00000 (~270mb)]"sv);
			MaxMemory = 0x0FF00000;
			break;
		default:
			F4SE::log::info("ScrapHeapMult invalid: value must be between 1 and 4, is {:d}."sv, iScrapHeapMult);
			break;
		}
	}

private:
	static std::uint32_t QMaxMemory()
	{
		return MaxMemory;
	}

	inline static std::uint32_t MaxMemory{ 0x04000000 };
};

namespace
{
	void MessageCallback(F4SE::MessagingInterface::Message* a_msg) noexcept
	{
		switch (a_msg->type)
		{
		case F4SE::MessagingInterface::kPostLoad:
		{
			Config::Load();
			ScrapHeap::Install();
			break;
		}
		default:
			break;
		}
	}
}

F4SEPluginLoad(const F4SE::LoadInterface* a_F4SE)
{
	F4SE::Init(a_F4SE);
	F4SE::GetMessagingInterface()->RegisterListener(MessageCallback);

	return true;
}
