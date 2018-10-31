#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <avr/sleep.h>
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
static uchar idleRate[3] = {0};
static uchar idleTime[3] = {0};
static keyboard::report_type type = keyboard::report_type::none;
static keyboard::keyhandler keyboard_handler;

/* ------------------------------------------------------------------------- */
uchar usbFunctionWrite(uchar *data, uchar len) {
	/* Only one report type to consider, which is one byte exactly */
	if (len == sizeof(keyboard::led_report_t) &&
	  static_cast<keyboard::report_type>(data[0]) == keyboard::report_type::key) {
		keyboard_handler.set_led_report(data[1]);
	}

	// Done
	return 1;
}

usbMsgLen_t usbFunctionSetup(uchar data[8]) {
	auto *rq = reinterpret_cast<usbRequest_t*>(data);

	if ((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS) {    /* class request type */
		/* wValue: ReportType (highbyte), ReportID (lowbyte) */
		/* we only have one report type, so don't look at wValue */
		type = static_cast<const keyboard::report_type>(rq->wValue.bytes[0]);
		if (rq->bRequest == USBRQ_HID_GET_REPORT) {
			return keyboard_handler.set_report_ptr(&usbMsgPtr, rq->wValue.bytes[1], type);
		} else if (rq->bRequest == USBRQ_HID_SET_REPORT) {
			// Let usbFunctionWrite take care of things
			return USB_NO_MSG;
		} else if (rq->bRequest == USBRQ_HID_GET_IDLE) {
			if (type == keyboard::report_type::boot) {
				usbMsgPtr = &idleRate[0];
			} else {
				usbMsgPtr = &idleRate[rq->wValue.bytes[0] - 1];
			}
			return 1;
		} else if (rq->bRequest == USBRQ_HID_SET_IDLE) {
			if (type == keyboard::report_type::boot) {
				idleTime[0] = idleRate[0] = rq->wValue.bytes[1];
			} else {
				idleRate[rq->wValue.bytes[0] - 1] =
					idleRate[rq->wValue.bytes[0] - 1] = rq->wValue.bytes[1];
			}
		} else if (rq->bRequest == USBRQ_HID_GET_PROTOCOL) {
			usbMsgPtr = &keyboard_handler.get_protocol();
			return 1;
		} else if (rq->bRequest == USBRQ_HID_SET_PROTOCOL) {
			keyboard_handler.set_protocol(rq->wValue.bytes[0]);
		}
	}
	return 0; // No data returned
}

/* ------------------------------------------------------------------------- */


void initTimer() {
	TCCR0A = _BV(WGM01);
	TCCR0B = _BV(CS01) | _BV(CS00); // freq = F_CPU/1024
	OCR0A = 0x10; // Compare 16, INTR_freq = freq/16r
	TIMSK0 = _BV(OCIE0A);

	TCCR1A = _BV(WGM12); // Simple CTC timer
	TCCR1B = _BV(WGM12) | _BV(CS12); // Simple CTC timer, prescale 256
	OCR1A = 2500; // Compare at 2500, meaning 4ms delay
	TIMSK1 = _BV(OCIE1A);
}

// Watchdog used to perform soft-reset on USB timeout
// (15ms is the min., USB specifies suspend already after 3ms)
static volatile uchar prevSofCount = 0;
extern volatile uchar usbSofCount;
volatile uchar usbSofCount;
ISR(TIMER0_COMPA_vect) {
    // Once every ~1ms
	if (prevSofCount != usbSofCount) {
		wdt_reset();
		prevSofCount = usbSofCount;
	} else {
		wdt_reset();
	}
}

ISR(TIMER1_COMPA_vect) {
	// Once every 4ms
	for (uint8_t i = 0; i < 3; i++) {
		if (idleTime[i] > 1) {
			idleTime[i]--;
		} else if (idleTime[i] == 1) {
			if (usbInterruptIsReady()) {
				// Disable interrupt until report is sent
				TIMSK1 = 0;
				//keyboard_handler.send_report_intr(static_cast<keyboard::report_type>(i));
				idleTime[i] = idleRate[i];
				TIMSK1 = _BV(OCIE1A);
			}
		}
	}
}

static void usbReset() {
	cli();
	usbDeviceDisconnect();
	usbInit();
	/* fake USB disconnect for > 250 ms */
	uchar i = 0;
	while(--i){
		_delay_ms(1);
	}
	usbDeviceConnect();
	sei();
}

//void main(void) __attribute__((noreturn));

int main()
{
	wdt_disable();
/*
	// Turn off stuff not needed
	PORTB = 0;
	DDRB |= _BV(PORTB1);
	PORTB |= _BV(PORTB1);
	ACSR |= _BV(ACD);
	ADCSRA = 0;
	PRR = _BV(PRLIN) | _BV(PRSPI) | _BV(PRTIM1) | _BV(PRTIM0) | _BV(PRUSI) | _BV(PRADC);

	// Clear reset status flag
	MCUSR = 0;

	cli();
	// Enable USB interrupts and go to sleep immediately, waking up
	// on USB activity
	usbDeviceConnect();
	// Enable interrupt on pin change,
	// usbdrv.h does not have a place for that
	EIMSK |= _BV(PCIE1);
	usbInit(); // Enables USB interrupts

	// Go to sleep if not in USB reset mode
	set_sleep_mode(SLEEP_MODE_PWR_DOWN);
	if (USBIN & USBMASK) { // Reset condition of both pins low
		PORTB &= ~_BV(PORTB1);
		sleep_enable();
		sei();
		// Not reset, assume suspended
		sleep_cpu();
		sleep_disable();
	}
	sei();

	PORTB |= _BV(PORTB1);
	// Re-enable LIN and timers
	PRR = _BV(PRSPI) | _BV(PRUSI) | _BV(PRADC);
*/
	// Init peripheries
	initTimer();
	uart::init(1200);
	keyboard_handler.init();
	usbReset();

	// Wait for configuration to be set. Only then enable kbd and draw power
	while(usbConfiguration != 1) {
		usbPoll();
	}

	keyboard_handler.enable();

	//wdt_enable(WDTO_15MS);
	for(;;) {                /* main event loop */
		usbPoll();
		keyboard_handler.poll_event();
	}
}
