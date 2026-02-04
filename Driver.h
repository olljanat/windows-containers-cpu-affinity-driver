#pragma once
#include <fltKernel.h>

// Set explicit prototypes for non-documented functions
NTKERNELAPI PUCHAR NTAPI PsGetProcessImageFileName(
    _In_ PEPROCESS Process
);

NTSYSAPI NTSTATUS NTAPI ZwSetInformationProcess(
    __in       HANDLE ProcessHandle,
    __in       PROCESSINFOCLASS ProcessInformationClass,
    __in       PVOID ProcessInformation,
    __in       ULONG ProcessInformationLength
);
