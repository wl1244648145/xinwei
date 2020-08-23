/* RTC_M48t201V.c - RTC M48t201V real time clock driver */

/*
DESCRIPTION
This is the driver for the real time clock portion of the Dallas Semiconductor
DS1643 Nonvolative Timekeeping RAM

USER CALLABLE ROUTINES
Most of the routines in this driver are accessible only through the I/O
system. Two routines, however, must be called directly, rtcDrv to
initialize the driver, and rtcDevCreate to create the device.

INSTALLING THE DRIVER
Before using the driver, it must be initialized by calling the routine:

.CS
rtcDrv ()
.CE

This routine should be called exactly once, before any ioctls or
DevCreates. Normally, it is called from usrRoot.

CREATING YOUR DEVICE
Before the real time clock device can be used, it has to be created.
This is done with the rtcDevCreate call:

.CS
STATUS rtcDevCreate (UINT)
.CE

The default name for this device is "/rtc".

RTC DATE AND TIME
This driver maintains the date and time in an internal structure.
There are two routines provided to set the date and the time:

.CS
rtcDateSet (1992, 1, 13, 1);  set date to Jan-13-1992, Mon
.CS
.CE
rtcTimeSet (14, 30, 22);      set time to 14:30:22
.CE

To set the time, 24 hour notation is required.

To read the date and the time the following two routines
have to be called:

.CS
rtcDateGet ();                print current date
.CS
.Ce
rtcTimeGet ();                print current time
.CE

DOS DATE AND TIME
To support date and time of local DOS files, a date/time hook
routine is provided. After installing the RTC driver and
creating the RTC device with the calls described above, this routine
has to be installed with the call:

.CS
dosFsDateTimeInstall (rtcHook);
.CE
*/

/* includes */

#include "vxWorks.h"
#include "stdio.h"
#include "types.h"
#include "iosLib.h"
#include "errnoLib.h"
#include "dosFsLib.h"
#include "sysBtsConfigData.h"
/* typedefs */

typedef struct              /* RTC_DEV */
    {
    DEV_HDR ioDev;
    BOOL created;
    UINT regBase;
    } RTC_DEV;

typedef struct              /* rtc date and time structure */
    {
    UCHAR rtcYear;
    UCHAR rtcMonth;
    UCHAR rtcDayOfMonth;
    UCHAR rtcDayOfWeek;
    UCHAR rtcHour;
    UCHAR rtcMinute;
    UCHAR rtcSecond;
    } RTC_DATE_TIME;


/* defines */

#define RTC_DEVICE "/rtc"

#define SET_DATE    100
#define GET_DATE    101
#define SET_TIME    102
#define GET_TIME    103
#define STOP_RTC    104
#define START_RTC   105

#define rtc_Year            rtcDateTime.rtcYear
#define rtc_Month           rtcDateTime.rtcMonth
#define rtc_DayOfMonth      rtcDateTime.rtcDayOfMonth
#define rtc_DayOfWeek       rtcDateTime.rtcDayOfWeek
#define rtc_Hour            rtcDateTime.rtcHour
#define rtc_Minute          rtcDateTime.rtcMinute
#define rtc_Second          rtcDateTime.rtcSecond

/* offsets of 8 bit RTC fields relative to the base address */
#define CLOCK_CONTROL  0x7FFF8
#define CLOCK_SECONDS  0x7FFF9
#define CLOCK_MINUTES  0x7FFFA
#define CLOCK_HOUR     0x7FFFB
#define CLOCK_DAY      0x7FFFC
#define CLOCK_DATE     0x7FFFD
#define CLOCK_MONTH    0x7FFFE
#define CLOCK_YEAR     0x7FFFF

#define CLOCK_CAL_BITS  0x3f
#define CLOCK_READ_BIT  0x40
#define CLOCK_WRITE_BIT 0x80
#define CLOCK_STOP_BIT  0x80

#define CLOCK_YEAR_BITS         0xff
#define CLOCK_MONTH_BITS        0x1f
#define CLOCK_DATE_BITS         0x3f
#define CLOCK_DAY_BITS          0x07
#define CLOCK_HOUR_BITS         0x3f
#define CLOCK_MINUTE_BITS       0x7f
#define CLOCK_SECOND_BITS       0x7f


/* locals */

LOCAL RTC_DEV rtcDv =       /* device descriptor */
    {
    {{NULL}},
    FALSE
    };

LOCAL char *monthTable [12] =
    {
    "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
    "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
    };

LOCAL char *dayTable [7] =
    {
    "Sunday", "Monday", "Tuesday",
    "Wednesday", "Thursday", "Friday", "Saturday"
    };

LOCAL int rtcDrvNum;        /* driver number assigned to this driver */

LOCAL RTC_DATE_TIME rtcDateTime;

