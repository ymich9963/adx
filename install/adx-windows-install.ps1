$ADX_install_path = 'C:\Program Files\adx'
$ADX_exe_link = "https://github.com/ymich9963/adx/releases/download/v0.1.0/adx.exe"

if (Test-Path -Path $ADX_install_path) {
    Write-Output "Removing previously installed executable."
    Remove-Item $ADX_install_path -r # rm command
}

mkdir $ADX_install_path 
curl -fsSLO $ADX_exe_link
Move-Item adx.exe $ADX_install_path # mv command
Write-Output "Downloaded executable." # echo command

$Sys_Env_Path_Value = Get-ItemProperty -Path 'HKLM:\SYSTEM\CurrentControlSet\Control\Session Manager\Environment\' -Name Path 

# Change the backslashes to frontslashes so that -split can work
$ADX_install_path_frontslash = $ADX_install_path -replace "\\","/"
$Sys_Env_Path_Value_frontslash = $Sys_Env_Path_Value.Path -replace "\\", "/"

# Check if the install path exists by splitting the Path variable value
$ADX_path_check = $Sys_Env_Path_Value_frontslash -split $ADX_install_path_frontslash | Measure-Object 

if ($ADX_path_check.Count -igt 1) {
    Write-Output "Detected previous ADX installation."
    Write-Output "Nothing was added to the system Path variable."
} else {
    Write-Output "Detected no previous ADX install."
    Write-Output "Adding executable to system Path environment variable."
    $New_Path_Value = $Sys_Env_Path_Value.Path + ";" + $ADX_install_path + ";" 
    Set-ItemProperty -Path 'HKLM:\SYSTEM\CurrentControlSet\Control\Session Manager\Environment\' -Name Path -Value $New_Path_Value # set the system environment variable for ADX 
}

Write-Output "Succesfully installed ADX."

Read-Host -Prompt "Press Enter to exit"

Exit

