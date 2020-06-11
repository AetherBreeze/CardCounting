# Hand comparison: in detail

StackedDeck achieves its blazing comparison speeds by representing poker hands internally as integers. Each integer hand is assigned three
scores: its score as a straight, its score as a flush, and its score as multiples. These are computed with a series of bit-shifting
operations. Finally, the highest of the three scores is returned as an integer, in such a way that better hands always have numerically
higher scores.

To understand the algorithm, we'll break it down into three parts:
  1) [How hands are represented in memory](#how-stackeddeck-represents-hands),
  2) [How hands are checked for flushes, straights, and multiples](#checking-for-flushes-straights-and-multiples),
  3) How scores are computed
  
## How StackedDeck represents hands

Each **7-card hand** is represented as a **64-bit integer** (`uint64`), of which 56 bits are used. The general hand format is this:

![in descending value, spades-hearts-diamonds-clubs](https://i.imgur.com/hRp2Imc.png)

Cards are stored in ascending order of value, and cards of the same value are stored in ascending alphabetical order of suit. Aces are
stored at both the top and the bottom of the hand; this helps us check quickly for straights. No distinction is made between private (hand)
cards and public (table) cards.

For example, the following hand:

![5h5s-KsQh7s6h2h](https://i.imgur.com/ymhjIBq.png)

would be stored internally as this:

![00001000010000000000000000001000010011000000000001000000](https://i.imgur.com/k0PXedh.png)

The observant reader might begin to notice some patterns that could be extracted with bitmasks. We'll be doing that very soon, in...

## Checking for flushes, straights, and multiples

Broadly, the valuable hands in poker can be grouped into straights, flushes, multiples, and combinations of those. We check for each type
in turn.

### Straights

Straights are the easiest type of hand to check for. Simply, we'll be looking for five cards in a row.

So, how do we check if a hand has a card? We just take the bitwise AND of the hand with `1111b,`, shifted left such that the four 1s
are aligned with section of the hand that has the card we're looking for. If that doesn't make sense, no problem; we'll work through 
an example.

Let's take the hand from above:

![00001000010000000000000000001000010011000000000001000000](https://i.imgur.com/ecRHF64.png)

We'll start our search at the high aces. The logic is that, if a hand has multiple straights, it should be scored based on the best one --
the one with the highest high card. By starting our search at the top, we only have to store the first straight we find (though we'll still
check for others, in case they're straight flushes).

Shifting `1111b` to the left to line it up with the high aces and taking the AND, we get:

![and that returns 0](https://i.imgur.com/rEnafs3.png)
