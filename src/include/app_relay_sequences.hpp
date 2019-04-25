/*
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <quackmore-ff@yahoo.com> wrote this file.  As long as you retain this notice
 * you can do whatever you want with this stuff. If we meet some day, and you 
 * think this stuff is worth it, you can buy me a beer in return. Quackmore
 * ----------------------------------------------------------------------------
 */
#ifndef __RELAY_SEQUENCES_HPP__
#define __RELAY_SEQUENCES_HPP__

void init_relay_seq_default(void);
void init_relay_sequences(int idx, int sequence_idx);
void exe_relay_sequences(int idx);
void enable_relay_seq(int idx, int period);
void disable_relay_seq(int idx);

// random sequence
// pin
// period min
// period max
// pulses_count_min
// pulses_count_max
// pulses_high_length_min
// pulses_high_length_max
// pulses_low_length_min
// pulses_low_length_max

class relay_seq
{
  public:
    relay_seq(int id, int pin, int pulse_count, int32 pulse_high_length, int32 pulse_low_length, int32 period);
    ~relay_seq();
    int get_id(void);
    int get_pin(void);
    int get_pulse_count(void);
    int32 get_pulse_high_length(void);
    int32 get_pulse_low_length(void);
    int32 get_period(void);
    void enable(void);
    void disable(void);
    void exe(void);

  private:
    int m_id;
    int m_pin;
    int m_pulse_count;
    int32 m_pulse_high_length;
    int32 m_pulse_low_length;
    int32 m_period;
    os_timer_t m_period_timer;
    bool m_enabled;
    struct do_seq *m_seq;
};

#endif