/* forward declarations */

LOCAL int rtcOpen (RTC_DEV *pRtcDv);
LOCAL int rtcClose (RTC_DEV *pRtcDv);
LOCAL STATUS rtcIoctl (RTC_DEV *pRtcDv, int request, BOOL print);
LOCAL int toBCD (int intValue);
LOCAL int fromBCD (int bcdValue);
LOCAL void rtcSpIoctl (int request, BOOL print);

/*******************************************************************************
*
* clockOutByte - writes one of the real time clock registers
*
* RETURNS: N/A
*/
void clockOutByte
    (
    UINT index,
    UCHAR value
    )
    {
    sysOutByte(rtcDv.regBase+index, value);
    }

/*******************************************************************************
*
* clockInByte - reads one of the real time clock registers
*
* RETURNS: contents of register read
*/
UCHAR clockInByte
    (
    UINT index
    )
    {
    return sysInByte(rtcDv.regBase+index);
    }

/*******************************************************************************
*
* rtcDrv - adding the driver to the driver system
*
* This routine must be called in supervisor state.
*
* RETURNS: OK or ERROR
*/
STATUS rtcDrv
    (
    void
    )
    {
    if (rtcDrvNum > 0)
        return (OK);

    rtcDrvNum = iosDrvInstall (rtcOpen, (FUNCPTR) NULL, rtcOpen,
                               rtcClose, (FUNCPTR) NULL, (FUNCPTR) NULL,
                               rtcIoctl);

    return (rtcDrvNum == ERROR ? ERROR : OK);
    }

/*******************************************************************************
*
* rtcDevCreate - create a device for the real time clock
*
* RETURN: OK or ERROR, if already created.
*/
STATUS rtcDevCreate
    (
    UINT rtcDevRegBase     /* the base addr is NVRAM_base  */
    )
    {
    /* test if driver already installed */

    if (rtcDrvNum <= 0)
        {
        (void) errnoSet (S_ioLib_NO_DRIVER);
        return (ERROR);
        }

    /*
     * if there is a device already created, don't do it
     */
    if (rtcDv.created)
        return (ERROR);

    /*
     * Save the base address of the clock,
     * mark the device as having been created,
     * and add it to the I/O system.
     */
    rtcDv.regBase = rtcDevRegBase;
    rtcDv.created = TRUE;

    return (iosDevAdd ((DEV_HDR *) &rtcDv, RTC_DEVICE, rtcDrvNum));
    }

/*******************************************************************************
*
* rtcOpen - open file to RTC
*/
LOCAL int rtcOpen
    (
    RTC_DEV *pRtcDv
    )
    {
    return ((int) pRtcDv);
    }

/*******************************************************************************
*
* rtcClose - close file to RTC
*/
LOCAL int rtcClose
    (
    RTC_DEV *pRtcDv
    )
    {
    return (OK);
    }

/*******************************************************************************
*
* rtcHook - date/time hook routine to set DOS file date and time
*
* NOMANUAL
*/
void rtcHook
    (
    DOS_DATE_TIME *pDateTime
    )
    {
    /*
     * update local date and time structure
     */
    rtcSpIoctl (GET_DATE, FALSE);
    rtcSpIoctl (GET_TIME, FALSE);

    /*
     * pass values to DOS date and time structure
     */
    if (rtc_Year <= 70)  /*if (rtc_Year >= 70)*/
        pDateTime->dosdt_year = 2000 + rtc_Year;    /* current year */
    else
        pDateTime->dosdt_year = 1900 + rtc_Year;    /* current year */
    pDateTime->dosdt_month = rtc_Month;             /* current month */
    pDateTime->dosdt_day = rtc_DayOfMonth;          /* current day */
    pDateTime->dosdt_hour = rtc_Hour;               /* current hour */
    pDateTime->dosdt_minute = rtc_Minute;           /* current minute */
    pDateTime->dosdt_second = rtc_Second;           /* current second */
    }
#if 1
void rtcDateTimeGet(T_TimeDate *pDateTime)
{

    rtcSpIoctl (GET_DATE, FALSE);
    rtcSpIoctl (GET_TIME, FALSE);

    /*
     * pass values to DOS date and time structure
     */
    if (rtc_Year <= 70)  /*if (rtc_Year >= 70)*/
        pDateTime->year = 2000 + rtc_Year;    /* current year */
    else
        pDateTime->year = 1900 + rtc_Year;    /* current year */
    pDateTime->month = rtc_Month;             /* current month */
    pDateTime->day = rtc_DayOfMonth;          /* current day */
    pDateTime->hour = rtc_Hour;               /* current hour */
    pDateTime->minute = rtc_Minute;           /* current minute */
    pDateTime->second = rtc_Second;           /* current second */
}
#endif
/*******************************************************************************
*
* rtcDateSet - set the date
*
* RETURNS: N/A.
*/
void rtcDateSet
    (
    int year,
    int month,
    int dayOfMonth,
    int dayOfWeek
    )
    {
    rtc_Year = year%100;
    rtc_Month = month;
    rtc_DayOfMonth = dayOfMonth;
    rtc_DayOfWeek = dayOfWeek;

    rtcSpIoctl (SET_DATE, FALSE);
    }

