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
   $p = [System.Diagnostics.Process]::Start($pinfo)
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

function Install ($arch) {
    $VCTargets = "$VSInstallDir\Common7\IDE\VC\VCTargets"
    $platformDir = "$VCTargets\Platforms\$arch\PlatformToolsets";
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
    
    if (!(Test-Path "$targetPath\bin")) {
        New-Item -ItemType Directory "$targetPath\bin"
    }
    Copy-Item $bin "$targetPath\bin"
    Set-Content -Path "$targetPath\bin\.target" "$LLVMDirectory\bin\clang.exe" -Encoding UTF8 -NoNewline

    function Replace-Node($node, $name, $display) {
        [System.XML.XMLElement] $tmp = $node.CloneNode($true)
        $tmp.SetAttribute("Name", "$name")
        $tmp.SetAttribute("Switch", "std=$name")
        $tmp.SetAttribute("DisplayName",  (($tmp.GetAttribute("DisplayName")) -replace "C\+\+03","C++$display"))
        $tmp.SetAttribute("Description", (($tmp.GetAttribute("Description")) -replace "C\+\+03","C++$display"))
        return $tmp
    }
    
    function Create-Lang-File ($langID) {
        $xml = [Xml](Get-Content "$VCTargets\$langID\clang.xml" -Encoding UTF8)
        $xmlns = @{ p = "http://schemas.microsoft.com/build/2009/properties" }
        $std = (Select-Xml "p:EnumProperty[@Name=""CppLanguageStandard""]" $xml.Rule -Namespace $xmlns).Node
        $cxx98 = (Select-Xml "p:EnumValue[@Name=""c++98""]" $std -Namespace $xmlns).Node
        $gnuxx98 = (Select-Xml "p:EnumValue[@Name=""gnu++98""]" $std -Namespace $xmlns).Node
    
        $tmp = Replace-Node $cxx98 "c++14" "14"
        $std.InsertBefore($tmp, $gnuxx98) | Out-Null
        $tmp = Replace-Node $cxx98 "c++17" "17"
        $std.InsertBefore($tmp, $gnuxx98) | Out-Null
        $tmp = Replace-Node $cxx98 "c++2a" "20"
        $std.InsertBefore($tmp, $gnuxx98) | Out-Null
        
        $tmp = Replace-Node $gnuxx98 "gnu++14" "14"
        $std.AppendChild($tmp) | Out-Null
        $tmp = Replace-Node $gnuxx98 "gnu++17" "17"
        $std.AppendChild($tmp) | Out-Null
        $tmp = Replace-Node $gnuxx98 "gnu++2a" "20"
        $std.AppendChild($tmp) | Out-Null
    
        $cxx1y = Select-Xml "p:EnumValue[@Name=""c++1y""]" $std -Namespace $xmlns
        $gnuxx1y = Select-Xml "p:EnumValue[@Name=""gnu++1y""]" $std -Namespace $xmlns
        if ($cxx1y) {
            $std.RemoveChild($cxx1y.Node) | Out-Null
        }
        if ($gnuxx1y) {
            $std.RemoveChild($gnuxx1y.Node) | Out-Null
        }
        if (!(Test-Path "$targetPath\$langID")) {
            New-Item "$targetPath\$langID" -ItemType Directory
        }
        $xml.Save("$targetPath\$langID\clang.xml")
    }
    
    foreach($dir in Get-ChildItem $VCTargets -Directory) {
        if(($dir.Name) -match "[0-9]+") {
            Create-Lang-File $dir.Name
        }
    }

    if ($ClangClToolsetName -ne "") {
        $targetPath = "$platformDir\$ClangClToolsetName"
        if (!(Test-Path $targetPath)) {
            New-Item -ItemType Directory $targetPath
        }
        Copy-Item "$assets\clang-cl\Toolset.targets" "$targetPath"
        $tmp = (Get-Content -Encoding UTF8 "$assets\clang-cl\Toolset.props") -replace "{{LLVMDir}}",$LLVMDirectory
        Set-Content "$targetPath\Toolset.props" $tmp -Encoding UTF8 | Out-Null
    }
}

Install "Win32"
Install "x64"

Write-Host -NoNewLine "Press any key to continue..."
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
""