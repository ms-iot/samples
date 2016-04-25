#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <memory.h>
#include <stdbool.h>

#define TM_BASE_YEAR   1900

static const int ydays[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };

bool is_leap_year(int yy)
{
    int yyyy = yy + TM_BASE_YEAR;

    bool bLeap = false;
    if (yyyy % 100 == 0)
    {
        return yyyy % 400 == 0;
    }
    return yyyy % 4 == 0;
}


char * strptime(const char *buf, const char *fmt, struct tm *tm)
{
    char c;
    int temp = 0;

    //initialize the tm struct values
    memset(tm, 0, sizeof(struct tm));
    tm->tm_mday = 1;

    while (buf && (c = *fmt++))
    {
        temp = 0;
        if (c == '%')
        {
            switch (c = *fmt++)
            {
            case '%':
                if (c != *buf++)
                {
                    return NULL;
                }
                break;

            case 'd':   /* day of the month (1..31)*/
                sscanf(buf, "%2d", &temp);
                if (temp >= 1 && temp <= 31)
                {
                    tm->tm_mday = temp;
                    buf += 2;
                }
                else
                    return NULL;
                break;

            case 'H':   /* hour (0..23) */
                sscanf(buf, "%2d", &temp);
                if (temp >= 0 && temp <= 23)
                {
                    tm->tm_hour = temp;
                    buf += 2;
                }
                else
                    return NULL;
                break;

            case 'M':   /* minute (0..59) */
                sscanf(buf, "%2d", &temp);
                if (temp >= 0 && temp <= 59)
                {
                    tm->tm_min = temp;
                    buf += 2;
                }
                else
                    return NULL;
                break;

            case 'm':   /* month (1..12) */
                sscanf(buf, "%2d", &temp);
                if (temp >= 1 && temp <= 12)
                {
                    tm->tm_mon = temp - 1;
                    buf += 2;
                }
                else
                    return NULL;
                break;

            case 'S':   /* seconds (0..59) */
                sscanf(buf, "%2d", &temp);
                if (temp >= 0 && temp <= 59)
                {
                    tm->tm_sec = temp;
                    buf += 2;
                }
                else
                    return NULL;
                break;

            case 'Y':   /* year */
                sscanf(buf, "%4d", &temp);
                if (temp >= 0 && temp <= 9999)
                {
                    tm->tm_year = temp - TM_BASE_YEAR;
                    buf += 4;
                }
                else
                    return NULL;
                break;

            default:
                return NULL;
            }
        }
        else
        {
            if (c != *buf++)
                return NULL;
        }
    }

    //calculate tm_wday and tm_yday
    tm->tm_yday = ydays[tm->tm_mon] + ((is_leap_year(tm->tm_year) && (tm->tm_mon >= 2)) ? 1 : 0) + tm->tm_mday - 1;

    //1st Jan 1900 was Monday, hence weekday = the number of days from 1/1/1900  modulus 7 + 1
    tm->tm_wday = (365 * tm->tm_year) + (tm->tm_year / 4) + tm->tm_yday + 1;        //1st Jan 1900 was Monday, hence add 1
    if (is_leap_year(tm->tm_year))
        tm->tm_wday--;
    tm->tm_wday %= 7;

    return (char*)buf;
}