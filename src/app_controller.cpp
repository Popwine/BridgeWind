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
        // ִ�м򵥵�����
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

        // ׼�� STARTUPINFO �ṹ��
        STARTUPINFO si;
        ZeroMemory(&si, sizeof(si)); // �Ƚ��ṹ������
        si.cb = sizeof(si);          // ���ýṹ���С

        // ׼�� PROCESS_INFORMATION �ṹ��
        PROCESS_INFORMATION pi;
        ZeroMemory(&pi, sizeof(pi)); // ����

        std::cout << "Calling external command \"" << cmd << "\" " << "using CreateProcess" << std::endl;

        // ��������
        // ����˵����
        // 1. lpApplicationName: ͨ��Ϊ NULL����ʱ�������������������ַ����Ŀ�ͷ
        // 2. lpCommandLine: Ҫִ�е������У������ǿ�д�ڴ�
        // 3. lpProcessAttributes: ���̰�ȫ���ԣ�NULL
        // 4. lpThreadAttributes: �̰߳�ȫ���ԣ�NULL
        // 5. bInheritHandles: �Ƿ�̳и����̵ľ�������ڼ򵥵�����Ϊ FALSE
        // 6. dwCreationFlags: ������־��0 ��ʾĬ�ϡ������� CREATE_NO_WINDOW �����ش���
        // 7. lpEnvironment: ����������NULL ��ʾʹ�ø����̵Ļ�������
        // 8. lpCurrentDirectory: ��ǰĿ¼��NULL ��ʾʹ�ø����̵ĵ�ǰĿ¼
        // 9. lpStartupInfo: ָ�� STARTUPINFO �ṹ���ָ��
        // 10. lpProcessInformation: ָ�� PROCESS_INFORMATION �ṹ���ָ�룬�����½�����Ϣ
        if (!CreateProcess(NULL,   // ʹ�� cmd �����еĳ�����
            cmd,    // �������ַ���
            NULL,   // Process handle not inheritable
            NULL,   // Thread handle not inheritable
            FALSE,  // Set handle inheritance to FALSE
            0,      // No creation flags
            NULL,   // Use parent's environment block
            NULL,   // Use parent's starting directory 
            &si,    // Pointer to STARTUPINFO structure
            &pi)    // Pointer to PROCESS_INFORMATION structure
            ) {
            // �������ʧ�ܣ���ӡ������Ϣ
            throw std::runtime_error("CreateProcess failed (" + std::to_string(GetLastError()) + ").");
            return;
        }

        // �ȴ��ӽ���ִ�н���
        WaitForSingleObject(pi.hProcess, INFINITE);

        std::cout << "External command executed." << std::endl;

        // ��ȡ�ӽ��̵��˳���
        DWORD exitCode;
        if (GetExitCodeProcess(pi.hProcess, &exitCode)) {
            std::cout << "Child process exit code: " << exitCode << std::endl;
        }

        // �رս��̺��߳̾������ֹ��Դй¶
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);

    }
	GmshController::GmshController(const std::string& geoPath, const std::string& meshPath)
		: geoFilePath(geoPath), meshFilePath(meshPath) {
	}
	// ���ü����ļ�·��
	void GmshController::setGeoFilePath(const std::string& path) {
		geoFilePath = path;
	}
	// ��ȡ�����ļ�·��
	std::string GmshController::getGeoFilePath() const {
		return geoFilePath;
	}
	// ���������ļ�·��
	void GmshController::GmshController::setMeshFilePath(const std::string& path) {
		meshFilePath = path;
	}
	// ��ȡ�����ļ�·��
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