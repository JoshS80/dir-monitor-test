#pragma once
#include <DirMonitorBase.h>
#include <memory>

namespace fs_directory
{
	namespace monitor
	{
		namespace platform
		{
			class CDirectoryMonitor final : public CDirectoryMonitorBase
			{
			public:
				CDirectoryMonitor(const std::filesystem::path& monitorPath)
					: CDirectoryMonitorBase(monitorPath)
				{
					m_addEvents = {
						{ "file.txt", fs_directory::monitor::DirMonitorEventType::FileAdd },
						{ "file.txt", fs_directory::monitor::DirMonitorEventType::FileDelete },
						{ "NewDir", fs_directory::monitor::DirMonitorEventType::DirDelete }
					};
				}

				~CDirectoryMonitor()
				{
					StopMonitor();
				}

			protected:
				void StartMonitor() override;
				void StopMonitor();

			private:
				void MonitorChanges(std::stop_token stopToken);
				std::vector<std::pair<std::filesystem::path, DirMonitorEventType>> m_addEvents;

				std::jthread m_monitorThread;
				std::stop_source m_monitorStopSource;
			};
		}
	}
}