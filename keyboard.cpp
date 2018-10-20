#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */

#include <avr/pgmspace.h>   /* required by usbdrv.h */
extern "C" {
	#include "usbdrv.h"
	#include "oddebug.h" /* This is also an example for using debug macros */
}

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
template<char ... Args>
constexpr char mk_tag(int code) noexcept {
	return (code | sizeof...(Args));
}
//NUMARGS(...)  (sizeof((int[]){__VA_ARGS__})/sizeof(int))

#define MK_TAG(val, ...)    mk_tag<__VA_ARGS__>(val), ##__VA_ARGS__

/* ------------------------------------------------------------------------- */
// Main item tags
#define INPUT(...)          MK_TAG(0b1000'0000, ##__VA_ARGS__)
#define OUTPUT(...)         MK_TAG(0b1001'0000, ##__VA_ARGS__)
#define FEATURE(...)        MK_TAG(0b1011'0000, ##__VA_ARGS__)
#define COLLECTION(...)     MK_TAG(0b1010'0000, ##__VA_ARGS__)
#define END_COLLECTION()    MK_TAG(0b1100'0000)

namespace MainFlag {
	constexpr char Constant = 0x01;
	constexpr char Variable = 0x02;
	constexpr char Relative = 0x04;
	constexpr char Wrap = 0x08;
	constexpr char Nonlinear = 0x10;
	constexpr char NoPreferred = 0x20;
	constexpr char NullState = 0x40;
	constexpr char Volatile = (char)0x80;
};

namespace Collection {
	constexpr char Physical = 0x00;
	constexpr char Application = 0x01;
	constexpr char Logical = 0x02;
	constexpr char Report = 0x03;
	constexpr char NamedArray = 0x04;
	constexpr char UsageSwitch = 0x05;
	constexpr char UsageModifier = 0x06;
}

// Global item tags
#define USAGE_PAGE(...)   MK_TAG(0b0000'0100, ##__VA_ARGS__)
#define LOGICAL_MIN(...)  MK_TAG(0b0001'0100, ##__VA_ARGS__)
#define LOGICAL_MAX(...)  MK_TAG(0b0010'0100, ##__VA_ARGS__)
#define PHYSICAL_MIN(...) MK_TAG(0b0011'0100, ##__VA_ARGS__)
#define PHYSICAL_MAX(...) MK_TAG(0b0100'0100, ##__VA_ARGS__)
#define REPORT_SIZE(...)  MK_TAG(0b0111'0100, ##__VA_ARGS__)
#define REPORT_ID(...)    MK_TAG(0b1000'0100, ##__VA_ARGS__)
#define REPORT_COUNT(...) MK_TAG(0b1001'0100, ##__VA_ARGS__)

namespace UsagePage {
	constexpr char Undefined = 0x00;
	constexpr char GenericDesktop = 0x01;
	constexpr char SimulationControls = 0x02;
	constexpr char VRControls = 0x03;
	constexpr char SportCOntrols = 0x04;
	constexpr char GameControls = 0x05;
	constexpr char GenericDeviceControls = 0x06;
	constexpr char Keyboard = 0x07;
	constexpr char LEDs = 0x08;
	constexpr char Button = 0x09;
	constexpr char Ordinal = 0x0A;
};

namespace GenericDesktop {
	constexpr char Undefined = 0x00;
	constexpr char Pointer = 0x01;
	constexpr char Mouse = 0x02;

	constexpr char JoyStick = 0x04;
	constexpr char GamePad = 0x05;
	constexpr char Keyboard = 0x06;
	constexpr char Keypad = 0x07;
	constexpr char MultiaxisController = 0x08;

	constexpr char X = 0x30;
	constexpr char Y = 0x31;
	constexpr char Z = 0x32;

	constexpr char Wheel = 0x38;
};

// Local item tags
#define USAGE(...)     MK_TAG(0b0000'1000, ##__VA_ARGS__)
#define USAGE_MIN(...) MK_TAG(0b0001'1000, ##__VA_ARGS__)
#define USAGE_MAX(...) MK_TAG(0b0010'1000, ##__VA_ARGS__)

