#ifndef __RTC_H_
#define __RTC_H_

void LPC17xxHwRTCInit( void );

#define   OS_DEVICE_CTRL_RTC_GET_TIME         0x01
#define   OS_DEVICE_CTRL_RTC_SET_TIME         0x02
#define   OS_DEVICE_CTRL_RTC_GET_SECOND       0x03

#ifndef RTC_ILR_BITMASK
typedef struct {
	INT32U SEC; 		/*!< Seconds Register */
	INT32U MIN; 		/*!< Minutes Register */
	INT32U HOUR; 		/*!< Hours Register */
	INT32U DOM;		/*!< Day of Month Register */
	INT32U DOW; 		/*!< Day of Week Register */
	INT32U DOY; 		/*!< Day of Year Register */
	INT32U MONTH; 	/*!< Months Register */
	INT32U YEAR; 		/*!< Years Register */
} RTC_TIME_Type;
#endif

INT32U OSGetRTCSeconds(RTC_TIME_Type *RTC);

#endif