#pragma once

static bool check_directory(const char* path)
{
	struct stat info;
	return stat(path, &info) == 0 && (info.st_mode & S_IFDIR);
}

static bool create_directory(const char* path)
{
	if (!path || *path == '\0') return false;

	std::string dir(path);
	if (dir.back() == '/' || dir.back() == '\\') {
		dir.pop_back();
	}

	if (MKDIR(dir.c_str()) == 0) return true; // Try to create directory

	// Create parent directory first
	size_t pos = dir.find_last_of("/\\");
	if (pos != std::string::npos) {
		std::string parent = dir.substr(0, pos);
		if (!create_directory(parent.c_str())) {
			return false;
		}
	}

	return MKDIR(dir.c_str()) == 0; // Try again after creating parent
}

static size_t get_memory_usage_mb() {
#if defined(_WIN32)
	PROCESS_MEMORY_COUNTERS memInfo;
	if (GetProcessMemoryInfo(GetCurrentProcess(), &memInfo, sizeof(memInfo))) {
		return memInfo.WorkingSetSize / (1024 * 1024); // MB
	}
#elif defined(__APPLE__)
	mach_task_basic_info info;
	mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;

	if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO, (task_info_t)&info, &infoCount) == KERN_SUCCESS) {
		return info.resident_size / (1024 * 1024); // MB
	}
#elif defined(__linux__)
	std::ifstream statm("/proc/self/status");
	std::string line;
	while (std::getline(statm, line)) {
		if (line.find("VmRSS:") == 0) {
			size_t kb = 0;
			sscanf(line.c_str(), "VmRSS: %zu kB", &kb);
			return kb / 1024; // MB
		}
	}
#else
#error "Unsupported platform"
#endif
	return 0;
}

static std::string get_executable_path() {
#if defined(_WIN32)
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	return std::filesystem::path(buffer).parent_path().string();

#elif defined(__APPLE__)
	char buffer[4096];
	uint32_t size = sizeof(buffer);
	if (_NSGetExecutablePath(buffer, &size) == 0) {
		return std::filesystem::weakly_canonical(buffer).parent_path().string();
	}
	else {
		printf("Error: _NSGetExecutablePath(): Buffer too small for executable path");
		exit(EXIT_FAILURE);
	}

#elif defined(__linux__)
	char buffer[4096];
	ssize_t count = readlink("/proc/self/exe", buffer, sizeof(buffer));
	if (count != -1) {
		return std::filesystem::weakly_canonical(std::string(buffer, count)).parent_path().string();
	}
	else {
		printf("Error: readlink(): Cannot read /proc/self/exe");
		exit(EXIT_FAILURE);
	}

#else
#error "Unsupported platform"
#endif
	return "";
}