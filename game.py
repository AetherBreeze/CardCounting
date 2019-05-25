# -*- coding: utf-8 -*-
from constants import suits, suit_ns, numbers
from random import shuffle

import ctypes
compare = ctypes.CDLL("./compare.so")
compare.rate_hand.argtypes = [ctypes.c_ulonglong, ctypes.c_ushort]

class Card:
    def __init__(self, s, n):
        self.suit = suits[s]
        self.suitN = s
        self.number = n
        self.value_straight = (1 << (n-1)) #value_straight is a 1 in the [number]th 
        self.value_full = (1 << (4*(n-1) + s))
        if(n == 14): #if the card is an ace
            self.value_straight += 1 #aces are represented as 100000000000001 for straight-checking purposes
            self.value_full += 1 << self.suitN #the ace bit block is [spades ace][hearts ace][diamonds ace][clubs ace]

class Player:
    def __init__(self, t, starting_worth):
        self.table = t
        self.hand = None
        self.worth = starting_worth
        self.still_in = True
        self.amount_bet_this_round = 0

    def net_worth(self):
        riches = 0
        for chip_value, chip_count in self.chips:
            riches += chip_value * chip_count
        return riches

    def can_match(self, amount):
        riches = 0
        for chip_value, chip_count in self.chips:
            riches += chip_value * chip_count
            if(riches >= amount):
                return True
        return False

    def fold(self):
        self.still_in = False

    def bet(self, amount):
        if self.worth <= amount:
            return self.all_in()
        else:
            self.worth -= amount
            return amount

    def all_in(self):
        ret = self.worth
        self.worth = 0
        return ret

    def rank_hand(self):
        full_hand = 0
        straight_hand = 0
        for cards in [self.hand, self.table.table_cards]:
            for card in cards:
                full_hand = full_hand | card.value_full #this adds the card's full_hand bit to full_hand
                straight_hand = straight_hand | card.value_straight #this adds the card's number bit to straight_hand, if it isn't there already
        return compare.rate_hand(full_hand, straight_hand)


    def take_action(self):
        #YOUR CODE HERE
        print("\n")
        for card in self.hand:
            print("{} of {}s\t".format(card.number, card.suit))
        return 0


class Table:
    def __init__(self, pc, chip_levels, starting_worth):
        self.dealer_index = 0
        self.amount_on_table = 0
        self.players = []
        if(pc > 23):
            raise("[ERROR] Deck does not have enough cards to support >23 players!")
        for i in range(pc):
            self.players.append(Player(self, starting_worth))
        self.players_in = len(self.players)
        self.chip_levels = chip_levels
        self.chip_levels.sort()
        self.blinds = [self.chip_levels[0], self.chip_levels[1]]

        self.deck = []
        for suit in suit_ns:
            for number in numbers:
                self.deck.append(Card(suit, number))

    def play_games(self, n):
        for i in range(n):
            self.play_round()
            self.determine_winner()
            for player in self.players: #stop the games when someone runs out of money
                if not player.worth:
                    return

    def play_round(self):
        card_index = 0
        self.amount_on_table = 0
        self.table_cards = []

        shuffle(self.deck) #the deal
        for player in self.players:
            player.hand = [self.deck[card_index], self.deck[card_index+1]]
            card_index += 2

        self.players[(self.dealer_index+1) % len(self.players)].bet(self.chip_levels[0]) #the blinds
        self.players[(self.dealer_index+2) % len(self.players)].bet(self.chip_levels[1])
        self.player_actions()

        if self.players_in == 1:
            return
        self.table_cards = [self.deck[card_index+1], self.deck[card_index+2], self.deck[card_index+3]] #the flop
        self.player_actions()

        if self.players_in == 1:
            return
        self.table_cards.append(self.deck[card_index+4]) #the turn
        self.player_actions()

        if self.players_in == 1:
            return
        self.table_cards.append(self.deck[card_index+5]) #the river
        self.player_actions()
        return

    def determine_winner(self):
        best_score = (0, 0)

        #TEMPORARY
        print("\n")
        for card in self.table_cards:
            print("{} of {}s\t".format(card.number, card.suit))
        print("\n")

        for player in self.players:
            if player.still_in:
                print("ranking hand for {}".format(player))
                score = player.rank_hand()
                print("{}\n".format(score))
                if score > best_score[1]:
                    best_score = (player, score)

        best_score[0].worth += self.amount_on_table

    def player_actions(self):
        if self.players_in == 1:
            return
        actions_since_last_raise = 0
        amount_added = 0
        player_acting = 0
        while player_acting != -1:
            bet = self.players[player_acting].take_action()
            self.amount_on_table += bet
            self.players[player_acting].amount_bet_this_round += bet
            if bet > amount_added: #players that've folded handle it themselves
                amount_added = bet
                actions_since_last_raise = 0
            else:
                actions_since_last_raise += 1
                
            if actions_since_last_raise == len(self.players):
                return
            player_acting = (player_acting + 1) % len(self.players)
            
def main():
    table = Table(4, [5, 10, 20, 50], 500)
    table.play_games(1)

main()
