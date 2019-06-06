#include <stdint.h>

#define STRAIGHT_FLUSH_BIT_OFFSET 36
#define FOUR_OF_A_KIND_BIT_OFFSET 32
#define FULL_HOUSE_BIT_OFFSET 28
#define FLUSH_BIT_OFFSET 27
#define STRAIGHT_BIT_OFFSET 26
#define THREE_OF_A_KIND_BIT_OFFSET 22
#define TWO_PAIR_BIT_OFFSET 18
#define PAIR_BIT_OFFSET 14
#define TOP_HAND_BIT_OFFSET 13

int bitwise_contains(uint64_t full_sequence, uint64_t contained_sequence)
{
    return ((full_sequence & contained_sequence) == contained_sequence); //bitwise AND removes all bits that aren't in the contained sequence
}

uint16_t find_flush_of_suit(uint64_t sequence, int suit)
{
    uint16_t ret = 0;
    int found_so_far = 0;
    uint64_t scanner = (1LL << (52 + suit)); //start off checking the high aces section, shifted to the right to match the slot of this suit in the full-hand expression [spades][hearts][diamonds][clubs]
    for(int card = TOP_HAND_BIT_OFFSET; card > 0; card--, scanner = scanner >> 4) //start at 4 because low aces are stripped out by flush(), to avoid double counting (since aces  are stored as 10...01) | end at 60 so we bitshift left a total of 55 times, which is the highest index a card (AS) can be at | iterate backwards so that higher cards are earlier in the array
    {
        if(sequence & scanner) //if the bit we're scanning is one, that means we have this card in this [suit]
        {
            ret += (1 << card); //mark this card in the output
            found_so_far++; //add one to the count we've found
            if(found_so_far == 5) //since we're iterating from top cards downward, the first 5 flush cards we find will be the best
            {
                return ret;
            }
        }
    }

    return 0; //less than five cards of a flush = nothing
}

uint16_t get_n_highest_cards(uint16_t straight_hand, int n)
{
    uint16_t ret = 0;
    int card_count = 0;  //start at the highest card slot for the flush's suit
    int card_index = TOP_HAND_BIT_OFFSET;
    do
    {
        uint16_t extracted_bit = (straight_hand & (1 << card_index)) >> card_index; //the inner bit_shift gets the [bit_shift]th bit of [temp_mask], the outer bit_shift brings it to either zero or one
        card_count += extracted_bit;
        ret += (extracted_bit << card_index);
        card_index--;
    } while(card_count < n); //when we get the top 5 flush cards, return

    return ret;
}

int straight_flush(uint64_t full_hand, int low_card)
{
    for(int suit = 3; suit <= 0; suit--)
    {
        uint64_t relevant_section = full_hand >> (4*(low_card - 1) + suit); //low_card will be 1 for an ace_low straight, but the straight itself would start at 0
        if(bitwise_contains(relevant_section, 69905LL)) //69905 = 2^0 + 2^4 + 2^8 + 2^12 + 2^15 = b10001000100010001
        {
            return 1;
        }
    }
    return 0; //if we didn't find one of any suit, return FALSE
}

uint64_t straight(uint64_t full_hand, uint16_t straight_hand)
{
    uint64_t mask = 0LL;
    uint64_t indicator = 15872LL; //31 = 2^13 + 2^12 + 2^11 + 2^10 + 2^9 = b11111000000000
    for(int i = 9; straight_hand > indicator; indicator = indicator >> 1, i--) //[i] counts the lowest card in the straight
    {
        if(bitwise_contains((uint64_t)straight_hand, indicator))
        {
            int sf = straight_flush(full_hand, i); //check if this straight is also a flush
            if(!mask || sf) //if there's no straight yet, or if this is a straight flush
            {
                mask = indicator | (1 << STRAIGHT_BIT_OFFSET) | ((uint64_t)sf << STRAIGHT_FLUSH_BIT_OFFSET); //store the list of 1's in the position corresponding to the flush cards
	    	if(sf) //if this does happen to be a straight flush
                {
                    break; //quit looking now, nothing beats the highest available straight flush
                }
	    }
        }
    }
    return mask; //the score is created from a) the five cards, b) the straight indicator,  and c) the straight flush indicator (seeing this makes the rater return immediately)
}

uint64_t flush(uint64_t full_hand)
{
    uint64_t mask = 0LL;
    uint64_t indicator = 38430716820228224LL; // = 2^7 + 2^11 + 2^15 + 2^19 + 2^23 + 2^27 + 2^31 + 2^35 + 2^39 + 2^43 + 2^47 + 2^51 + 2^55 = every 4th bit is 1 | we OMIT THE LAST BIT to avoid double-counting aces
    for(int i = 3; i >= 0; indicator = indicator >> 1, i--) //check all four suits, bit-shifting one to the right each time
    {
        uint16_t temp = find_flush_of_suit((full_hand & indicator), i); //this returns the top 5 cards of a flush, if we have one in this suit
        if(temp) //if we found a flush
        {
            mask = temp + (1LL << FLUSH_BIT_OFFSET); //set the flush indicator bit, then return that plus the top five cards
            break; //there can't be more than 1 suit flush in seven cards
        }
    }
    return mask;
}

