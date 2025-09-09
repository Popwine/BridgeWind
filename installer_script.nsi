; ##################################################################
; ## BridgeWind 安装程序脚本
; ##################################################################
; 请求管理员权限。这将导致在运行安装包时，弹出 UAC 提权对话框。
RequestExecutionLevel admin
;Target "amd64-unicode"
; --- 基础信息 ---
Name "BridgeWind"
OutFile "BridgeWind-1.0.2-Setup.exe"  ; 这是最终生成的安装包文件名
InstallDir "$PROGRAMFILES64\BridgeWind" ; 默认安装路径
RequestExecutionLevel admin ; 请求管理员权限

; --- 界面设置 (使用现代界面) ---
!include "MUI2.nsh"

; 定义安装程序的图标
!define MUI_ICON "res\icons\app_icon.ico" ; 确保 C:\Projects\BridgeWind\res\app_icon.ico 文件存在
!define MUI_UNICON "res\icons\app_icon.ico" ; 卸载程序的图标

; --- 定义安装界面上显示的页面 ---
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "readme.md" ; 您可以把 readme.md 换成 LICENSE.txt
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

; --- 定义卸载界面上显示的页面 ---
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

; --- 设置界面语言 ---
!insertmacro MUI_LANGUAGE "SimpChinese"
!insertmacro MUI_LANGUAGE "English"


; ##################################################################
; ## 安装逻辑段
; ##################################################################
Section "Install"
    ; 设置输出路径为用户选择的安装目录
    SetOutPath $INSTDIR
     ; --- 1. 安装 Visual C++ 可再发行组件包 ---
    DetailPrint "Installing Microsoft Visual C++ Redistributable..."
    ; 将 VC_redist.x64.exe 释放到临时目录
    File "VC_redist.x64.exe"
    ; 执行安装，并等待它完成
    ExecWait '"$INSTDIR\VC_redist.x64.exe" /install /quiet /norestart'
    ; 删除临时文件
    Delete "$INSTDIR\VC_redist.x64.exe"
    ; --- 2. 安装 Visual C++ 2013 可再发行组件包 ---
    DetailPrint "Installing Microsoft Visual C++ 2013 Redistributable..."
    ; 假设您已将下载的文件重命名
    File "vcredist_2013_x64.exe" 
    ; 执行静默安装
    ExecWait '"$INSTDIR\vcredist_2013_x64.exe" /install /quiet /norestart'
    ; 删除临时文件
    Delete "$INSTDIR\vcredist_2013_x64.exe"
    ; --- 2. 安装 Microsoft MPI ---
    DetailPrint "Installing Microsoft MPI..."
    File "msmpisetup.exe" 

    ; 定义日志文件的路径
    Var /GLOBAL MpiLogPath
    StrCpy $MpiLogPath "$INSTDIR\msmpi_install_log.txt"

    ; 构造完整的安装命令，使用从帮助文档中找到的原生参数
    ; -unattend 表示无界面安装
    ; -full 确保安装所有组件
    ; -log 和 -verbose 创建详细的安装日志
    ExecWait '"$INSTDIR\msmpisetup.exe" -unattend -full -log "$MpiLogPath" -verbose' $0

    ; --- 关键：检查上一条命令的返回代码 ---
    ; $0 变量保存了 ExecWait 命令的返回代码。
    ; 对于 MS-MPI 的 EXE，0 通常代表成功，其他值代表失败。
    IntCmp $0 0 continue_install

        ; 如果代码不为 0 (安装失败)，则弹窗提示用户
        MessageBox MB_OK|MB_ICONSTOP "Microsoft MPI installation failed (Return code: $0). Please check the log file for details:$\n$\n$MpiLogPath"
        ; Abort "MPI Installation Failed." ; 您甚至可以在这里选择终止整个安装

    continue_install:
        Delete "$INSTDIR\msmpisetup.exe"
    ; ##################################################################

    ; /r 参数表示递归地包含所有子文件夹和文件
    ; 这里指向我们之前创建的 "舞台" 文件夹
    File /r "C:\Projects\BridgeWind\Release_Package\*"

    ; --- 创建快捷方式 ---
    ; 创建开始菜单文件夹
    CreateDirectory "$SMPROGRAMS\BridgeWind"
    ; 创建开始菜单中的程序快捷方式
    CreateShortCut "$SMPROGRAMS\BridgeWind\BridgeWind.lnk" "$INSTDIR\BridgeWindApp.exe"
    ; 创建桌面快捷方式
    CreateShortCut "$DESKTOP\BridgeWind.lnk" "$INSTDIR\BridgeWindApp.exe"

    ; --- 写入卸载信息到注册表 ---
    WriteUninstaller "$INSTDIR\Uninstall.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BridgeWind" "DisplayName" "BridgeWind"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BridgeWind" "UninstallString" '"$INSTDIR\Uninstall.exe"'
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BridgeWind" "DisplayIcon" "$INSTDIR\BridgeWindApp.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BridgeWind" "DisplayVersion" "1.0.0"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BridgeWind" "Publisher" "dlut"
SectionEnd

; ##################################################################
; ## 卸载逻辑段
; ##################################################################
Section "Uninstall"
    ; 删除文件
    Delete "$INSTDIR\*"
    
    ; 删除文件夹 (必须是空的才能删除)
    RMDir /r "$INSTDIR\translations"
    RMDir /r "$INSTDIR\platforms"
    RMDir /r "$INSTDIR\styles"
    ; ... (windeployqt 创建的其他文件夹)
    RMDir /r "$INSTDIR" ; 最后删除根目录
    
    ; 删除快捷方式
    Delete "$SMPROGRAMS\BridgeWind\BridgeWind.lnk"
    Delete "$DESKTOP\BridgeWind.lnk"
    
    ; 删除开始菜单文件夹
    RMDir "$SMPROGRAMS\BridgeWind"

    ; 删除注册表中的卸载信息
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\BridgeWind"
SectionEnd