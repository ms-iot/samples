// Copyright (c) Microsoft. All rights reserved.

// ConsoleApplication.cpp : Defines the entry point for the console application.
//

#include "pch.h"

#include <windows.h>
#include <chrono>
#include <thread>

using namespace std;

// Use to convert bytes to KB
#define DIV 1024

// Specify the width of the field in which to print the numbers. 
#define MESSAGE_WIDTH 30
#define NUMERIC_WIDTH 10

void printMessage(LPCSTR msg, bool addColon)
{
    cout.width(MESSAGE_WIDTH);
    cout << msg;
    if (addColon)
    {
        cout << " : ";
    }
}

void printMessageLine(LPCSTR msg)
{
    printMessage(msg, false);
    cout << endl;
}

void printMessageLine(LPCSTR msg, DWORD value)
{
    printMessage(msg, true);
    cout.width(NUMERIC_WIDTH);
    cout << right << value << endl;
}

void printMessageLine(LPCSTR msg, DWORDLONG value)
{
    printMessage(msg, true);
    cout.width(NUMERIC_WIDTH);
    cout << right << value << endl;
}

int main(int argc, char **argv)
{
    printMessageLine("Starting to monitor memory consumption!");
    for (;;)
    {
        MEMORYSTATUSEX statex;
        statex.dwLength = sizeof(statex);

        BOOL success = GlobalMemoryStatusEx(&statex);
        if (!success)
        {
            DWORD error = GetLastError();
            printMessageLine("*************************************************");
            printMessageLine("Error getting memory information", error);
            printMessageLine("*************************************************");
        }
        else
        {
            DWORD load = statex.dwMemoryLoad;
            DWORDLONG physKb = statex.ullTotalPhys / DIV;
            DWORDLONG freePhysKb = statex.ullAvailPhys / DIV;
            DWORDLONG pageKb = statex.ullTotalPageFile / DIV;
            DWORDLONG freePageKb = statex.ullAvailPageFile / DIV;
            DWORDLONG virtualKb = statex.ullTotalVirtual / DIV;
            DWORDLONG freeVirtualKb = statex.ullAvailVirtual / DIV;
            DWORDLONG freeExtKb = statex.ullAvailExtendedVirtual / DIV;

            printMessageLine("*************************************************");

            printMessageLine("Percent of memory in use", load);
            printMessageLine("KB of physical memory", physKb);
            printMessageLine("KB of free physical memory", freePhysKb);
            printMessageLine("KB of paging file", pageKb);
            printMessageLine("KB of free paging file", freePageKb);
            printMessageLine("KB of virtual memory", virtualKb);
            printMessageLine("KB of free virtual memory", freeVirtualKb);
            printMessageLine("KB of free extended memory", freeExtKb);

            printMessageLine("*************************************************");

        }

        this_thread::sleep_for(chrono::milliseconds(100));
    }
    printMessageLine("No longer monitoring memory consumption!");
}
