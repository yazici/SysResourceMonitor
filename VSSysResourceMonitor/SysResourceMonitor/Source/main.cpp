#include <windows.h>
#include <QtWidgets/QApplication>
#include <QMessageBox>
#include <QDir>
#include "SRMError.h"
#include "SRMScopeOnExit.h"
#include "SysResourceMonitor.h"

// 判断程序之间的互斥, 只能打开一个软件
bool checkUnique()
{
	std::wstring sTitle = L"提示";
	std::wstring sMessage = L"SysResourceMonitor 已经在运行中";
	std::wstring sMutex = L"VS_SysResourceMonitor";
	HANDLE hMutex = CreateMutex(NULL, TRUE, sMutex.c_str());
	if (!hMutex){
		MessageBox(NULL, sMessage.c_str(), sTitle.c_str(), S_OK);
		return false;
	}
		
	if (GetLastError() == ERROR_ALREADY_EXISTS){
		MessageBox(NULL, sMessage.c_str(), sTitle.c_str(), S_OK);
		CloseHandle(hMutex);
		return false;
	};

	return true;
}

bool initCOM()
{
	if (S_OK != CoInitializeEx(nullptr, COINIT_MULTITHREADED))
		return false;	

	if (S_OK != CoInitializeSecurity(nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_NONE, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE, 0))
		return false;

	return true;
}

void unInitCOM()
{
	CoUninitialize();
}

int main(int argc, char* argv[])
{
	// 1. com组件统一初始化，其他地方不要再初始化
	if (!initCOM())
		return -1;
	SCOPE_EXIT{ unInitCOM(); };

	// 2. 不支持多进程
	if (!checkUnique()){
		return 0;
	}

	// 3. 主程序
	QApplication a(argc, argv);
	QDir::setCurrent(qApp->applicationDirPath());
	try{
		SysResourceMonitor w;
		w.setWindowIcon(QIcon(":/Images/SysResourceMonitor.png"));
		w.hide(); // 主窗体不显示
		w.doWork();
		return a.exec();
	}
	catch (SRMError & e){
		QMessageBox::critical(nullptr, QString::fromStdWString(L"运行错误"), QString::fromStdWString(L"错误码:%1").arg(e.errorCode()), QMessageBox::Ok);
		return -1;
	}
}