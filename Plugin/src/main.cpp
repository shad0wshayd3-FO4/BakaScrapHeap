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

class Config
{
public:
	class General
	{
	public:
		inline static DKUtil::Alias::Integer iScrapHeapMult{ "iScrapHeapMult", "General" };
	};

	static void Load()
	{
		static auto Config = COMPILE_PROXY("BakaScrapHeap.ini");
		Config.Bind(General::iScrapHeapMult, 2);
		Config.Load();
	}
};

class ScrapHeap
{
public:
	static void Install()
	{
		static REL::Relocation<std::uintptr_t> target{ REL::ID(2228361) };
		stl::asm_replace(target.address(), 0x10, reinterpret_cast<std::uintptr_t>(QMaxMemory));

		switch (*Config::General::iScrapHeapMult)
		{
		case 1:
			INFO("ScrapHeap default [0x04000000 (~70mb)]"sv);
			break;
		case 2:
			INFO("ScrapHeap patched [0x08000000 (~130mb)]"sv);
			MaxMemory = 0x08000000;
			break;
		case 3:
			INFO("ScrapHeap patched [0x0C000000 (~200mb)]"sv);
			MaxMemory = 0x0C000000;
			break;
		case 4:
			INFO("ScrapHeap patched [0x0FF00000 (~270mb)]"sv);
			MaxMemory = 0x0FF00000;
			break;
		default:
			INFO("ScrapHeapMult invalid: value must be between 1 and 4, is {:d}."sv, *Config::General::iScrapHeapMult);
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

DLLEXPORT bool F4SEAPI F4SEPlugin_Load(const F4SE::LoadInterface* a_F4SE)
{
#ifndef NDEBUG
	MessageBoxA(NULL, "Loaded. You can now attach the debugger or continue execution.", Plugin::NAME.data(), NULL);
#endif

	F4SE::Init(a_F4SE);

	DKUtil::Logger::Init(Plugin::NAME, std::to_string(Plugin::Version));
	INFO("{} v{} loaded."sv, Plugin::NAME, Plugin::Version);

	Config::Load();
	ScrapHeap::Install();

	return true;
}
