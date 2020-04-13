param($noexit)

#restart PowerShell with -noexit, the same script, and 1
if (!$noexit) {
    powershell -noexit -file $MyInvocation.MyCommand.Path 1
    return
}

cls

#Stop the script on cmdlet error
Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"
$PSDefaultParameterValues['*:ErrorAction']='Stop'

#Retrieve Visual Studio installation path with Microsoft's VSSetup Powershell module
Install-Module VSSetup -Scope CurrentUser -Force
$vs_path = (Get-VSSetupInstance).InstallationPath

$workspace_path_default = "C:\ggpoplusr_workspace"
$current_script_filepath = Split-Path $MyInvocation.MyCommand.Path -Parent

#Sets the Developer Command Prompt for Visual Studio environment variables. This enables the use of nmake in Powershell.
#NOTE: This will work for VS 2017 and later. In earlier versions the bat name and location can differ.
pushd "$vs_path\Common7\Tools"
cmd /c "VsDevCmd.bat & set" |
foreach {
  if ($_ -match "=") {
    $v = $_.split("="); set-item -force -path "ENV:\$($v[0])"  -value "$($v[1])"
  }
}
popd
Write-Host "`nVisual Studio Command Prompt variables set." -ForegroundColor Yellow

if (!($workspace_path = Read-Host "`nPlease specify where you want to create your GGPOPLUSR workspace and press ENTER [Leave empty for default: $workspace_path_default]")) { $workspace_path = $workspace_path_default }

if (!($ggpoplusr_filepath = Read-Host "`nPlease specify where your GGPOPLUSR source code directory is located. It will be copied to the newly created workspace. [Leave empty for default, which is the folder containing this script: $current_script_filepath]")) { $ggpoplusr_filepath = $current_script_filepath }

Write-Host "`nCreate directory $workspace_path" -ForegroundColor Yellow
mkdir $workspace_path

Write-Host "`nMove to $workspace_path" -ForegroundColor Yellow
cd $workspace_path

Write-Host "`nClone GGPO" -ForegroundColor Yellow
git clone https://github.com/pond3r/ggpo $workspace_path\ggpo
if (-not $?) {throw "Failed to clone GGPO"}

Write-Host "`nClone Detours" -ForegroundColor Yellow
git clone https://github.com/microsoft/Detours $workspace_path\Detours
if (-not $?) {throw "Failed to clone Detours"}

Write-Host "`nClone ValveFileVDF" -ForegroundColor Yellow
git clone https://github.com/TinyTinni/ValveFileVDF $workspace_path\ValveFileVDF
if (-not $?) {throw "Failed to clone ValveFileVDF"}

Write-Host "`nClone Dear ImGui" -ForegroundColor Yellow
git clone https://github.com/ocornut/imgui $workspace_path\imgui
if (-not $?) {throw "Failed to clone Dear ImGui"}

Write-Host "`nClone Clone Json for modern C++" -ForegroundColor Yellow
git clone https://github.com/nlohmann/json $workspace_path\json
if (-not $?) {throw "Failed to clone Json for modern C++"}

Write-Host "`nConfigure GGPO" -ForegroundColor Yellow
cd $workspace_path\ggpo\build
cmake .. -A Win32
if (-not $?) {throw "Failed to configure GGPO"}

Write-Host "`nBuild GGPO" -ForegroundColor Yellow
cd "$vs_path\MSBuild\Current\Bin"
.\msbuild.exe $workspace_path\ggpo\build\src\GGPO.vcxproj /property:Configuration=Debug
if (-not $?) {throw "Failed to build GGPO with Debug configuration"}
.\msbuild.exe $workspace_path\ggpo\build\src\GGPO.vcxproj /property:Configuration=Release
if (-not $?) {throw "Failed to build GGPO with Release configuration"}

Write-Host "`nBuild Detours" -ForegroundColor Yellow
cd $workspace_path\Detours
nmake
if (-not $?) {throw "Failed to build Detours"}

Write-Host "`nCopying $ggpoplusr_filepath to $workspace_path\GGPOPLUSR" -ForegroundColor Yellow
Copy-Item -Path $ggpoplusr_filepath -Recurse -Destination $workspace_path\GGPOPLUSR

Write-Host "`nCreate directory and move into directory $workspace_path\GGPOPLUSR\build" -ForegroundColor Yellow
mkdir "$workspace_path\GGPOPLUSR\build"
cd "$workspace_path\GGPOPLUSR\build"

Write-Host "`nDownload vcredist.x86.exe from Microsoft" -ForegroundColor Yellow
Invoke-WebRequest -Uri https://aka.ms/vs/16/release/vc_redist.x86.exe -OutFile vc_redist.x86.exe

Write-Host "`nConfigure GGPOPLUSR" -ForegroundColor Yellow
cmake .. -A Win32 -DVC_REDIST_X86_EXE="vc_redist.x86.exe"
if (-not $?) {throw "Failed to configure GGPOPLUSR"}

Write-Host "`nGGPOPLUSR workspace creation and configuration should be successful at this point." -ForegroundColor Green
Write-Host "`nYou can delete $ggpoplusr_filepath" -ForegroundColor Green
Write-Host "`nThe GGPOPLUSR solution file location should be $workspace_path\GGPOPLUSR\build\GGPOPLUSR.sln" -ForegroundColor Green
