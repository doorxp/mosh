/*
    Mosh: the mobile shell
    Copyright 2012 Keith Winstein

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef NETWORK_TRANSPORT_HPP
#define NETWORK_TRANSPORT_HPP

#include <string>
#include <signal.h>
#include <time.h>
#include <list>
#include <vector>

#include "network.h"
#include "transportsender.h"
#include "transportfragment.h"


namespace Network {
  template <class MyState, class RemoteState>
  class Transport
  {
  private:
    /* the underlying, encrypted network connection */
    Connection connection;

    /* sender side */
    TransportSender<MyState> sender;

    /* helper methods for recv() */
    void process_throwaway_until( uint64_t throwaway_num );

    /* simple receiver */
    list< TimestampedState<RemoteState> > received_states;
    RemoteState last_receiver_state; /* the state we were in when user last queried state */
    FragmentAssembly fragments;
    bool verbose;

  public:
    Transport( MyState &initial_state, RemoteState &initial_remote,
	       const char *desired_ip, const char *desired_port );
    Transport( MyState &initial_state, RemoteState &initial_remote,
	       const char *key_str, const char *ip, int port );

    /* Send data or an ack if necessary. */
    void tick( void ) { sender.tick(); }

    /* Returns the number of ms to wait until next possible event. */
    int wait_time( void ) { return sender.wait_time(); }

    /* Blocks waiting for a packet. */
    void recv( void );

    /* Find diff between last receiver state and current remote state, then rationalize states. */
    string get_remote_diff( void );

    /* Shut down other side of connection. */
    /* Illegal to change current_state after this. */
    void start_shutdown( void ) { sender.start_shutdown(); }
    bool shutdown_in_progress( void ) const { return sender.get_shutdown_in_progress(); }
    bool shutdown_acknowledged( void ) const { return sender.get_shutdown_acknowledged(); }
    bool shutdown_ack_timed_out( void ) const { return sender.shutdown_ack_timed_out(); }
    bool has_remote_addr( void ) const { return connection.get_has_remote_addr(); }

    /* Other side has requested shutdown and we have sent one ACK */
    bool counterparty_shutdown_ack_sent( void ) const { return sender.get_counterparty_shutdown_acknowledged(); }

    int port( void ) const { return connection.port(); }
    string get_key( void ) const { return connection.get_key(); }

    MyState &get_current_state( void ) { return sender.get_current_state(); }
    void set_current_state( const MyState &x ) { sender.set_current_state( x ); }

    uint64_t get_remote_state_num( void ) const { return received_states.back().num; }

    const TimestampedState<RemoteState> & get_latest_remote_state( void ) const { return received_states.back(); }

    int fd( void ) const { return connection.fd(); }

    void set_verbose( void ) { sender.set_verbose(); verbose = true; }

    void set_send_delay( int new_delay ) { sender.set_send_delay( new_delay ); }

    uint64_t get_sent_state_acked_timestamp( void ) const { return sender.get_sent_state_acked_timestamp(); }
    uint64_t get_sent_state_acked( void ) const { return sender.get_sent_state_acked(); }
    uint64_t get_sent_state_last( void ) const { return sender.get_sent_state_last(); }

    unsigned int send_interval( void ) const { return sender.send_interval(); }

    const struct in_addr & get_remote_ip( void ) const { return connection.get_remote_ip(); }

    const NetworkException *get_send_exception( void ) const { return connection.get_send_exception(); }
  };
}

#endif
