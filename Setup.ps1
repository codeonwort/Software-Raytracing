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
if (Test-Path "$external_dir/FreeImage") {
	Write-Host "FreeImage already unzipped. skip unzip."
} else {
	Write-Host "Unzip FreeImage.zip"
	Expand-Archive -Path $freeimage_path -DestinationPath $external_dir
	Write-Host "Unzip done"
}

