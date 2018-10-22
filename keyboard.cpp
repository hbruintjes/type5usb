#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */
#include <avr/pgmspace.h>   /* required by usbdrv.h */

#include "usb/descriptor_kbd.h"

extern "C" {
	#include "usbdrv.h"
	#include "oddebug.h" /* This is also an example for using debug macros */
}

#include "keyboard/keyboard.h"

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
template<uint8_t Size>
class ring_buffer {
	static_assert(Size >= 2, "Ringbuffer too small");
public:
	void push(uint8_t c) {
		buffer[w_pos] = c;
		w_pos = static_cast<uint8_t>(w_pos + 1) % Size;
	}
	uint8_t cur() {
		return buffer[r_pos];
	}
	uint8_t pop() {
		auto& c = buffer[r_pos];
		r_pos = static_cast<uint8_t>(r_pos + 1) % Size;
		return c;
	}
	uint8_t size() {
		if (r_pos > w_pos) {
			return Size - (r_pos - w_pos);
		} else {
			return w_pos - r_pos;
		}
	}
	bool full() const {
		return (w_pos + 1) % Size == r_pos;
	}
	bool empty() const {
		return w_pos == r_pos;
	}
private:
	uint8_t buffer[Size] = {0};
	uint8_t r_pos = 0;
	uint8_t w_pos = 0;
};

static auto rx_buffer = ring_buffer<8>();
static auto tx_buffer = ring_buffer<8>();

static report_t reportBuffer = { 0 };
static uchar    idleRate;   /* repeat rate for keyboards, never used for mice */
bool keyIntr = false;

static inline void recv_command(uint8_t c) {
	if (c < 0x80) {
		reportBuffer.keys[0] = keymap[c];
	} else {
		KeyUsage usbKey = keymap[c - 0x80];
		reportBuffer.keys[0] = KeyUsage::RESERVED;
	}
	keyIntr = true;
}

static inline void send_command(uint8_t c) {
	cli();
	if (tx_buffer.full()) {
		sei();
		return;
	}
	if(LINSIR & _BV(LBUSY)) {
		tx_buffer.push(c);
	} else {
		LINDAT = c;
	}

	sei();
}

/* ------------------------------------------------------------------------- */
usbMsgLen_t usbFunctionSetup(uchar data[8])
{
	usbRequest_t *rq = (usbRequest_t *)data;

	/* The following requests are never used. But since they are required by
	 * the specification, we implement them in this example.
	 */
	if((rq->bmRequestType & USBRQ_TYPE_MASK) == USBRQ_TYPE_CLASS){    /* class request type */
		if(rq->bRequest == USBRQ_HID_GET_REPORT){  /* wValue: ReportType (highbyte), ReportID (lowbyte) */
			/* we only have one report type, so don't look at wValue */
			usbMsgPtr = (usbMsgPtr_t)&reportBuffer;
			return sizeof(reportBuffer);
		}else if(rq->bRequest == USBRQ_HID_SET_REPORT){
			/* Only one report type to consider, which is one byte exactly */
			if (rq->wLength.word == 1) {
				// Map LED bits to keyboard
				uint8_t ledStatus = 0;
				if (rq->wValue.bytes[1] & _BV(0)) {
					ledStatus |= Keyboard::LED::NUMLOCK;
				}
				if (rq->wValue.bytes[1] & _BV(1)) {
					ledStatus |= Keyboard::LED::CAPSLOCK;
				}
				if (rq->wValue.bytes[1] & _BV(2)) {
					ledStatus |= Keyboard::LED::SCROLLLOCK;
				}
				if (rq->wValue.bytes[1] & _BV(3)) {
					ledStatus |= Keyboard::LED::COMPOSE;
				}
				send_command(Keyboard::Command::LED_STATUS);
				send_command(ledStatus);
			}
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

void uartInit(uint16_t baudrate, uint8_t sampling = 32u)
{
	PORTA = _BV(PA0) & ~_BV(PA1); // RX on A0, TX on A1
	DDRA = 0xFF;

	// Reset uart
	LINCR = _BV(LSWRES);

	// INTR on RX and TX
	LINENIR =_BV(LENRXOK) | _BV(LENTXOK);
	LINDAT = 0;

	LINBTR = static_cast<uint8_t>(_BV(LDISR) | sampling);

	// LINBRR  = ((SYS_CLK /(BAUDRATE*LINBTR))-1)&0x0FFF;
	LINBRR = static_cast<uint16_t>(((F_CPU*10/(baudrate*sampling)+5)/10) - 1);

	// enable UART Full Duplex mode
	LINCR = _BV(LENA) | _BV(LCMD0) | _BV(LCMD1) | _BV(LCMD2);
}

ISR(LIN_TC_vect) {
	if(LINSIR & _BV(LRXOK)) {
		keyIntr = true;
		if (!rx_buffer.full()) {
			rx_buffer.push(LINDAT);
		} else {
			// Drop value
			uint8_t c = LINDAT;
		}
	}
	if(LINSIR & _BV(LTXOK)) {
		if (tx_buffer.empty()) {
			PORTB &= ~_BV(PORTB1);
		} else {
			LINDAT = tx_buffer.pop();
		}
	}
}


/* ------------------------------------------------------------------------- */


void initTimer() {
	TCCR0A = _BV(WGM01);
	TCCR0B = _BV(CS01) | _BV(CS00);
	OCR0A = 0xff; // max delay
	TIMSK0 = _BV(OCIE0A);
}

ISR(TIMER0_COMPA_vect) {
	static uint16_t count = 0;
	if (count++ == 0x800)
	{
		count = 0;
	}
}

static inline void main_body() {
	wdt_reset(); //reset watchdog timer
	usbPoll();
	while (!rx_buffer.empty()) {
		// Parse UART data
		recv_command(rx_buffer.pop());
	}
	if(keyIntr && usbInterruptIsReady()) {
		keyIntr = false;
		//usbSetInterrupt((uchar *)&reportBuffer, sizeof(reportBuffer));
	}
}

int main(void)
{
	wdt_enable(WDTO_1S); // enable the watchdog
	uartInit(1200, 32);
	usbInit();
	initTimer();
	DDRB = _BV(PORTB1);
	usbDeviceDisconnect();  /* enforce re-enumeration, do this while interrupts are disabled! */
	uchar i = 0;
	while(--i){             /* fake USB disconnect for > 250 ms */
		wdt_reset();
		_delay_ms(1);
	}
	usbDeviceConnect();
	sei();
	send_command(Keyboard::Command::RESET);
	for(;;) {                /* main event loop */
		main_body();
	}
}