uint64_t multiples(uint64_t full_hand, uint64_t straight_hand)
{
    int high_pairs[2] = {0, 0};
    int high_three_of_a_kind= 0;
    int high_four_of_a_kind = 0;
    int i = TOP_HAND_BIT_OFFSET;

    for(uint64_t hand = full_hand; hand > 2 ; hand = hand << 4, i--) //[i] keeps track of what card we're on, hand always has the [i]th card in the rightmost 4 bits | [i] starts at 2 so that pairs of aces are considered pairs of 14s, not pairs of 1s
    {
        int indicator = (int)((hand & 67553994410557440LL) >> 52); //= 2^55 + 2^54 + 2^53 + 2^52 = b1111000... | Bitshifting 52 brings the 4 relevant bits to the 4 lowest bits, so we don't have to do 11 comparisons to uint64_t's
        if(indicator > 2)
        {
            if(indicator == 3 || indicator == 5 || indicator == 6 || indicator == 9 || indicator == 10 || indicator == 12) //checking equality to 0011, 0101, 0110, 1001, 1010, 1100
            {
                if(!high_pairs[0]) //if there isn't a highest pair yet
                {
                    high_pairs[0] = i; //then this pair is highest pair | +1 because [i] is one below the card's rank, i.e. 4 is stored in 2^3
                }
                else if(high_pairs[0] && !high_pairs[1]) //if there's already a highest pair, but no second highest
                {
                    high_pairs[1] = i; //then this pair is second highest pair | +1 because [i] is 1 below the card's rank, i.e. 12 is stored in 2^11
                }
                continue; //no need to check for 3s or 4s if its definitely a pair
            }
            if((indicator == 7 || indicator == 11 || indicator == 13 || indicator == 14) && !high_three_of_a_kind) //checking equality to 0111, 1011, 1101, 1110
            {
                high_three_of_a_kind = i; //plus one, because the index of a card is one below its rank (i.e., 2s are stored in 2^1)
                continue; //no need to check for 4s if its definitely a 3
            }
            if(indicator == 15)
            {
                return ((uint64_t)(i+1LL) << FOUR_OF_A_KIND_BIT_OFFSET) | (1 << i); //if we find a 4-of-a-kind, stop; there's no better multiples to be found  | return the index of the 4-of-a-kind and the indicator bit | +1 because [i] is one below the card's rank, i.e. 7s are stored in 2^6
            }
        }
    }

    if(high_three_of_a_kind) //if there's a three of a kind
    {
        if(high_pairs[0]) //and there's a seperate pair
        {
            return ((high_three_of_a_kind+1) << FULL_HOUSE_BIT_OFFSET) | (1 << high_three_of_a_kind) | (1 << high_pairs[0]); //return the top card in the full house in that indicator, and mark the cards involved
        }
        //otherwise
        uint64_t top_two_other_cards = get_n_highest_cards(straight_hand ^ (1 << high_three_of_a_kind), 2);
        return ((high_three_of_a_kind+1) << THREE_OF_A_KIND_BIT_OFFSET) | (1 << high_three_of_a_kind) | top_two_other_cards; //return the card in the three-of-a-kind, and mark the 3-kind card plus the two other highest cards that aren't the 3-kind card
    }
    //otherwise
    if(high_pairs[0]) //if there's a pair
    {
        if(high_pairs[1]) //and there's another pair
        {
            uint64_t top_one_other_card = get_n_highest_cards(straight_hand ^ (1 << high_pairs[0]) ^ (1 << high_pairs[1]), 1);
            return ((high_pairs[0] + 1) << TWO_PAIR_BIT_OFFSET) | ((high_pairs[1] + 1) << PAIR_BIT_OFFSET) | (1 << high_pairs[0]) | (1 << high_pairs[1]) | top_one_other_card; //return the higher pair in the two pair bits, the lower pair in the one pair bit, and mark the pair cards plus the one other highest card that isn't a pair card
        }
        //otherwise, if it's just one pair
        uint64_t top_three_other_cards = get_n_highest_cards(straight_hand ^ (1 << high_pairs[0]), 3);
        return ((high_pairs[0]+1) << PAIR_BIT_OFFSET) | (1 << high_pairs[0]) | top_three_other_cards; //return the pair card in the pair bits, and mark the card involved plus the top three other cards that aren't involved in the pair
    }
    //otherwise, if there are no pairs
    return get_n_highest_cards(straight_hand, 5); //we can iterate over straight_hand here, since no pairs means all relevant cards are distinct when classified by number
}

uint64_t rate_hand(uint64_t full_hand, uint16_t straight_hand)
{
    uint64_t straight_score = straight(full_hand, straight_hand);
    if(straight_score & (1LL << STRAIGHT_FLUSH_BIT_OFFSET)) //if the straight happened to be a straight flush
    {
        return straight_score; //then just return it
    }
    uint64_t flush_score = flush(full_hand);
    uint64_t multiple_score = multiples(full_hand, straight_hand);

    if((!flush_score && !straight_score) || multiple_score > (1 << FLUSH_BIT_OFFSET)) //if there's no flush, and no straight, OR the multiples are better than a flush (since we know there isn't a straight flush)
    {
        return multiple_score; //then the best hand consists of whatever multiples you had
    }
    if(flush_score) //otherwise, if there was a flush, (and we already know there wasn't a straight flush, and there weren't better multiples)
    {
        return flush_score; //then a flush was the best hand
    }
    return straight_score; //if multiples weren't the best hand, and there wasn't a flush, but we still got here, then there must have been a straight
}
