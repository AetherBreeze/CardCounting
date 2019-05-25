#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdint.h>

#define STRAIGHT_FLUSH_BIT_OFFSET 36
#define FOUR_OF_A_KIND_BIT_OFFSET 32
#define FULL_HOUSE_BIT_OFFSET 28
#define FLUSH_BIT_OFFSET 27
#define STRAIGHT_BIT_OFFSET 26
#define THREE_OF_A_KIND_BIT_OFFSET 22
#define TWO_PAIR_INDICATOR_BIT_OFFSET 21
#define PAIR_BIT_OFFSET 16

int bitwise_contains(uint64_t full_sequence, uint64_t contained_sequence)
{
    return ((full_sequence & contained_sequence) = contained_sequence) //bitwise AND removes all bits that aren't in the contained sequence
}

int count_binary_ones(uint64_t sequence)
{
    int ret = 0;
    for(int i = 0; i < 64; i++, sequence = sequence >> 1)
    {
        ret +=  sequence & 1; //add the last bit of the sequence, shift right by one, repeat until bits run out
    }
    
    return ret;
}

uint64_t get_n_highest_cards(uint64_t full_hand, int n, int start_index, int jump_size)
{
    uint64_t ret = 0;
    int card_count = 0;
    int bit_shift = start_index; //start at the highest card slot for the flush's suit
    int card_index = 15
    do
    {
        uint64_t extracted_bit = (temp_mask & (1 << bit_shift)) >> bit_shift;
        card_count += (temp_mask & (1 << bit_shift)) >> bit_shift; //the inner bit_shift gets the [bit_shift]th bit of [temp_mask], the outer bit_shift brings it to either zero or one
        ret += extracted_bit << card_index;
        bit_shift -= jump_size;
        card_index--;
    } while(card_count < n); //when we get the top 5 flush cards, return
    return (temp_mask >> bit_shift) << bit_shift //drop all the 1's below the selected top 5 cards and return
}

int straight_flush(uint64_t full_hand, int low_card)
{
    uint64_t relevant_section = full_hand >> (low_card - 1); //low_card will be 1 for an ace-low straight, i.e. 00000000011111; this cuts off the whole hand until the straight begins
    uint64_t indicator = 69905LL; //69905 = 2^0 + 2^4 + 2^8 + 2^12 + 2^16 = b10001000100010001
    for(int i = 0; i < 4; relevant_section = relevant_section >> 1, i++) //check all four suits, bit-shifting one to the right each time
    {
        if(bitwise_contains(relevant_section, indicator))
        {
            return 1;
        }
    }
    return 0; //if there isn't a straight flush of any suit, return false
}

uint64_t straight(uint64_t full_hand, uint16_t straight_hand)
{
    uint64_t mask = 0LL;
    int straight_flush = 0;
    uint64_t indicator = 31LL; //31 = 2^4 + 2^3 + 2^2 + 2^1 + 2^0 = b11111
    for(int i = 9; straight_hand > indicator; indicator = indicator << 1, i--) //[i] counts the lowest card in the straight
    {
        if(bitwise_contains((uint64_t)straight_hand, indicator))
        {
            int sf = straight_flush(full_hand, i); //check if this straight is also a flush
            if(!mask || !straight_flush && sf)) //if there's no straight yet, or if there is, but not a straight flush
            {
                mask = indicator; //store the list of 1's in the position corresponding to the flush cards
            }
        }
    }
    return (mask | (straight << STRAIGHT_BIT_OFFSET) | (straight_flush << STRAIGHT_FLUSH_BIT_OFFSET)); //create the score from a) the five cards and b) the straight flush indicator (seeing this makes the rater return immediately)
}

uint64_t flush(uint64_t full_hand)
{
    uint64_t mask = 0LL;
    uint64_t indicator = 38430716820228224LL; // = 2^7 + 2^11 + 2^15 + 2^19 + 2^23 + 2^27 + 2^31 + 2^35 + 2^39 + 2^43 + 2^47 + 2^51 + 2^55 = every 4th bit is 1 | we OMIT THE LAST BIT to avoid double-counting aces
    for(int i = 0; i < 4; indicator = indicator >> 1) //check all four suits, bit-shifting one to the right each time
    {
        uint64_t temp_mask = full_hand & indicator;
        int suit_count = count_binary_ones(temp_mask);
        if(suit_count > 4)
        {
            if(suit_count > 5) //this ensures that two people with identical flushes both win even if one had 6+ clubs
            {
                temp_mask = get_n_highest_cards(full_hand, 5, 55 - i, 4); //get the 5 highest cards of the flush, starting at the correct suit at the high aces (55-i), and jump one card level at a time
            }
            mask = temp_mask; //return the five flush cards, plus the flush bit indicator
            break; //no more than one flush can happen in 7 cards; once we find one, we're done
        }
    }
    return mask | (1 << FLUSH_BIT_OFFSET);
}

