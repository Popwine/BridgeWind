#include "app_controller.h"
namespace BridgeWind {
    void AppController::simpleExecute(const std::string & command) {
        
        if (command.empty()) {
            throw std::runtime_error("Command cannot be empty.");
        }
        if (command.length() >= 1024) {
            throw std::runtime_error("Command is too long, must be less than 1024 characters.");
        }
        char cmd[1024];
        std::strcpy(cmd, command.c_str());

        std::cout << "Calling external command \"" << cmd << "\" " << "using system" << std::endl;
        // 执行简单的命令
        system(command.c_str());
    }
    void AppController::simpleExecuteWithCreateProcess(const std::string& command) {
		if (command.empty()) {
			throw std::runtime_error("Command cannot be empty.");
		}
		if (command.length() >= 1024) {
			throw std::runtime_error("Command is too long, must be less than 1024 characters.");
		}
        char cmd[1024];
        std::strcpy(cmd, command.c_str());

        // 准备 STARTUPINFO 结构体
        STARTUPINFO si;
        ZeroMemory(&si, sizeof(si)); // 先将结构体清零
        si.cb = sizeof(si);          // 设置结构体大小

        // 准备 PROCESS_INFORMATION 结构体
        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi)); // 清零

        std::cout << "Calling external command \"" << cmd << "\" " << "using CreateProcess" << std::endl;

        // 创建进程
        // 参数说明：
        // 1. lpApplicationName: 通常为 NULL，此时程序名必须在命令行字符串的开头
        // 2. lpCommandLine: 要执行的命令行，必须是可写内存
        // 3. lpProcessAttributes: 进程安全属性，NULL
        // 4. lpThreadAttributes: 线程安全属性，NULL
        // 5. bInheritHandles: 是否继承父进程的句柄，对于简单调用设为 FALSE
        // 6. dwCreationFlags: 创建标志，0 表示默认。可以用 CREATE_NO_WINDOW 来隐藏窗口
        // 7. lpEnvironment: 环境变量，NULL 表示使用父进程的环境变量
        // 8. lpCurrentDirectory: 当前目录，NULL 表示使用父进程的当前目录
        // 9. lpStartupInfo: 指向 STARTUPINFO 结构体的指针
        // 10. lpProcessInformation: 指向 PROCESS_INFORMATION 结构体的指针，接收新进程信息
        if (!CreateProcess(NULL,   // 使用 cmd 参数中的程序名
            cmd,    // 命令行字符串
            NULL,   // Process handle not inheritable
            NULL,   // Thread handle not inheritable
            FALSE,  // Set handle inheritance to FALSE
            0,      // No creation flags
            NULL,   // Use parent's environment block
            NULL,   // Use parent's starting directory 
            &si,    // Pointer to STARTUPINFO structure
            &pi)    // Pointer to PROCESS_INFORMATION structure
            ) {
            // 如果创建失败，打印错误信息
            throw std::runtime_error("CreateProcess failed (" + std::to_string(GetLastError()) + ").");
            return;
        }

        // 等待子进程执行结束
        WaitForSingleObject(pi.hProcess, INFINITE);

        std::cout << "External command executed." << std::endl;

        // 获取子进程的退出码
        DWORD exitCode;
        if (GetExitCodeProcess(pi.hProcess, &exitCode)) {
            std::cout << "Child process exit code: " << exitCode << std::endl;
        }

        // 关闭进程和线程句柄，防止资源泄露
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

    }
    void AppController::simpleExecuteWithCreateProcess(
        const std::string& command, 
        const std::string& workingDirectory
    ) {
        
        if (command.empty()) {
            throw std::runtime_error("Command cannot be empty.");
        }
        if (command.length() >= 1024) {
            throw std::runtime_error("Command is too long, must be less than 1024 characters.");
        }
        char cmd[1024];
        std::strcpy(cmd, command.c_str());
		const char* workingDir = workingDirectory.empty() ? NULL : workingDirectory.c_str();
        // 准备 STARTUPINFO 结构体
        STARTUPINFO si;
        ZeroMemory(&si, sizeof(si)); // 先将结构体清零
        si.cb = sizeof(si);          // 设置结构体大小

        // 准备 PROCESS_INFORMATION 结构体
        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi)); // 清零

        std::cout << "Calling external command \"" << cmd << "\" " << "using CreateProcess" << std::endl;

        // 创建进程
        // 参数说明：
        // 1. lpApplicationName: 通常为 NULL，此时程序名必须在命令行字符串的开头
        // 2. lpCommandLine: 要执行的命令行，必须是可写内存
        // 3. lpProcessAttributes: 进程安全属性，NULL
        // 4. lpThreadAttributes: 线程安全属性，NULL
        // 5. bInheritHandles: 是否继承父进程的句柄，对于简单调用设为 FALSE
        // 6. dwCreationFlags: 创建标志，0 表示默认。可以用 CREATE_NO_WINDOW 来隐藏窗口
        // 7. lpEnvironment: 环境变量，NULL 表示使用父进程的环境变量
        // 8. lpCurrentDirectory: 当前目录，NULL 表示使用父进程的当前目录
        // 9. lpStartupInfo: 指向 STARTUPINFO 结构体的指针
        // 10. lpProcessInformation: 指向 PROCESS_INFORMATION 结构体的指针，接收新进程信息
        if (!CreateProcess(NULL,   // 使用 cmd 参数中的程序名
            cmd,    // 命令行字符串
            NULL,   // Process handle not inheritable
            NULL,   // Thread handle not inheritable
            FALSE,  // Set handle inheritance to FALSE
            0,      // No creation flags
            NULL,   // Use parent's environment block
            workingDir,   // Use parent's starting directory 
            &si,    // Pointer to STARTUPINFO structure
            &pi)    // Pointer to PROCESS_INFORMATION structure
            ) {
            // 如果创建失败，打印错误信息
            throw std::runtime_error("CreateProcess failed (" + std::to_string(GetLastError()) + ").");
            return;
        }

        // 等待子进程执行结束
        WaitForSingleObject(pi.hProcess, INFINITE);

        std::cout << "External command executed." << std::endl;

        // 获取子进程的退出码
        DWORD exitCode;
        if (GetExitCodeProcess(pi.hProcess, &exitCode)) {
            std::cout << "Child process exit code: " << exitCode << std::endl;
        }

        // 关闭进程和线程句柄，防止资源泄露
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

    }
	GmshController::GmshController(const std::string& geoPath, const std::string& meshPath)
		: geoFilePath(geoPath), meshFilePath(meshPath) {
	}
	// 设置几何文件路径
	void GmshController::setGeoFilePath(const std::string& path) {
		geoFilePath = path;
	}
	// 获取几何文件路径
	std::string GmshController::getGeoFilePath() const {
		return geoFilePath;
	}
	// 设置网格文件路径
	void GmshController::GmshController::setMeshFilePath(const std::string& path) {
		meshFilePath = path;
	}
	// 获取网格文件路径
	std::string GmshController::getMeshFilePath() const {
		return meshFilePath;
	}
	void GmshController::generateMesh() {
        std::string arguments;
		if (formatMsh2) {
			arguments = " -2 -format msh2";
		}
		else {
			arguments = " -2";
		}
		std::string command = 
			gmshExecutablePath + 
			" " + 
			geoFilePath + 
			arguments +
			" -o " + 
			meshFilePath;
		simpleExecuteWithCreateProcess(command);
		std::cout << "Mesh generated successfully at: " << meshFilePath << std::endl;
	}
    void GmshController::openOutputMeshFile() {
		std::string command = gmshExecutablePath + " " + meshFilePath;
		simpleExecuteWithCreateProcess(command);
    }

}