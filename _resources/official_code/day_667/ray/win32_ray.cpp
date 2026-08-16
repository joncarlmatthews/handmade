#include <windows.h>

internal u64
LockedAddAndReturnPreviousValue(u64 volatile *Value, u64 Addend)
{
    u64 Result = InterlockedExchangeAdd64((volatile LONG64 *)Value, Addend);
    return(Result);
}

internal DWORD WINAPI 
WorkerThread(void *lpParameter)
{
    work_queue *Queue = (work_queue *)lpParameter;
    while(RenderTile(Queue)) {};
    return(0);
}

internal u32
GetCPUCoreCount(void)
{
    SYSTEM_INFO Info;
    GetSystemInfo(&Info);
    u32 Result = Info.dwNumberOfProcessors;
    return(Result);
}

internal void
CreateWorkThread(void *Parameter)
{
    DWORD ThreadID;
    HANDLE ThreadHandle = CreateThread(0, 0, WorkerThread, Parameter, 0, &ThreadID);
    CloseHandle(ThreadHandle);
}
