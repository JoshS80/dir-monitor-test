#include <format>
#include <DirMonitor.h>
#include <conio.h>

class CTestFileAddListener final : public fs_directory::monitor::CDirMonitorListener
{
public:
	fs_directory::monitor::DirMonitorEventType GetEventFilter() const override
	{
		return fs_directory::monitor::DirMonitorEventType::FileAdd;
	}

	void Notify(const fs_directory::monitor::DirChange& change)
	{
		_putws(std::format(L"TestListener notify: {}", change._changePath.wstring()).c_str());

		_changes.push_back(change);
	}

	std::vector<fs_directory::monitor::DirChange> _changes;
};

class CTestFileDeleteListener final : public fs_directory::monitor::CDirMonitorListener
{
public:
	fs_directory::monitor::DirMonitorEventType GetEventFilter() const override
	{
		return fs_directory::monitor::DirMonitorEventType::FileDelete;
	}

	void Notify(const fs_directory::monitor::DirChange& change)
	{
		_putws(std::format(L"TestFileDeleteListener notify: {}", change._changePath.wstring()).c_str());

		_changes.push_back(change);
	}

	std::vector<fs_directory::monitor::DirChange> _changes;
};

int main()
{
	std::shared_ptr<CTestFileAddListener> listener1 = std::make_shared<CTestFileAddListener>(CTestFileAddListener());
	std::shared_ptr<CTestFileDeleteListener> listener2 = std::make_shared<CTestFileDeleteListener>(CTestFileDeleteListener());

	{
		auto monitor = fs_directory::monitor::GetDirectoryMonitor(R"(C:\test\dir\path)");

		monitor->AddListener(listener1);
		monitor->AddListener(listener2);

		monitor->Start();

		while (1)
		{
			auto c = _getch();
			if (c == 27)
				break;
		}
	}

	_ASSERTE(listener1->_changes.size() == 3);
	_ASSERTE(listener2->_changes.size() == 3);
}

