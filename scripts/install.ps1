param($LLVMDirectory = "", $ToolsetName = "", $ClangClToolsetName = "")
$id=[System.Security.Principal.WindowsIdentity]::GetCurrent()
$principal=new-object System.Security.Principal.WindowsPrincipal($id)

$role=[System.Security.Principal.WindowsBuiltInRole]::Administrator
if (!$principal.IsInRole($role)) {
   $pinfo = New-Object System.Diagnostics.ProcessStartInfo "powershell"
   [string[]]$arguments = @("-ExecutionPolicy","Bypass",$myInvocation.MyCommand.Definition)
   foreach ($key in $myInvocation.BoundParameters.Keys) {
       $arguments += ,@("-$key")
       $arguments += ,@($myInvocation.BoundParameters[$key])
    }
   $pinfo.Arguments = $arguments
   $pinfo.Verb = "runas"
   $null = [System.Diagnostics.Process]::Start($pinfo)
   exit
}
$Host.UI.RawUI.WindowTitle = "Fafnir - Clang MSBuild toolset installer"

function Install-Failed($message) {
    Write-Error $message
    Write-Host -NoNewLine "Press any key to continue..."
    $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
    exit
}

function Get-Registry($key, $valuekey = "") {
    $reg = Get-Item -Path $key -ErrorAction SilentlyContinue
    if ($reg) {
        return $reg.GetValue($valuekey)
    }
    return $null
}

$VSInstallDir = Get-Registry Registry::HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\VisualStudio\SxS\VS7 "15.0"
if (!$VSInstallDir) {
    $VSInstallDir = Get-Registry Registry::HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Microsoft\VisualStudio\SxS\VS7 "15.0"
}
$LLVMDir = Get-Registry Registry::HKEY_LOCAL_MACHINE\SOFTWARE\LLVM\LLVM -ErrorAction
if (!$LLVMDir) {
    $LLVMDir = Get-Registry Registry::HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\LLVM\LLVM
}

if (!$VSInstallDir) {
    Install-Failed "Visual Studio 2017 is not found."
}

$reset = $false

do {
    if ($reset -or $LLVMDirectory -eq "") {
        for(;;) {
            $prompt = "Where is the LLVM Directory?"
            if ($LLVMDirectory -ne "") {
                $prompt += " (current: $LLVMDirectory)"
            } elseif ($LLVMDir -ne "") {
                $prompt += " (default: $LLVMDir)"
            }
            $tmp = Read-Host $prompt
            if ($tmp -eq "") {
                if ($LLVMDirectory -eq "") {
                    $LLVMDirectory = $LLVMDir
                }
            } else {
                $LLVMDirectory = $tmp
            }
            if ($LLVMDirectory -eq "") {
                "LLVM directory must be specified."
                continue
            }
            $clangPath = "$LLVMDirectory\bin\clang.exe"
            if (!(Test-Path $clangPath)) {
                if (!((Read-Host "$clangPath doesn't exist. Would you like to continue installation? (y/N)") -match "y|yes")) {
                    $LLVMDirectory = ""
                    continue
                }
            }
            break
        }
    }

    if ($reset -or $ToolsetName -eq "") {
        $prompt = "What is the clang toolset name?"
        if ($reset) {
            $prompt += "(current: $ToolsetName)"
        } else {
            $prompt += " (default: v100_clang_fafnir)"
        }
        $tmp = Read-Host $prompt
        if ($tmp -eq "" -and $ToolsetName -eq "") {
            $ToolsetName = "v100_clang_fafnir"
        } else {
            $ToolsetName = $tmp
        }
    }

    if ($reset -or ($ClangClToolsetName -eq "" -and (Read-Host "Do you want to install a toolset for clang-cl? (y/N)") -match "y|yes")) {
        $prompt = "What is the clang-cl toolset name?"
        if ($reset) {
            $prompt += "(current: $ClangClToolsetName)"
        } else {
            $prompt += " (default: fafnir_clang_cl)"
        }
        $tmp = Read-Host $prompt
        if ($tmp -eq "" -and $ClangClToolsetName -eq "") {
            $ClangClToolsetName = "fafnir_clang_cl"
        } else {
            $ClangClToolsetName = $tmp
        }
    }

    ""
    "=== Install configuration ==="
    "* LLVM install directory: $LLVMDirectory"
    "* Toolset name: $ToolsetName"
    if ($ClangClToolsetName -eq "") {
        "* Clang-cl toolset won't install."
    } else {
        "* Clang-cl toolset: $ClangClToolsetName"
    }
    ""
    $reset = $true
} while((Read-Host "Is it OK to install? (Y/n)") -match "n|no")

$rootDir = Split-Path -Parent $myInvocation.MyCommand.Definition | Split-Path -Parent
$assets = "$rootDir\assets"
$bin = "$rootDir\bin\clang.exe"
$dll = "$rootDir\bin\fafnir_injection.dll"

function Install ($arch) {
    $platformDir = "$VSInstallDir\Common7\IDE\VC\VCTargets\Platforms\$arch\PlatformToolsets";
    if (!(Test-Path $platformDir)) {
        return
    }
    $targetPath = "$platformDir\$ToolsetName"

    if (!(Test-Path $targetPath)) {
        New-Item -ItemType Directory $targetPath
    }
    Copy-Item "$assets\clang\Toolset.targets" "$targetPath"
    $content = (Get-Content -Encoding UTF8 "$assets\clang\Toolset.props") -replace "{{LLVMDir}}",$LLVMDirectory
    Set-Content "$targetPath\Toolset.props" $content -Encoding UTF8

    $targetBinPath = "$targetPath\bin"
    if (!(Test-Path $targetBinPath)) {
        New-Item -ItemType Directory $targetBinPath
    }
    Copy-Item $bin $targetBinPath
    Copy-Item $dll $targetBinPath
    $mingwDLLs = @("libstdc++-6.dll", "libwinpthread-1.dll", "libgcc_s_seh-1.dll", "libgcc_s_dw2-1.dll")
    foreach ($mingwDLL in $mingwDLLs) {
        $mingwDLLFullPath = "${rootDir}\bin\${mingwDLL}"
        if (Test-Path $mingwDLLFullPath) {
            Copy-Item $mingwDLLFullPath $targetBinPath
        }
    }
    Set-Content -Path "${targetBinPath}\.target" "$LLVMDirectory\bin\clang.exe" -Encoding UTF8 -NoNewline

    if ($ClangClToolsetName -ne "") {
        $targetPath = "$platformDir\$ClangClToolsetName"
        if (!(Test-Path $targetPath)) {
            New-Item -ItemType Directory $targetPath
        }
        Copy-Item "$assets\clang-cl\Toolset.targets" "$targetPath"
        $content = (Get-Content -Encoding UTF8 "$assets\clang-cl\Toolset.props") -replace "{{LLVMDir}}",$LLVMDirectory
        Set-Content "$targetPath\Toolset.props" $content -Encoding UTF8 | Out-Null
    }
}

Install "Win32"
Install "x64"

Write-Host -NoNewLine "Press any key to continue..."
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
""
