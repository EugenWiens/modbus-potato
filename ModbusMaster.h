#include <initializer_list>
#include <iterator>
#include "ModbusInterface.h"
namespace ModbusPotato
{
    /// <summary>
    /// This class implements a basic Modbus master interface.
    /// </summary>
    class CModbusMaster : public IFrameHandler
    {
    public:
        CModbusMaster(IMasterHandler* handler, IFramer* framer, ITimeProvider* timer, unsigned int response_time_out = 200, unsigned int turnaround_delay = 1000);

        bool read_coils_req(void);
        bool read_discrete_inputs_req(void);
        inline bool read_holding_registers_req(const uint8_t slave, const uint16_t address, const uint16_t n)
        {
            return read_registers_req(function_code::read_holding_registers, slave, address, n);
        }
        inline bool read_input_registers_req(const uint8_t slave, const uint16_t address, const uint16_t n)
        {
            return read_registers_req(function_code::read_input_registers, slave, address, n);
        }

        bool write_single_coil_req(void);
        inline bool write_single_register_req(const uint8_t slave, const uint16_t address, const uint16_t value)
        {
            const std::initializer_list<uint16_t> data = {value};
            return write_registers_req(function_code::write_single_register, slave, address, 1, data.begin(), data.end());
        }
        bool write_multiple_coils_req(void);
        inline bool write_multiple_registers_req(const uint8_t slave, const uint16_t address, const std::initializer_list<uint16_t> data)
        {
            return write_multiple_registers_req(slave, address, data.size(), data.begin(), data.end());
        }
        template <typename ITER>
        inline bool write_multiple_registers_req(const uint8_t slave, const uint16_t address, const size_t n, const ITER begin, const ITER end)
        {
            return write_registers_req(function_code::write_multiple_registers, slave, address, n, begin, end);
        }

        void poll(void);

        void frame_ready(IFramer* framer);

    private:
        enum class state {
                idle,
                waiting_for_reply,
                processing_reply,
                waiting_turnaround_reply,
                processing_error,
        };

        IMasterHandler* m_handler;
        IFramer* m_framer;
        ITimeProvider* m_time_provider;
        system_tick_t m_response_time_out;
        system_tick_t m_turnaround_delay;

        enum state m_state;
        system_tick_t m_timer;
        uint16_t m_slave_address;
        uint16_t m_starting_register;

        bool read_registers_req(const enum function_code::function_code func, const uint8_t slave, const uint16_t address, const uint16_t n);
        bool read_registers_rsp(IFramer* framer, bool holding);

        template <typename ITER>
        bool write_registers_req(const enum function_code::function_code func, const uint8_t slave, const uint16_t address, const size_t n, const ITER begin, const ITER end);
        bool write_registers_rsp(IFramer* framer, bool single);

        bool sanity_check(const size_t n, const size_t len);
        void send_and_wait(uint8_t slave, uint16_t address, size_t len);
};
}
