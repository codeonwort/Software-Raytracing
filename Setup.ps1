#
# TODO: Variables that need auto-detect ###
#
$msbuild_path = "C:/Program Files (x86)/Microsoft Visual Studio/2017/Community/MSBuild/15.0/Bin/MSBuild.exe"


#
# Download third party libraries
#

$external_dir = "$pwd/external"
$webclient = new-object System.Net.WebClient

### FreeImage
Write-Host "Download library: FreeImage" -ForegroundColor Green

$freeimage_url = "http://downloads.sourceforge.net/freeimage/FreeImage3180.zip"
$freeimage_path = "$external_dir/FreeImage.zip"

if (Test-Path $freeimage_path -PathType Leaf) {
	Write-Host "FreeImage.zip already exists. skip download."
} else {
	Write-Host "url: ", $freeimage_url
	Write-Host "Downloading..."
	$webclient.DownloadFile($freeimage_url, $freeimage_path)
	Write-Host "Download finished."
}


#
# Build third party libraries
#

### FreeImage
# 1. Unzip
if (Test-Path "$external_dir/FreeImage") {
	Write-Host "FreeImage already unzipped. skip unzip."
} else {
	Write-Host "Unzip FreeImage.zip"
	Expand-Archive -Path $freeimage_path -DestinationPath $external_dir
	Write-Host "Unzip done"
}
# 2. Build
Write-Host "Build FreeImage (x64 | release)"
& $msbuild_path ./external/FreeImage/FreeImage.2017.sln -t:FreeImage -p:Configuration=Release -p:Platform=x64
# 3. Copy
$freeimage_source_dir = "./thirdparty/FreeImage/source"
$freeimage_binary_dir = "./thirdparty/FreeImage/binaries"
Write-Host "Copy source to $freeimage_source_dir"
Write-Host "Copy binaries to $freeimage_binary_dir"
if (!(Test-Path $freeimage_source_dir)) {
	New-Item -ItemType Directory -Force -Path $freeimage_source_dir
}
if (!(Test-Path $freeimage_binary_dir)) {
	New-Item -ItemType Directory -Force -Path $freeimage_binary_dir
}
Copy-Item "./external/FreeImage/Source" -Destination $freeimage_source_dir -Recurse
Copy-Item "./external/FreeImage/x64/Release/FreeImage.dll" -Destination $freeimage_binary_dir
Copy-Item "./external/FreeImage/x64/Release/FreeImage.pdb" -Destination $freeimage_binary_dir
Copy-Item "./external/FreeImage/x64/Release/FreeImage.lib" -Destination $freeimage_binary_dir
