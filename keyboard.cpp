#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  /* for sei() */
#include <util/delay.h>     /* for _delay_ms() */
#include <avr/pgmspace.h>   /* required by usbdrv.h */

#include "usb/descriptor.h"

extern "C" {
	#include "usbdrv.h"
	#include "oddebug.h" /* This is also an example for using debug macros */
}

/* ------------------------------------------------------------------------- */
/* ----------------------------- USB interface ----------------------------- */
template<uint8_t Size>
class ring_buffer {
public:
	void push(uint8_t c) {
		buffer[w_pos] = c;
		w_pos = static_cast<uint8_t>(w_pos + 1) % Size;
	}
	uint8_t pop() {
		auto& c = buffer[r_pos];
		r_pos = static_cast<uint8_t>(r_pos + 1) % Size;
		return c;
	}
	bool full() const {
		return (r_pos + 1) % Size == w_pos;
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

static report_t reportBuffer;
static int      x = 0, y = 0;
static uchar    idleRate;   /* repeat rate for keyboards, never used for mice */

static void advanceCircleByFixedAngle()
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

void uartInit(uint16_t baudrate, uint8_t sampling = 32u)
{
	PORTA = _BV(PA0) & ~_BV(PA1); // RX on A0, TX on A1
	DDRA = 0xFF;

	// Reset uart
	LINCR = _BV(LSWRES);

	// INTR on RX
	LINENIR =_BV(LENRXOK);

	LINBTR = static_cast<uint8_t>(_BV(LDISR) | sampling);

	// LINBRR  = ((SYS_CLK /(BAUDRATE*LINBTR))-1)&0x0FFF;
	LINBRR = static_cast<uint16_t>(((F_CPU*10/(baudrate*sampling)+5)/10) - 1);

	// enable UART Full Duplex mode
	LINCR = _BV(LENA) | _BV(LCMD0) | _BV(LCMD1) | _BV(LCMD2);
}

SIGNAL(LIN_TC_vect)
{
	if(LINSIR & _BV(LRXOK)) {
		if (!rx_buffer.full()) {
			rx_buffer.push(LINDAT);
		} else {
			// Drop value
			uint8_t c = LINDAT;
		}
	}
	if(LINSIR & _BV(LTXOK)){
		if (tx_buffer.empty()) {
			// Buffer empty, so disable interrupts
			LINENIR &= ~_BV(LENTXOK);
		} else {
			// There is more data in the output buffer. Send the next byte
			LINDAT = tx_buffer.pop();
		}
	}
}

/* ------------------------------------------------------------------------- */
static inline void recv(uint8_t c) {
	//
}

static inline void send(uint8_t c) {
	while (tx_buffer.full()) {
		//Idle
	}
	if (tx_buffer.empty()) {
		LINENIR |= _BV(LENTXOK);
		LINDAT = c;
	} else {
		tx_buffer.push(c);
	}
}

static inline void main_body() {
	wdt_reset(); //reset watchdog timer
	usbPoll();
	if(usbInterruptIsReady()){
		/* called after every poll of the interrupt endpoint */
		advanceCircleByFixedAngle();
		usbSetInterrupt((uchar *)&reportBuffer, sizeof(reportBuffer));
	}
	if (!rx_buffer.empty()) {
		// Parse UART data
		rx_buffer.pop();
	}
}
int main(void)
{
	wdt_enable(WDTO_1S); // enable the watchdog
	uartInit(1200, 32);
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
