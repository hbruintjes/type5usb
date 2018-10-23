#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */
#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include <string.h>
#include <keyboard/Keyboard.h>
#include <keyboard/uart.h>

#include "usb/descriptor_kbd.h"

extern "C" {
	#include "usbdrv.h"
	#include "oddebug.h" /* This is also an example for using debug macros */
}

#include "keyboard/keymap.h"

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */

/* repeat rate for keyboards, never used for mice */
static uchar idleRate = 0;
static uchar idleTime = 0;
static keyboard::keyboard keyboard_handler;

/* ------------------------------------------------------------------------- */
uchar usbFunctionWrite(uchar *data, uchar len) {
	/* Only one report type to consider, which is one byte exactly */
	if (len == 1) {
		keyboard_handler.set_led_report(data[0]);
	}

	// Done
	return 1;
}

usbMsgLen_t usbFunctionSetup(uchar data[8]) {
	auto *rq = reinterpret_cast<usbRequest_t*>(data);

	if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {    /* class request type */
		/* wValue: ReportType (highbyte), ReportID (lowbyte) */
		/* we only have one report type, so don't look at wValue */
		if(rq->bRequest == USBRQ_HID_GET_REPORT) {
			return keyboard_handler.set_report_ptr(&usbMsgPtr);
		}else if(rq->bRequest == USBRQ_HID_SET_REPORT) {
			// Let usbFunctionWrite take care of things
			return USB_NO_MSG;
		}else if(rq->bRequest == USBRQ_HID_GET_IDLE) {
			usbMsgPtr = &idleRate;
			return 1;
		}else if(rq->bRequest == USBRQ_HID_SET_IDLE) {
			idleRate = rq->wValue.bytes[1];
			idleTime = idleRate;
		}
	}
	return 0; // No data returned
}

/* ------------------------------------------------------------------------- */


void initTimer() {
//	TCCR0A = _BV(WGM01);
//	TCCR0B = _BV(CS01) | _BV(CS00); // freq = F_CPU/1024
//	OCR0A = 0xff; // Compare 255, INTR_freq = freq/255
//	TIMSK0 = _BV(OCIE0A);

	TCCR1A = _BV(WGM12); // Simple CTC timer
	TCCR1B = _BV(WGM12) | _BV(CS12); // Simple CTC timer, prescale 256
	OCR1A = 2500; // Compare at 2500, meaning 4ms delay
	TIMSK1 = _BV(OCIE1A);
}

ISR(TIMER0_COMPA_vect) {
    // Once every 16.32 ms
}

ISR(TIMER1_COMPA_vect) {
	// Once every 4ms
	if (idleTime > 1) {
		idleTime--;
	} else if (idleTime == 1) {
		if (usbInterruptIsReady()) {
			// Disable interrupt until report is sent
			TIMSK1 = 0;
			keyboard_handler.send_report_intr();
			idleTime = idleRate;
			TIMSK1 = _BV(OCIE1A);
		}
	}
}

static inline void main_body() {
	usbPoll();
	bool keyIntr = false;
	// Read (slow) uart data
	while (!keyIntr && uart::poll()) {
		// Parse UART data
		keyIntr = keyboard_handler.poll_event();
	}
	if (keyIntr) {
		// Wait for USB ready before handling next key
		while(!usbInterruptIsReady() && !uart::full()) {
			usbPoll();
			wdt_reset();
		}
		keyboard_handler.send_report_intr();
	}
}

static void usbReset() {
	cli();
	usbDeviceDisconnect();
	usbInit();
	/* fake USB disconnect for > 250 ms */
	uchar i = 0;
	while(--i){
		wdt_reset();
		_delay_ms(1);
	}
	usbDeviceConnect();
	sei();
}

int main()
{
	wdt_disable();
	initTimer();
	uart::init(1200);
	keyboard_handler.init();

	usbReset();

	wdt_enable(WDTO_1S);
	for(;;) {                /* main event loop */
		wdt_reset();
		main_body();
	}
}