PROGMEM const char usbHidReportDescriptor[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = { /* USB report descriptor, size must match usbconfig.h */
	USAGE_PAGE(UsagePage::GenericDesktop),
	USAGE(GenericDesktop::Mouse),
	COLLECTION(Collection::Application),
		USAGE(GenericDesktop::Pointer),
		COLLECTION(Collection::Physical),
			USAGE_PAGE(UsagePage::Button),
			USAGE_MIN(0x01),
			USAGE_MAX(0x03),
			LOGICAL_MIN(0x00),
			LOGICAL_MAX(0x01),
			REPORT_COUNT(0x03),
			REPORT_SIZE(0x01),
			INPUT(0x02),                    // (Data,Var,Abs)

			REPORT_COUNT(0x01),
			REPORT_SIZE(0x05),
			INPUT(0x03),                    // (Const,Var,Abs)

			USAGE_PAGE(UsagePage::GenericDesktop),
			USAGE(GenericDesktop::X),
			USAGE(GenericDesktop::Y),
			USAGE(GenericDesktop::Wheel),
			LOGICAL_MIN((char)0x81),
			LOGICAL_MAX(0x7F),
			REPORT_SIZE(0x08),
			REPORT_COUNT(0x03),
			INPUT(0x06),                    // (Data,Var,Rel)
		END_COLLECTION(),
	END_COLLECTION()
};
/* This is the same report descriptor as seen in a Logitech mouse. The data
 * described by this descriptor consists of 4 bytes:
 *      .  .  .  .  . B2 B1 B0 .... one byte with mouse button states
 *     X7 X6 X5 X4 X3 X2 X1 X0 .... 8 bit signed relative coordinate x
 *     Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0 .... 8 bit signed relative coordinate y
 *     W7 W6 W5 W4 W3 W2 W1 W0 .... 8 bit signed relative coordinate wheel
 */
typedef struct{
	uchar   buttonMask;
	char    dx;
	char    dy;
	char    dWheel;
}report_t;

static report_t reportBuffer;
static int      x = 0, y = 0;
static uchar    idleRate;   /* repeat rate for keyboards, never used for mice */

static void advanceCircleByFixedAngle(void)
{
	if (x < 5 && y == 0) {
		x++;
		reportBuffer.dx = 1;
	} else if (x == 5 && y < 5) {
		y++;
		reportBuffer.dy = 1;
	} else if (x > 0 && y == 5) {
		x--;
		reportBuffer.dx = -1;
	} else {
		y--;
		reportBuffer.dy = -1;
	}
}

/* ------------------------------------------------------------------------- */

usbMsgLen_t usbFunctionSetup(uchar data[8])
{
	usbRequest_t    *rq = (usbRequest_t *)data;

	/* The following requests are never used. But since they are required by
	 * the specification, we implement them in this example.
	 */
	if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* class request type */
		DBG1(0x50, &rq->bRequest, 1);   /* debug output: print our request */
		if(rq->bRequest == USBRQ_HID_GET_REPORT){  /* wValue: ReportType (highbyte), ReportID (lowbyte) */
			/* we only have one report type, so don't look at wValue */
			usbMsgPtr = (usbMsgPtr_t)&reportBuffer;
			return sizeof(reportBuffer);
		}else if(rq->bRequest == USBRQ_HID_GET_IDLE){
			usbMsgPtr = &idleRate;
			return 1;
		}else if(rq->bRequest == USBRQ_HID_SET_IDLE){
			idleRate = rq->wValue.bytes[1];
		}
	}else{
		/* no vendor specific requests implemented */
	}
	return 0;   /* default for not implemented requests: return no data back to host */
}

/* ------------------------------------------------------------------------- */
static inline void main_body() {
	wdt_reset();
	usbPoll();
	if(usbInterruptIsReady()){
		/* called after every poll of the interrupt endpoint */
		advanceCircleByFixedAngle();
		usbSetInterrupt((uchar *)&reportBuffer, sizeof(reportBuffer));
	}
}
int main(void)
{
	wdt_enable(WDTO_1S);
	/* If you don't use the watchdog, replace the call above with a wdt_disable().
	 * On newer devices, the status of the watchdog (on/off, period) is PRESERVED
	 * OVER RESET!
	 */
	/* RESET status: all port bits are inputs without pull-up.
	 * That's the way we need D+ and D-. Therefore we don't need any
	 * additional hardware initialization.
	 */
	usbInit();
	usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
	uchar i = 0;
	while(--i){             /* fake USB disconnect for > 250 ms */
		wdt_reset();
		_delay_ms(1);
	}
	usbDeviceConnect();
	sei();
	for(;;){                /* main event loop */
		main_body();
	}
}
