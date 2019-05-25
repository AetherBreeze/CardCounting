#ifndef COMPARISONHEADER
#define COMPARISONHEADER

#define STRAIGHT_FLUSH_BIT_OFFSET 36
#define FOUR_OF_A_KIND_BIT_OFFSET 32
#define FULL_HOUSE_BIT_OFFSET 28
#define FLUSH_BIT_OFFSET 27
#define STRAIGHT_BIT_OFFSET 26
#define THREE_OF_A_KIND_BIT_OFFSET 22
#define TWO_PAIR_INDICATOR_BIT_OFFSET 21
#define PAIR_BIT_OFFSET 16

int bitwise_contains(uint64_t full_sequence, uint64_t contained_sequence);

int count_binary_ones(uint64_t sequence);

uint64_t get_n_highest_cards(uint64_t full_hand, int n, int start_index, int jump_size);

int straight_flush(uint64_t full_hand, int low_card);

uint64_t straight(uint64_t full_hand, uint16_t straight_hand);

uint64_t flush(uint64_t full_hand);

uint64_t multiples(uint64_t full_hand, uint64_t straight_hand);

uint64_t rate_hand(uint64_t full_hand, uint16_t straight_hand);
#endif
