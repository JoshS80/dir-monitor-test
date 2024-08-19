#pragma once

#include <filesystem>
#include <type_traits>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace fs_directory
{
	namespace monitor
	{
		enum class DirMonitorEventType
		{
			FileAdd = 1,
			FileDelete = 2,
			DirAdd = 4,
			DirDelete = 8,
		};

		inline bool operator&(DirMonitorEventType a, DirMonitorEventType b)
		{
			return std::underlying_type_t<DirMonitorEventType>(a) & std::underlying_type_t<DirMonitorEventType>(b);
		}

		inline DirMonitorEventType operator|(DirMonitorEventType a, DirMonitorEventType b)
		{
			return static_cast<DirMonitorEventType>(std::underlying_type_t<DirMonitorEventType>(a) | std::underlying_type_t<DirMonitorEventType>(b));
		}

		struct DirChange
		{
			std::filesystem::path _changePath;
			DirMonitorEventType _changeType;
		};

		class CDirMonitorListener
		{
		public:
			virtual ~CDirMonitorListener() {}

			virtual DirMonitorEventType GetEventFilter() const = 0;
			virtual void Notify(const DirChange& change) = 0;
		};

		class CDirectoryMonitorBase
		{
		public:
			CDirectoryMonitorBase(const std::filesystem::path& monitorPath)
				: m_monitorPath(monitorPath)
			{}

			virtual ~CDirectoryMonitorBase()
			{
				Stop();
			}

			void AddListener(std::shared_ptr<CDirMonitorListener> listener)
			{
				m_listeners.push_back(listener);
			}

			void Start()
			{
				StartMonitor();

				m_listenerThread = std::jthread(&CDirectoryMonitorBase::ListenForEvents, this, m_stopSource.get_token());
			}

			void Stop()
			{
				m_stopSource.request_stop();

				if(m_listenerThread.joinable())
					m_listenerThread.join();
			}

		protected:
			void PushDirChangeEvent(const std::filesystem::path& changePath, DirMonitorEventType changeType)
			{
				std::filesystem::path fullPath = m_monitorPath / changePath;
				m_events.push({ fullPath, changeType });
			}

			virtual void StartMonitor() = 0;

			std::mutex m_eventMutex;
			std::condition_variable_any m_eventCond;
			std::stop_source m_stopSource;

		private:
			void ListenForEvents(std::stop_token stopToken)
			{
				while(1)
				{
					printf("Listener alive\n");
					std::unique_lock<std::mutex> lock(m_eventMutex);
					auto waitResult = m_eventCond.wait(lock, stopToken, [this]() {
						return !m_events.empty();
					});

					if (stopToken.stop_requested())
					{
						printf("Listener exiting...\n");
						break;
					}

					while (!m_events.empty())
					{
						DirChange d = m_events.front();
						m_events.pop();

						for (auto& l : m_listeners)
						{
							//if (l->GetEventFilter() & d._changeType)
								l->Notify(d);
						}
					}
				}
			}

			std::vector<std::shared_ptr<CDirMonitorListener>> m_listeners;
			std::queue<DirChange> m_events;
			std::filesystem::path m_monitorPath;
			std::jthread m_listenerThread;
		};
	}
}
