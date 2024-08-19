#pragma once

#ifdef MSW
#include <DirMonitorPseudoPlatform.h>
#else
#include <DirMonitorOtherPlatform.h>
#endif

namespace fs_directory
{
	namespace monitor
	{
		std::unique_ptr<CDirectoryMonitorBase> GetDirectoryMonitor(const std::filesystem::path& monitorPath)
		{
			return std::make_unique<platform::CDirectoryMonitor>(monitorPath);
		}
	}
}