uint64_t multiples(uint64_t full_hand, uint64_t straight_hand)
{
    int high_pairs[2] = [0, 0];
    int high_three_of_a_kind= 0;
    int high_four_of_a_kind = 0;
    
    for(uint64_t hand = full_hand, int i = 15; hand > 2 ; hand = hand << 4, i--) //[i] keeps track of what card we're on, hand always has the [i]th card in the rightmost 4 bits | [i] starts at 2 so that pairs of aces are considered pairs of 14s, not pairs of 1s
    {
        int indicator = (int)((hand & 67553994410557440LL) >> 52); //= 2^55 + 2^54 + 2^53 + 2^52 = b1111000... | Bitshifting 52 brings the 4 relevant bits to the 4 lowest bits, so we don't have to do 11 comparisons to uint64_t's
        if(indicator > 2)
        {
            if(indicator == 3 || indicator == 5 || indicator == 6 || indicator == 9 || indicator == 10 || indicator == 12) //checking equality to 0011, 0101, 0110, 1001, 1010, 1100
            {
                if(!pairs[0]) //if there isn't a highest pair yet
                {
                    pairs[0] = i; //then this pair is highest pair
                }
                else if(pairs[0] && !pairs[1]) //if there's already a highest pair, but no second highest
                {
                    pairs[1] = i; //then this pair is second highest pair
                }
                continue; //no need to check for 3s or 4s if its definitely a pair
            }
            if((indicator == 7 || indicator == 11 || indicator == 13 || indicator == 14) && !high_three_of_a_kind) //checking equality to 0111, 1011, 1101, 1110
            {
                high_three_of_a_kind = i;
                continue; //no need to check for 4s if its definitely a 3
            }
            if(indicator == 15)
            {
                return (i << FOUR_OF_A_KIND_BIT_OFFSET) | (1 << i); //if we find a 4-of-a-kind, stop; there's no better multiples to be found  | return the index of the 4-of-a-kind and the indicator bit
            }
        }
    }
    
    if(high_three_of_a_kind) //if there's a three of a kind
    {
        if(high_pairs[0]) //and there's a seperate pair
        {
            return (high_three_of_a_kind << FULL_HOUSE_BIT_OFFSET) | (1 << high_three_of_a_kind) | (1 << high_pairs[0]); //return the top card in the full house in that indicator, and mark the cards involved
        }
        //otherwise
        uint64_t top_two_other_cards = get_n_highest_cards(straight_hand ^ (1 << high_three_of_a_kind), 2, 15, 1);
        return (high_three_of_a_kind << THREE_OF_A_KIND_BIT_OFFSET) | (1 << high_three_of_a_kind) | top_two_other_cards; //return the card in the three-of-a-kind, and mark the 3-kind card plus the two other highest cards that aren't the 3-kind card
    }
    //otherwise
    if(high_pairs[0]) //if there's a pair
    {
        if(high_pairs[1]) //and there's another pair
        {
            uint64_t top_one_other_card = get_n_highest_cards(straight_hand ^ (1 << high_pairs[0]) ^ (1 << high_pairs[1]), 1, 15, 1);
            return (1 << TWO_PAIR_INDICATOR_BIT_OFFSET) | ((high_pairs[0] + high_pairs[1]) << PAIR_BIT_OFFSET) | (1 << high_pairs[0]) | (1 << high_pairs[1]); //return the two pair bit, the sum of the two pairs in the pair bits, and mark the pair cards plus the one other highest card that isn't a pair card
        }
        //otherwise, if it's just one pair
        return (high_pairs[0] << PAIR_BIT_OFFSET) | (1 << high_pairs[0]); //return the pair card in the pair bits, and mark the card involved
    }
    //otherwise, if there are no pairs
    return get_n_highest_cards(straight_hand, 5, 15, 1); //we can iterate over straight_hand here, since no pairs means all relevant cards are distinct when classified by number
}

uint64_t rate_hand(uint64_t full_hand, uint16_t straight_hand)
{
    uint64_t straight_score = straight(full_hand, straight_hand);
    if(straight_score >> STRAIGHT_FLUSH_BIT_OFFSET) //if we found a straight flush (if this score has the straight_flush_bit set to 1)
    {
        return straight_score; //just return that; we won't find anything better
    }
    uint64_t flush_score = flush(full_hand);
    uint64_t multiple_score = multiples(full_hand, straight_hand);
    
    if((!flush_score && !straight_score) | multiple_score > (1 << FLUSH_BIT_OFFSET)) //if there's no flush, and no straight, OR the multiples are better than a flush (since we know there isn't a straight flush)
    {
        return multiple_score; //then the best hand consists of whatever multiples you had
    }
    if(flush_score) //otherwise, if there was a flush, (and we already know there wasn't a straight flush, and there weren't better multiples)
    {
        return flush_score; //then a flush was the best hand
    }                             
    return straight_score; //if multiples weren't the best hand, and there wasn't a flush, but we still got here, then there must have been a straight
}

static PyObject *rate_rateHand(PyObject* self, PyObject *args)
{
    uint64_t full_hand;
    uint16_t straight_hand;
    PyArg_ParseTuple(args, "KH", &full_hand, &straight_hand); //parse the two python arguments as unsigned LL and unsigned short respectively
    
    uint64_t straight_score = straight(full_hand, straight_hand);
    if(straight_score >> STRAIGHT_FLUSH_BIT_OFFSET) //if we found a straight flush (if this score has the straight_flush_bit set to 1)
    {
        return PyLong_FromUnsignedLongLong(straight_score); //just return that; we won't find anything better
    }
    uint64_t flush_score = flush(full_hand);
    uint64_t multiple_score = multiples(full_hand, straight_hand);
    
    if((!flush_score && !straight_score) | multiple_score > (1 << FLUSH_BIT_OFFSET)) //if there's no flush, and no straight, OR the multiples are better than a flush (since we know there isn't a straight flush)
    {
        return PyLong_FromUnsignedLongLong(multiple_score); //then the best hand consists of whatever multiples you had
    }
    if(flush_score) //otherwise, if there was a flush, (and we already know there wasn't a straight flush, and there weren't better multiples)
    {
        return PyLong_FromUnsignedLongLong(flush_score); //then a flush was the best hand
    }                             
    return PyLong_FromUnsignedLongLong(straight_score); //if multiples weren't the best hand, and there wasn't a flush, but we still got here, then there must have been a straight
}
