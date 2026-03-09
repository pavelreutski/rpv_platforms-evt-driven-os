#pragma once

typedef unsigned long eth_counter_t;

eth_counter_t eth_rxqhead(void);
eth_counter_t eth_rxqtail(void);

eth_counter_t eth_txqhead(void);
eth_counter_t eth_txqtail(void);

eth_counter_t eth_rxcount(void);
eth_counter_t eth_txcount(void);
eth_counter_t eth_lostcount(void);