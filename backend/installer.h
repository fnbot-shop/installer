#pragma once
#include <filesystem>
#include <thread>
#include <string>
#include <functional>

namespace fs = std::filesystem;

namespace installer {
	enum class STATE {
		INIT_PINNING,
		INIT_STREAMS,
		INIT_DOWNLOAD,
		DOWNLOAD,
		REGISTRY,
		SHORTCUT,
		DONE
	};

	struct INSTALL_INFO {
		fs::path install_location;
		STATE state;
		char state_info[100];
		double progress;
		void* lParam;
		void(*callback)(const INSTALL_INFO* info);
		void(*error_callback)(void* lParam, const char* error);
	};

	class InstallManager {
	public:
        InstallManager(INSTALL_INFO* info);

        void Run();

		INSTALL_INFO* info;

		fs::path startExe;

	private:
        void run_thread();

        void update_info(STATE state, double progress, const char* state_info);

		std::thread thread;
	};
}