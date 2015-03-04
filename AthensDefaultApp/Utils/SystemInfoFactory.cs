// © Copyright(C) Microsoft. All rights reserved.

using System;
using System.Runtime.InteropServices;

namespace AthensDefaultApp
{
    [StructLayout(LayoutKind.Sequential)]
    internal struct SYSTEM_INFO
    {
        public ushort wProcessorArchitecture;
        public ushort wReserved;
        public uint dwPageSize;
        public IntPtr lpMinimumApplicationAddress;
        public IntPtr lpMaximumApplicationAddress;
        public UIntPtr dwActiveProcessorMask;
        public uint dwNumberOfProcessors;
        public uint dwProcessorType;
        public uint dwAllocationGranularity;
        public ushort wProcessorLevel;
        public ushort wProcessorRevision;
    }

    internal static class SystemInfoFactory
    {
        internal const ushort PROCESSOR_ARCHITECTURE_INTEL = 0;
        internal const ushort PROCESSOR_ARCHITECTURE_ARM = 5;
        internal const ushort PROCESSOR_ARCHITECTURE_IA64 = 6;
        internal const ushort PROCESSOR_ARCHITECTURE_AMD64 = 9;
        internal const ushort PROCESSOR_ARCHITECTURE_UNKNOWN = 0xFFFF;

        [DllImport("api-ms-win-core-sysinfo-l1-2-1.dll")]
        private static extern void GetNativeSystemInfo(ref SYSTEM_INFO lpSystemInfo);

        internal static SYSTEM_INFO GetNativeSystemInfo()
        {
            SYSTEM_INFO systemInfo = new SYSTEM_INFO();

            GetNativeSystemInfo(ref systemInfo);

            return systemInfo;
        }
    }
}
