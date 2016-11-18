
#include "monoplex.hpp"

namespace monoplex {

void Serial::begin(std::size_t baud) {
    byte_time_delay = (90000000 / baud);
    raw->begin(baud);

    rts_reg->PIO_PDR   |=  rts_pin;        // disable PIO control of the RTS pin
    rts_reg->PIO_ABSR  &=  ~rts_pin;       // set the RTS pin to be a well... RTS
    uart->UART_MR      |=  US_MR_USART_MODE_RS485;
}

void Serial::end() {
    raw->end();
    rts_reg->PIO_PER |= rts_pin;           // release (enable PIO control of) the rts pin
}


std::size_t Serial::write(std::uint8_t data) {
    status = Status::SEND;
    return raw->write(data);
}

std::size_t Serial::write(const std::uint8_t* buffer,  std::size_t size) {
    status = Status::SEND;
    return raw->write(buffer, size);
    // TODO: done here?
}


bool Serial::skip(const std::size_t count) {
    auto timeout = micros() + byte_time_delay * count;

    while (available() < count) {
        if (micros() > timeout) { return false; } // TODO: is "uptime" a reasonable clock
    }
    
    return true;
}

Response Serial::read() {
    int val = raw->read();
    return Response{
        (val >= 0) ? Error::NONE : Error::FAILURE,  // TODO: verify return codes and elaborate errors
        (std::uint8_t)(val & 0xFF)
    };
}

} // namepsace
