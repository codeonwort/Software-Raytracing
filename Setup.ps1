Param (
	[Switch]$skipdownload,
	[Switch]$skipbuild
)

$should_download = !($PSBoundParameters.ContainsKey('skipdownload'))
$should_build = !($PSBoundParameters.ContainsKey('skipbuild'))


#
# Constants
#
$external_dir       = "$pwd/external"

$freeimage_url      = "http://downloads.sourceforge.net/freeimage/FreeImage3180.zip"
$freeimage_path     = "$external_dir/FreeImage.zip"
$tinyobjloader_url  = "https://github.com/syoyo/tinyobjloader/archive/v2.0.0-rc1.zip"
$tinyobjloader_path = "$external_dir/tinyobjloader-v2.0.0-rc1.zip"


#
# Find MSBuild.exe
# https://stackoverflow.com/questions/328017/path-to-msbuild
$vswhere_path = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$msbuild_path = &$vswhere_path -latest -prerelease -products * -requires Microsoft.Component.MSBuild -find MSBuild\**\Bin\MSBuild.exe
#$msbuild_path = "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe"


#
# Utilities
#
function Download-URL {
	Param ($webclient, $target_url, $target_path)
	if (Test-Path $target_path -PathType Leaf) {
		Write-Host $target_path, "already exists and download will be skipped." -ForegroundColor Green
	} else {
		Write-Host "url: ", $target_url -ForegroundColor Green
		Write-Host "Downloading..." -ForegroundColor Green
		$webclient.DownloadFile($target_url, $target_path)
		Write-Host "Download finished."
	}
}
function Clear-Directory {
	Param ($dir)
	Write-Host "Clear directory:", $dir -ForegroundColor Green
	if (Test-Path $dir) {
		Remove-Item -Recurse -Force $dir
	}
	New-Item -ItemType Directory -Force -Path $dir
}


#
# Download third party libraries
#
if ($should_download) {
	Write-Host "Download libraries..." -ForegroundColor Green
	$webclient = New-Object System.Net.WebClient
	Download-URL $webclient $freeimage_url $freeimage_path
	Download-URL $webclient $tinyobjloader_url $tinyobjloader_path
	# TODO: How to close connection
	#$webclient.Close()
}


#
# Build third party libraries
#
if ($should_build) {
	### FreeImage
	# 1. Unzip
	if (Test-Path "$external_dir/FreeImage") {
		Write-Host "FreeImage already unzipped. skip unzip."
	} else {
		Write-Host "Unzip FreeImage"
		Expand-Archive -Path $freeimage_path -DestinationPath $external_dir
		Write-Host "Unzip done"
	}
	# 2. Build
	Write-Host "Build FreeImage (x64 | release)"
	& $msbuild_path ./external/FreeImage/FreeImage.2017.sln -t:FreeImage -p:Configuration=Release -p:Platform=x64
	# 3. Copy
	$freeimage_source_dir = "./thirdparty/FreeImage/source"
	$freeimage_binary_dir = "./thirdparty/FreeImage/binaries"
	Clear-Directory $freeimage_source_dir
	Clear-Directory $freeimage_binary_dir
	Write-Host "Copy source to $freeimage_source_dir"
	Write-Host "Copy binaries to $freeimage_binary_dir"
	Copy-Item "./external/FreeImage/Dist/x64/FreeImage.h" -Destination $freeimage_source_dir
	Copy-Item "./external/FreeImage/Dist/x64/FreeImage.dll" -Destination $freeimage_binary_dir
	Copy-Item "./external/FreeImage/Dist/x64/FreeImage.lib" -Destination $freeimage_binary_dir
	
	### tinyobjloader
	# 1. Unzip
	if (Test-Path "$external_dir/tinyobjloader-2.0.0-rc1") {
		Write-Host "tinyobjloader already unzipped. skip unzip."
	} else {
		Write-Host "Unzip tinyobjloader"
		Expand-Archive -Path $tinyobjloader_path -DestinationPath $external_dir
		Write-Host "Unzip done"
	}
	# 2. Copy
	$tinyobjloader_source_dir = "./thirdparty/tinyobjloader/source"
	Clear-Directory $tinyobjloader_source_dir
	Write-Host "Copy source to $tinyobjloader_source_dir"
	Copy-Item "./external/tinyobjloader-2.0.0-rc1/tiny_obj_loader.h" -Destination $tinyobjloader_source_dir
	Copy-Item "./external/tinyobjloader-2.0.0-rc1/tiny_obj_loader.cc" -Destination $tinyobjloader_source_dir
}
