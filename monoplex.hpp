#ifndef MONOPLEX_HPP
#define MONOPLEX_HPP

#include <cstdint>
#include <utility>

#include <Arduino.h>
#include <HardwareSerial.h>

#if !defined(__SAM3X8E__)
#error This library assumes the SAM3X8E (Arduino Due) chipset. __SAM3X8E__ must also be defined.
#endif

namespace monoplex {
    /** Bus error codes */
    enum class Error : std::uint8_t {
        NONE = 0,
        FAILURE,
    };

    /** Combined error and response for reads. */
    struct Response { // TODO: union instead?
    public:
        Error err;
        std::uint8_t value;

        inline bool is_ok() const { return !is_err(); }
        inline bool is_err() const { return err != Error::NONE; }
    };

    /** State of the bus' RS485 toggling */
    enum class Status : std::uint8_t {
        SEND = 0,
        RECV,
        UNKNOWN,
    };


    /** Wraps the native HardwareSerial to toggle RS485 mode enabling half-duplex hardware serial.
     */
    class Serial {
    protected:
        HardwareSerial*   raw;
        Uart*             uart;
        Pio*              rts_reg;
        std::uint32_t     rts_pin;

        std::size_t       byte_time_delay;      // max timeout for one byte (amortized) in microseconds
        volatile Status   status;               // status of the transmitter line (thus the bus)

    public:
        Serial(HardwareSerial* serial_port, Usart* usart, Pio* rts_reg, std::size_t rts_pin)
            : raw(serial_port)
            , uart(reinterpret_cast<Uart*>(usart))
            , rts_reg(rts_reg), rts_pin(rts_pin)
            , byte_time_delay(0), status(Status::UNKNOWN)
        {}

        /** Initialize communication and places the bus in RS485 mode.
         */
        void begin(std::size_t baud);

        /** Finalizes the communication, undoes RS485 settings, and returns control of the RTS pin.
         */
        void end();

        /** Writes a single byte to the wire (or internal buffer)
         *
         * Does not flush the internal buffer.
         *
         * Returns 0 on error (0 bytes written).
         * Returns 1 on success (1 byte written) though only non-null should be assumed.
         */
        std::size_t write(const std::uint8_t data);

        /** Writes a buffer byte-by-byte to the wire (or internal buffer)
         *
         * Blocks until all bytes have been pushed, but does no flushing.
         *
         * Returns 0 on error (0 bytes written).
         * Returns N on success (N byte written) though only non-null should be assumed.
         */
        std::size_t write(const std::uint8_t *buffer, std::size_t size);

        /** Read a message off the wire.
         *
         * Returns a Response object containing the value read, and an error field (mainly for checking
         * validity of the returned value).
         */
        Response read();

        /** Expect to receive (and ignore) a set number of bytes.
         *
         * Returns false if a timeout occurs, otherwise true.
         */
        bool skip(const std::size_t count);

        /** Are bytes available to be read. (legacy naming convention)
         */
        inline std::size_t available() { return raw->available(); }

        /** Flush the transmitting internal buffer */
        void flush() { return raw->flush(); }

        /** Flush the receiving internal buffer, skipping the data */
        void flush_rx() { while (available()) { read(); } }

        /** Is the bus currently in transmit mode. */
        inline bool is_transmitting() { return status == Status::SEND; }


        #ifdef ID_USART0
        static const Serial at_usart0() {
            // RTS pin = PB25 = D2     TODO
            return Serial(&Serial1, USART0, PIOB, PIO_PB25A_RTS0);
        }
        #endif
    
        #ifdef ID_USART1
        static const Serial at_usart1() {
            // RTS pin = PA14 = D23
            return Serial(&Serial2, USART1, PIOA, PIO_PA14A_RTS1);
        }
        #endif
    
    }; // serial class
} // namespace

#endif // header guard
