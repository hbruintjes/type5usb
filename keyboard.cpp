#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */
#include <avr/pgmspace.h>   /* required by usbdrv.h */
#include <string.h>

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

static inline void send_command(uint8_t c);

static inline void recv_command(uint8_t c) {
	if (c == 0xFF) {
		// Reset reply, ignore for now
		return;
	}
	if (c == 0x7F) {
		// Clear
		reportBuffer.modMask = 0;
		memset(reportBuffer.keys, 0x00, 6);
		return;
	}

	bool is_break = c >= 0x80;

	c &= 0x7F;
	auto key = keymap[c];
	if (key == KeyUsage::RESERVED) {
		return;
	}

	if (key >= KeyUsage::LEFTCTRL && key <= KeyUsage::RIGHTGUI)
	{
		auto bit = as_byte(key) - as_byte(KeyUsage::LEFTCTRL);
		if (!is_break) {
			reportBuffer.modMask |= _BV(bit);
		} else {
			reportBuffer.modMask &= ~_BV(bit);
		}
	} else {
		if (!is_break) {
			for (size_t i = 0; i < 6; i++) {
				if (reportBuffer.keys[i] == KeyUsage::RESERVED) {
					reportBuffer.keys[i] = key;
					break;
				}
			}
		} else {
			for (size_t i = 0; i < 6; i++) {
				if (reportBuffer.keys[i] == key) {
					reportBuffer.keys[i] = KeyUsage::RESERVED;
					break;
				}
			}
		}

	}
}

static inline void send_command(uint8_t c) {
	if (tx_buffer.full()) {
		return;
	}

	tx_buffer.push(c);
	if(!(LINENIR & _BV(LENTXOK))) {
		LINENIR |= _BV(LENTXOK);
		LINDAT = tx_buffer.pop();
	}
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
	// Reset uart
	LINCR = _BV(LSWRES);

	// INTR on RX and TX
	LINENIR = _BV(LENRXOK);// | _BV(LENTXOK);

	LINBTR = /*_BV(LDISR) | */static_cast<uint8_t>(sampling);

	// LINBRR  = ((SYS_CLK /(BAUDRATE*LINBTR))-1)&0x0FFF;
	LINBRR = static_cast<uint16_t>(((F_CPU*10/(baudrate*sampling)+5)/10) - 1) & 0x0FFF;

	// enable UART Full Duplex mode
	LINCR = _BV(LENA) | _BV(LCMD0) | _BV(LCMD1) | _BV(LCMD2);
}

ISR(LIN_TC_vect) {
	if(LINSIR & _BV(LRXOK)) {
		if (!rx_buffer.full()) {
			rx_buffer.push(LINDAT);
		} else {
			// Drop value
			uint8_t c = LINDAT;
		}
	}
	if(LINSIR & _BV(LTXOK)) {
		if (!tx_buffer.empty()) {
			LINDAT = tx_buffer.pop();
		} else {
			LINENIR &= ~_BV(LENTXOK);
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

	usbPoll();
	bool keyIntr = !rx_buffer.empty();
	while (!rx_buffer.empty()) {
		// Parse UART data
		recv_command(rx_buffer.pop());
	}
	if(keyIntr && usbInterruptIsReady()) {
		usbSetInterrupt((uchar *)&reportBuffer, sizeof(reportBuffer));
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

int main(void)
{
	wdt_disable();
	initTimer();
	uartInit(1200, 32);
	send_command(Keyboard::Command::RESET);

	usbReset();

	wdt_enable(WDTO_1S);
	for(;;) {                /* main event loop */
		wdt_reset();
		main_body();
	}
}
