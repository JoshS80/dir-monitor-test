#include <DirMonitorPseudoPlatform.h>

namespace fs_directory
{
	namespace monitor
	{
		namespace platform
		{
			void CDirectoryMonitor::StartMonitor()
			{
				m_monitorThread = std::jthread(&CDirectoryMonitor::MonitorChanges, this, m_monitorStopSource.get_token());
			}

			void CDirectoryMonitor::StopMonitor()
			{
				m_monitorStopSource.request_stop();

				if(m_monitorThread.joinable())
					m_monitorThread.join();
			}

			void CDirectoryMonitor::MonitorChanges(std::stop_token stopToken)
			{
				for(auto eventIter = m_addEvents.begin(); eventIter != m_addEvents.end(); ++eventIter)
				{
					{
						std::lock_guard<std::mutex> lock(m_eventMutex);
						printf("%S\n", std::format(L"Pushing {}", eventIter->first.wstring()).c_str());
						PushDirChangeEvent(eventIter->first, eventIter->second);

						m_eventCond.notify_one();
					}

					std::this_thread::sleep_for(std::chrono::seconds(1));
				}

				// no more "events"...
				while (1)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(250));

					if (stopToken.stop_requested())
						break;
				}

				printf("Monitor exiting...\n");
			}
		}
	}
}