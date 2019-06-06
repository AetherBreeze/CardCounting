#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

void generate_full_hand(int ran)
{
    //int ran = (rand() % 52) + 4;
    uint64_t card = (1LL << ran);
    uint64_t card2 = (1LL << ran);
    printf("%d | %llu | %llu\n", ran, card, card2);
}

uint64_t generate_hand()
{
    uint64_t ret = 0;
    int cards_added = 0;
    while(cards_added < 7)
    {
        int r = (rand() % 52) + 4;
        uint64_t card = (1LL << r); //generate random number from 0 to 51, then add four (since we add low aces manually)
        printf("rand: %d\n", r);
        printf("card: %llu\n", card);
        printf("what card should be: %llu\n", (uint64_t)(1LL << r));
        if(!(ret & card))
        {
	    ret += card;
            if(card > (1LL << 51)) //if its a high ace
            {
		ret += (card >> 52); //add the low ace marker as well
            }
            cards_added++;
        }
    };

    return ret;
}

uint16_t generate_straight_hand(uint64_t full_hand)
{
    uint16_t ret = 0;
    uint64_t indicator = 15LL; //2^0 + 2^1 + 2^2 + 2^3

    for(int i = 1; i < 14; i++)
    {
        if((full_hand >> (4*i)) & indicator)
        {
            ret += (1 << i);
        }
    }
    return ret;
}

int main()
{
    for(int i = 0; i < 64; i++)
    {
        generate_hand();
    }
    /*uint64_t fh_1 = generate_hand();
    uint16_t sh_1 = generate_straight_hand(fh_1);
    uint64_t fh_2 = generate_hand();
    uint16_t sh_2 = generate_straight_hand(fh_2);
    printf("%lu | %d\n", fh_1, sh_1);
    printf("%lu | %d\n", fh_2, sh_2);
    clock_t total_time = 0.0;
    for(int i = 0; i < 100000; i++)
    {
        clock_t start_time = clock(), diff;

        total_time += (clock() - start_time);
    }*/
    return 0;
}
