using System;
using System.Runtime.InteropServices;

namespace IoTCoreDefaultApp.Utils
{
    internal static class NativeTimeMethods
    {
        [DllImport("api-ms-win-core-sysinfo-l1-2-1.dll")]
        internal static extern void GetLocalTime(out SYSTEMTIME lpLocalTime);
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct SYSTEMTIME
    {
        [MarshalAs(UnmanagedType.U2)]
        internal short Year;


        [MarshalAs(UnmanagedType.U2)]
        internal short Month;


        [MarshalAs(UnmanagedType.U2)]
        internal short DayOfWeek;


        [MarshalAs(UnmanagedType.U2)]
        internal short Day;


        [MarshalAs(UnmanagedType.U2)]
        internal short Hour;


        [MarshalAs(UnmanagedType.U2)]
        internal short Minute;


        [MarshalAs(UnmanagedType.U2)]
        internal short Second;


        [MarshalAs(UnmanagedType.U2)]
        internal short Milliseconds;

        internal DateTime ToDateTime() {
            return new DateTime(Year, Month, Day, Hour, Minute, Second);
        }
    }


}