/*******************************************************************************
*
* rtcDateGet - get the date
*
* RETURNS: N/A.
*/
void rtcDateGet
    (
    void
    )
    {
    rtcSpIoctl (GET_DATE, TRUE);
    }

/*******************************************************************************
*
* rtcTimeSet - set the time
*
* RETURNS: N/A.
*/
void rtcTimeSet
    (
    int hour,
    int minute,
    int second
    )
    {
    rtc_Hour = hour;
    rtc_Minute = minute;
    rtc_Second = second;

    rtcSpIoctl (SET_TIME, FALSE);
    }

/*******************************************************************************
*
* rtcTimeGet - get the time
*
* RETURNS: N/A.
*/
void rtcTimeGet
    (
    void
    )
    {
    rtcSpIoctl (GET_TIME, TRUE);
    }

/*******************************************************************************
*
* rtcClockStop - stop the real time clock
*
* RETURNS: N/A.
*/
void rtcClockStop
    (
    void
    )
    {
    rtcSpIoctl (STOP_RTC, FALSE);
    }

/*******************************************************************************
*
* rtcClockStart - start the real time clock
*
* RETURNS: N/A.
*/
void rtcClockStart
    (
    void
    )
    {
    rtcSpIoctl (START_RTC, FALSE);
    }

/*******************************************************************************
*
* rtcSpIoctl - special device control routine
*
* This routine must be called by the user to access the driver ioctl routine
* correctly.
*
* NOMANUAL
*/
LOCAL void rtcSpIoctl
    (
    int request,    /* request code */
    BOOL print      /* print if TRUE, else only set structure */
    )
    {
    int fd;

    fd = open (RTC_DEVICE, 0, 0);

    ioctl (fd, request, print);

    close (fd);
    }

