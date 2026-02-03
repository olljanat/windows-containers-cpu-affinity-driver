sc stop ContainerCpuAffinity
sc delete ContainerCpuAffinity
takeown /f C:\Windows\System32\drivers\ContainerCpuAffinity.sys
icacls C:\Windows\System32\drivers\ContainerCpuAffinity.sys /grant Administrators:F
del /Q /F C:\Windows\System32\drivers\ContainerCpuAffinity.sys
