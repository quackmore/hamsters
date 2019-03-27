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

#endif