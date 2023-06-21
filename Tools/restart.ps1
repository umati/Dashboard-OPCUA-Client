# MIT License
#
# Copyright (c) 2023 Tiberiu Dobai, Carl Zeiss Industrielle Messtechnik GmbH
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.


$ProcessName = "DashboardOpcUaClient.exe"

# Add own path to the executable
$ProcessPath = "C:\work\umati\UmatiDashboardOpcUaClient-Release-windows-2022-amd64\bin"

Write-Output " ++ Start the umati gateway..."
Get-Date -UFormat "%A %d.%m.%Y %R %Z"

$toolpath="$($ProcessPath)"
if (-not (Test-Path $toolpath))
{
	Write-Output "*** Error: No path >$toolpath< found!!"
	Write-Output "*** Test aborted..."
	Get-Date -Format "dddd dd.MM.yyyy HH:mm:ss K"
	sleep 3
	exit 1
}

Set-Location $toolpath
Get-Location
Write-Output ""
Write-Output " == Start the test loop..."
$i = 0

while($true) {

	$i++
	Write-Output " == start gateway count = $i ..."
	Get-Date -Format "dddd dd.MM.yyyy HH:mm:ss"

	# start the execution of the inspection
	# Write-Output ">>$toolpath\$ProcessName<<"
	& $toolpath\$ProcessName

	#if ($i -ge 2) {break}

	Get-Date -Format "dddd dd.MM.yyyy HH:mm:ss"
	Write-Output "*** execution $i for the gateway ended !"
}

Write-Output " -- Test aborted..."
sleep 3

exit 0
