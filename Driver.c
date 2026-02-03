#include "Driver.h"

// Helper to get restricted affinity mask (exclude first two cores)
KAFFINITY GetRestrictedAffinityMask() {
    // For prototype, assume single processor group and <64 cores
    // Exclude logical processors 0 and 1
    KAFFINITY FullMask = (KAFFINITY)((1ULL << KeQueryActiveProcessorCount(NULL)) - 1);
    KAFFINITY RestrictedMask = FullMask & ~0x3ULL;
    return RestrictedMask;
}

VOID ProcessNotifyCallback(
    HANDLE  ParentId,
    HANDLE  ProcessId,
    BOOLEAN Create)
{
    UNREFERENCED_PARAMETER(ParentId);
    if (!Create)
    {
        return;
    }

    PEPROCESS Process = NULL;
    NTSTATUS status = PsLookupProcessByProcessId(ProcessId, &Process);
    if (!NT_SUCCESS(status))
    {
        return;
    }

    BOOLEAN IsTarget = FALSE;
    PUCHAR ImageFileName = PsGetProcessImageFileName(Process);
    if (ImageFileName)
    {
        if (_stricmp((char*)ImageFileName, "CExecSvc.exe") == 0)
        {
            IsTarget = TRUE;
        }
    }

    if (IsTarget)
    {
        HANDLE hProcess = NULL;
        CLIENT_ID Cid = { ProcessId, NULL };
        OBJECT_ATTRIBUTES ObjAttr;
        InitializeObjectAttributes(&ObjAttr, NULL, OBJ_KERNEL_HANDLE, NULL, NULL);

        status = ZwOpenProcess(&hProcess,
            0x0200,
            &ObjAttr,
            &Cid);
        if (!NT_SUCCESS(status))
        {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "!!!! ContainerCpuAffinity: ZwOpenProcess of CExecSvc.exe failed: 0x%08X\n", status));
            return;
        }

        KAFFINITY RestrictedMask = GetRestrictedAffinityMask();
        status = ZwSetInformationProcess(hProcess,
            ProcessAffinityMask,
            &RestrictedMask,
            sizeof(KAFFINITY));
        ZwClose(hProcess);
        if (!NT_SUCCESS(status))
        {
            KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "!!!! ContainerCpuAffinity: Failed to set CPU affinity for CExecSvc.exe: 0x%08X\n", status));
            return;
        }
    }
    ObDereferenceObject(Process);
}

VOID DriverUnload(PDRIVER_OBJECT DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);
    PsSetCreateProcessNotifyRoutine(ProcessNotifyCallback, TRUE);
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath)
{
    UNREFERENCED_PARAMETER(RegistryPath);
    DriverObject->DriverUnload = DriverUnload;
    NTSTATUS status = PsSetCreateProcessNotifyRoutine(ProcessNotifyCallback, FALSE);
    if (!NT_SUCCESS(status))
    {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, "!!!! ContainerCpuAffinity: Registering PsSetCreateProcessNotifyRoutine failed: 0x%08X\n", status));
        return status;
    }
    return STATUS_SUCCESS;
}