/*******************************************************************************
*
* rtcIoctl - special device control
*
* This driver responds to the ioctl calls described in the header.
*/
LOCAL STATUS rtcIoctl
    (
    FAST RTC_DEV *pRtcDv,   /* device to control */
    FAST int request,       /* request code */
    BOOL print              /* print if TRUE, else only set structure */
    )
    {
    UCHAR calibration;
    UCHAR stop_bit;
    UCHAR seconds;
    STATUS state = OK;      /* state of the ioctl calls */
    int century;
    int temp;

    switch (request)
        {
        case SET_DATE:
            temp = (int)rtc_DayOfWeek;
            if (temp < 0 || temp > 6)
                {
                printErr ("Invalid day of week: %d\n", rtc_DayOfWeek);
                break;
                }
            temp = (int)rtc_Month;
            if (temp < 1 || temp > 12)
                {
                printErr ("Invalid month: %d\n", rtc_Month);
                break;
                }

            calibration = clockInByte(CLOCK_CONTROL) & CLOCK_CAL_BITS;
            stop_bit = clockInByte(CLOCK_SECONDS) & CLOCK_STOP_BIT;
            /*
             * put clock in write mode
             */
            clockOutByte(CLOCK_CONTROL, calibration | CLOCK_WRITE_BIT);
            /*
             * update with passed in values
             */
            clockOutByte(CLOCK_DAY, toBCD(rtc_DayOfWeek));
            clockOutByte(CLOCK_DATE, toBCD(rtc_DayOfMonth));
            clockOutByte(CLOCK_MONTH, toBCD(rtc_Month));
            clockOutByte(CLOCK_YEAR, toBCD(rtc_Year % 100));
            /*
             * restore control register (to set the time)
             */
            clockOutByte(CLOCK_CONTROL, calibration);

        break;

        case GET_DATE:
     /*   	printf("rtc--getdate--------dddddddddd----------------\n");*/
            calibration = clockInByte(CLOCK_CONTROL) & CLOCK_CAL_BITS;
            /*
             * put clock in read mode
             */
            clockOutByte(CLOCK_CONTROL, calibration | CLOCK_READ_BIT);
            /*
             * read current values
             */
            rtc_DayOfWeek = fromBCD(clockInByte(CLOCK_DAY) & CLOCK_DAY_BITS);
            rtc_DayOfMonth = fromBCD(clockInByte(CLOCK_DATE) & CLOCK_DATE_BITS);
            rtc_Month = fromBCD(clockInByte(CLOCK_MONTH) & CLOCK_MONTH_BITS);
            rtc_Year = fromBCD(clockInByte(CLOCK_YEAR) & CLOCK_YEAR_BITS);
            /*
             * restore control register
             */
            clockOutByte(CLOCK_CONTROL, calibration);

            if (print)
                {
                temp = (int)rtc_DayOfWeek;
                if (temp < 0 || temp > 6)
                    {
                    printErr ("Invalid day of week: %d\n", rtc_DayOfWeek);
                    break;
                    }
                temp = (int)rtc_Month;
                if (temp < 1 || temp > 12)
                    {
                    printErr ("Invalid month: %d\n", rtc_Month);
                    break;
                    }
                if (rtc_Year <= 70)
                    century = 2000;
                else
                    century = 1900;
                printf ("Date: %s, %02d-%s-%4d.\n",
                    dayTable [rtc_DayOfWeek],
                    rtc_DayOfMonth,
                    monthTable [rtc_Month - 1],
                    century + rtc_Year);
                }

            break;

        case SET_TIME:

            calibration = clockInByte(CLOCK_CONTROL) & CLOCK_CAL_BITS;
            stop_bit = clockInByte(CLOCK_SECONDS) & CLOCK_STOP_BIT;
            /*
             * put clock in write mode
             */
            clockOutByte(CLOCK_CONTROL, calibration | CLOCK_WRITE_BIT);
            /*
             * update with passed in values
             */
            clockOutByte(CLOCK_SECONDS, toBCD(rtc_Second | stop_bit));
            clockOutByte(CLOCK_MINUTES, toBCD(rtc_Minute));
            clockOutByte(CLOCK_HOUR, toBCD(rtc_Hour));
            /*
             * restore control register (to set the time)
             */
            clockOutByte(CLOCK_CONTROL, calibration);

            break;

        case GET_TIME:
            calibration = clockInByte(CLOCK_CONTROL) & CLOCK_CAL_BITS;
            /*
             * put clock in read mode
             */
            clockOutByte(CLOCK_CONTROL, calibration | CLOCK_READ_BIT);
            /*
             * read current values
             */
            rtc_Second = fromBCD(clockInByte(CLOCK_SECONDS) & CLOCK_SECOND_BITS);
            rtc_Minute = fromBCD(clockInByte(CLOCK_MINUTES) & CLOCK_MINUTE_BITS);
            rtc_Hour = fromBCD(clockInByte(CLOCK_HOUR) & CLOCK_HOUR_BITS);
            /*
             * restore control register
             */
            clockOutByte(CLOCK_CONTROL, calibration);

            if (print)
                {
                printf ("Time: %02d:%02d:%02d.\n",
                    rtc_Hour, rtc_Minute, rtc_Second);
                }

            break;

        case STOP_RTC:
            /*
             * Read the current number of seconds and write it back with
             * the stop bit turned on.
             */
            calibration = clockInByte(CLOCK_CONTROL) & CLOCK_CAL_BITS;
            /*
             * put clock in read AND write mode
             */
            clockOutByte(CLOCK_CONTROL,
                calibration | CLOCK_READ_BIT | CLOCK_WRITE_BIT);
            seconds = clockInByte(CLOCK_SECONDS) & CLOCK_SECOND_BITS;
            clockOutByte(CLOCK_SECONDS, seconds | CLOCK_STOP_BIT);
            /*
             * restore control register
             */
            clockOutByte(CLOCK_CONTROL, calibration);

            break;

        case START_RTC:
            /*
             * Read the current number of seconds and write it back with
             * the stop bit cleared.
             */
            calibration = clockInByte(CLOCK_CONTROL) & CLOCK_CAL_BITS;
            /*
             * put clock in read AND write mode
             */
            clockOutByte(CLOCK_CONTROL,
                calibration | CLOCK_READ_BIT | CLOCK_WRITE_BIT);
            seconds = clockInByte(CLOCK_SECONDS) & CLOCK_SECOND_BITS;
            clockOutByte(CLOCK_SECONDS, seconds);
            /*
             * restore control register
             */
            clockOutByte(CLOCK_CONTROL, calibration);

            break;

        default:
            state = S_ioLib_UNKNOWN_REQUEST;
            break;
        }

    return (state);
    }

/*******************************************************************************
*
* toBCD - convert to binary coded decimal
*/
LOCAL int toBCD
    (
    int intValue
    )
    {
    int bcdValue;

    bcdValue = intValue%10;
    intValue /= 10;
    intValue *= 0x10;
    bcdValue += intValue;

    return (bcdValue);
    }

/*******************************************************************************
*
* fromBCD - convert from binary coded decimal
*/
LOCAL int fromBCD
    (
    int bcdValue
    )
    {
    int intValue;

    intValue = bcdValue&0xf;
    intValue += ((bcdValue&0xf0)/0x10)*10;

    return (intValue);
    }

