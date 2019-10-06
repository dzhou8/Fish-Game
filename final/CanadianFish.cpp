/*
ID: dzhouwave
PROG: Canadian Fish
LANG: C++11
*/

// @BEGIN_OF_SOURCE_CODE

#include <iostream>
#include <random>
#include <vector>
#include <time.h>
#include <iterator>
#include <conio.h>

using namespace std;

const int SUIT_MAX(4);
const int NUMBER_MAX(12);
const int NUM_PLAYERS(6);

const char SUIT[SUIT_MAX] = { 'C', 'D', 'H', 'S' };
const string SUIT_NAME[SUIT_MAX] = { "Clubs", "Diamonds", "Hearts", "Spades" };
const char NUMBER[NUMBER_MAX] = { '2', '3', '4', '5', '6', '7', '9', 'T', 'J', 'Q', 'K', 'A' };

//quality of life functions that make it easier to convert from (identification) numbers to (card) strings
string getCardString(int cardID)
{
	string result = "";
	result += NUMBER[cardID % NUMBER_MAX];
	result += SUIT[cardID / NUMBER_MAX];
	return result;
}
int getCardID(string cardString)
{
	int result = 0;
	for (int i = 0; i < SUIT_MAX; i++)
	{
		if (cardString[1] == SUIT[i])
		{
			result += i * NUMBER_MAX;
		}
	}
	for (int i = 0; i < NUMBER_MAX; i++)
	{
		if (cardString[0] == NUMBER[i])
		{
			result += i;
		}
	}
	return result;
}
void printCard(int cardID)
{
	cout << getCardString(cardID) << " ";
}
int getHalfSuit(int cardID) 
{
	return 2 * cardID / NUMBER_MAX;
}

//forward declaration
struct Game;

struct Player
{
	Game* g; //pointer to the common Game object

	string name;
	bool cards[SUIT_MAX * NUMBER_MAX]; //true if this player possesses the card of ID [i]
	int ID;
	int teamID; //either team 1 or team 2
	int numCards;

	Player()
	{

	}

	Player(string name, Game* g)
	{
		this->name = name;
		for (int i = 0; i < SUIT_MAX * NUMBER_MAX; i++)
		{
			cards[i] = false;
		}
		this->g = g;
		numCards = 0;
	}

	bool hasCard(int cardID)
	{
		return cards[cardID];
	}

	void takeCard(int cardID)
	{
		cards[cardID] = true;
		numCards++;
	}

	void giveCard(int cardID)
	{
		cards[cardID] = false;
		numCards--;
	}

	int getNumberInHalfSuit(int halfSuit)
	{
		int cnt = 0;
		for (int i = halfSuit * NUMBER_MAX / 2; i < (halfSuit + 1) * NUMBER_MAX / 2; i++)
		{
			if (cards[i] == true)
			{
				cnt++;
			}
		}
		return cnt;
	}

	//get half suits you have (warning: will also return halfSuits you have all the cards of)
	vector<int> getAvailableHalfSuits() // 0 is low club, 1 is high club, 2 is low diamond, etc...
	{
		vector<int> result;
		for (int i = 0; i < SUIT_MAX * 2; i++)
		{
			int cnt = getNumberInHalfSuit(i);
			if (cnt > 0)
			{
				result.push_back(i);
			}
		}
		return result;
	}

	//get cards you can ask for in a halfSuit
	vector<int> getAvailableCards(int halfSuit)
	{
		vector<int> result;
		for (int i = halfSuit * NUMBER_MAX / 2; i < (halfSuit + 1) * NUMBER_MAX / 2; i++)
		{
			if (cards[i] == false)
			{
				result.push_back(i);
			}
		}
		return result;
	}

	//get all cards you can ask for
	vector<int> getAvailableCards()
	{
		vector<int> result;
		vector<int> halfSuits = getAvailableHalfSuits();
		for (int i = 0; i < halfSuits.size(); i++)
		{
			vector<int> availableCards = getAvailableCards(halfSuits[i]);
			result.insert(result.end(), availableCards.begin(), availableCards.end());
		}
		return result;
	}

	vector<int> getTeammates()
	{
		vector<int> result;
		if (teamID == 1)
		{
			for (int i = 0; i < NUM_PLAYERS / 2; i++)
			{
				if (i != ID)
				{
					result.push_back(i);
				}
			}
		}
		else
		{
			for (int i = NUM_PLAYERS / 2; i < NUM_PLAYERS; i++)
			{
				if (i != ID)
				{
					result.push_back(i);
				}
			}
		}
		return result;
	}

	/*get index of opponents who still have cards
	requires external declaration*/
	vector<int> getAvailableOpponents();

	void print()
	{
		cout << name << " has the following cards: " << endl;
		for (int i = 0; i < SUIT_MAX * NUMBER_MAX; i++)
		{
			if (cards[i] == true)
			{
				printCard(i);
			}
		}
		cout << endl;
	}

	virtual void takeTurn() //ABSTRACT - SHOULD NEVER BE CALLED
	{

	}

	virtual void endGame() //ABSTRACT - SHOULD NEVER BE CALLED
	{

	}
};

struct Game
{
	Player* players[NUM_PLAYERS];
	vector<int> cardLocation; //lookup vector for who has a card --> id of the player, or -1 if its not in play anymore
	int turnID;
	int score[SUIT_MAX * 2];
	int endGameTeam;

	vector <pair<vector<int>, vector<int> > > history; 
	/*
	History is essentially datakeeping what the game state has been previously
	only useful for thinkingPlayer

	history[i] is the game state at move i
		history[i].first is cardLocation
		history[i].second is the move that was enacted
		history[i].second[0] is the person whose turn it was
		history[i].second[1] is the type of move, 0 is asking for card, 1 is declaring fish
			0 - asking for card
				history[i].second[2] = the person they asked
				history[i].second[3] = the card they asked for
				history[i].second[4] = whether or not the move was successful - not really necessary
			1 - declaring fish
				history[i].second[2] = the halfSuit id they declared fish on
				history[i].second[3-8] = the distribution they declared
				history[i].second[9] = whether or not the move was successful - not really necessary
	*/

	Game()
	{

	}

	void setPlayers(Player* players[NUM_PLAYERS])
	{
		for (int i = 0; i < NUM_PLAYERS; i++)
		{
			this->players[i] = players[i];
			this->players[i]->ID = i;
			if (i < NUM_PLAYERS / 2)
			{
				this->players[i]->teamID = 1;
			}
			else
			{
				this->players[i]->teamID = 2;
			}
		}
		turnID = rand() % NUM_PLAYERS;
		for (int i = 0; i < SUIT_MAX * 2; i++)
		{
			score[i] = 0;
		}
	}

	void DealCards()
	{
		cardLocation.resize(SUIT_MAX * NUMBER_MAX);
		vector<int> cards;
		cards.resize(SUIT_MAX * NUMBER_MAX);
		for (int i = 0; i < cards.size(); i++)
		{
			cards[i] = i;
		}
		random_shuffle(cards.begin(), cards.end());
		for (int i = 0; i < cards.size(); i++) //distribute to team 1
		{
			players[i * NUM_PLAYERS / cards.size()]->takeCard(cards[i]);
			cardLocation[cards[i]] = i * NUM_PLAYERS / cards.size();
		}

		vector<int> blank;
		history.push_back(make_pair(cardLocation, blank));
	}

	//global game event in which a player (on their turn) decides to declare Fish
	void declareFish(int halfSuit, vector<int> distribution)
	{
		cout << players[turnID]->name << " declares Fish on the " << ((halfSuit % 2 == 0) ? " Low " : " High ") << SUIT_NAME[halfSuit / 2] << endl;
		bool isCorrect = true;
		for (int i = halfSuit * NUMBER_MAX / 2; i < (halfSuit + 1) * NUMBER_MAX / 2; i++)
		{
			cout << players[turnID]->name << " declares that " << players[distribution[i - halfSuit * NUMBER_MAX / 2]]->name << " has " << getCardString(i) << endl;
			if (cardLocation[i] == distribution[i - halfSuit * NUMBER_MAX / 2])
			{
				cout << "This is correct!" << endl;
			}
			else
			{
				cout << "This is incorrect. " << players[cardLocation[i]]->name << " has " << getCardString(i) << endl;
				isCorrect = false;
			}
			players[cardLocation[i]]->giveCard(i);
			cardLocation[i] = -1;
		}
		if (isCorrect)
		{
			cout << "The declaration was correct, so Team " << players[turnID]->teamID << " wins the half-suit." << endl;
			score[halfSuit] = players[turnID]->teamID;
		}
		else
		{
			cout << "The declaration was incorrect, so Team " << 3 - players[turnID]->teamID << " wins the half-suit." << endl;
			score[halfSuit] = 3 - players[turnID]->teamID;
		}

		//add this move to history
		pair<vector<int>, vector<int>> thisHistory;
		thisHistory.first = cardLocation;
		thisHistory.second.push_back(turnID);
		thisHistory.second.push_back(1);
		thisHistory.second.push_back(halfSuit);
		for (int i = 0; i < distribution.size(); i++)
		{
			thisHistory.second.push_back(distribution[i]);
		}
		thisHistory.second.push_back(isCorrect);
	}

	//global game event in which a player (on their turn) asks another player for a card
	void processTurn(int targetPlayer, int targetCard)
	{
		cout << players[turnID]->name << " is asking " << players[targetPlayer]->name << " for the " << getCardString(targetCard) << endl;
		if (players[targetPlayer]->hasCard(targetCard))
		{
			players[turnID]->takeCard(targetCard);
			players[targetPlayer]->giveCard(targetCard);
			cout << players[targetPlayer]->name << " gave " << players[turnID]->name << " the " << getCardString(targetCard) << endl;
			cardLocation[targetCard] = turnID;

			//add this move to history
			pair<vector<int>, vector<int>> thisHistory;
			thisHistory.first = cardLocation;
			thisHistory.second.push_back(turnID);
			thisHistory.second.push_back(0);
			thisHistory.second.push_back(targetPlayer);
			thisHistory.second.push_back(targetCard);
			thisHistory.second.push_back(true);
			history.push_back(thisHistory);
		}
		else
		{
			cout << players[targetPlayer]->name << " doesn't have the " << getCardString(targetCard) << endl;

			//add this move to history
			pair<vector<int>, vector<int>> thisHistory;
			thisHistory.first = cardLocation;
			thisHistory.second.push_back(turnID);
			thisHistory.second.push_back(0);
			thisHistory.second.push_back(targetPlayer);
			thisHistory.second.push_back(targetCard);
			thisHistory.second.push_back(false);
			history.push_back(thisHistory);

			turnID = targetPlayer;
		}
	}

	void takeTurn()
	{
		system("CLS");
		print();

		while (players[turnID]->numCards == 0) //no cards --> turn goes to clockwise teammate
		{
			cout << players[turnID]->name << " has run out of cards, so the turn automatically passes to ";
			if ((turnID + 1) % (NUM_PLAYERS / 2) == 0)
			{
				turnID++;
				turnID -= NUM_PLAYERS / 2;
			}
			else
			{
				turnID++;
			}
			cout << players[turnID]->name << "." << endl;
		}
		cout << "It's " << players[turnID]->name << "'s turn." << endl;

		players[turnID]->takeTurn();

		_getch();
	}

	void print()
	{
		int teamOneScore = 0;
		int teamTwoScore = 0;
		for (int i = 0; i < SUIT_MAX * 2; i++)
		{
			if (score[i] == 1)
			{
				teamOneScore++;
			}
			else if(score[i] == 2)
			{
				teamTwoScore++;
			}
		}
		cout << "Score: " << teamOneScore << " to " << teamTwoScore << endl;
		for (int i = 0; i < NUM_PLAYERS; i++)
		{
			cout << players[i]->name << " has " << players[i]->numCards << " cards." << endl;
		}
		cout << endl;
		players[0]->print();
		cout << endl;
	}

	bool finished()
	{
		for (int i = 0; i < SUIT_MAX * 2; i++)
		{
			if (score[i] == 0)
			{
				return false;
			}
		}
		return true;
	}

	void printResult()
	{
		_getch();
		system("CLS");
		int teamOneScore = 0;
		int teamTwoScore = 0;
		for (int i = 0; i < SUIT_MAX * 2; i++)
		{
			if (score[i] == 1)
			{
				teamOneScore++;
			}
			else
			{
				teamTwoScore++;
			}
		}
		if (teamOneScore >= 5)
		{
			cout << "Team 1 wins " << teamOneScore << " to " << teamTwoScore << endl;
		}
		else
		{
			cout << "Team 2 wins " << teamTwoScore << " to " << teamOneScore << endl;
		}
	}

	//get halfSuits that are still in the Game (playable by everyone) (fish has not been declared yet)
	vector<int> getPlayableHalfSuits()
	{
		vector<int> result;
		for (int i = 0; i < NUMBER_MAX * 2; i++)
		{
			if (score[i] == 0)
			{
				result.push_back(i);
			}
		}
		return result;
	}

	//return if one team is completely out of cards --> go to endgame, where the other team must declare Fish without any more information
	bool oneTeamEmpty()
	{
		bool teamOneCards = false;
		bool teamTwoCards = false;
		for (int i = 0; i < SUIT_MAX * NUMBER_MAX; i++)
		{
			if (cardLocation[i] != -1)
			{
				if (cardLocation[i] < NUM_PLAYERS / 2)
				{
					teamOneCards = true;
				}
				else
				{
					teamTwoCards = true;
				}
			}
		}
		if (teamOneCards == false && teamTwoCards == false) //this shouldn't ever happen... If they are both false, then the game should have been finished
		{
			return false;
		}
		else if (teamOneCards == false)
		{
			endGameTeam = 2;
			return true;
		}
		else if (teamTwoCards == false)
		{
			endGameTeam = 1;
			return true;
		}
		else
		{
			return false;
		}
	}

	void endGame()
	{
		system("CLS");
		print();
		cout << "Team " << 3-endGameTeam << " has run out of cards, so now Team " << endGameTeam << " must declare fish." << endl;
		if (endGameTeam == 1) //human player's team has to endgame
		{
			turnID = 0;
			players[0]->endGame();
		}
		else
		{
			int highestIndex = -1;
			int highestCards = 0;
			for (int i = NUM_PLAYERS / 2; i < NUM_PLAYERS; i++)
			{
				if (players[i]->numCards > highestCards)
				{
					highestIndex = i;
					highestCards = players[i]->numCards;
				}
			}
			turnID = highestIndex;
			players[highestIndex]->endGame();
		}
	}
};

//get index of opponents you can still ask for cards
vector<int> Player::getAvailableOpponents()
{
	vector<int> result;
	for (int i = 0; i < NUM_PLAYERS; i++)
	{
		if (teamID != g->players[i]->teamID
			&& (g->players[i]->numCards > 0))
		{
			result.push_back(i);
		}
	}
	return result;
}

/*
A type of computer player I constructed that simply asks at random.

Asking policy: Random player & random card
Fish policy: It will declare fish when it has all the cards of a halfSuit.
Endgame policy: It will randomly declare a halfSuit and will randomly declare which teammate has the cards.
*/
struct RandomPlayer : Player
{
	RandomPlayer(string name, Game* g) : Player(name, g) {};

	bool fishCheck()
	{
		vector<int> halfSuits = getAvailableHalfSuits();
		for (int halfSuit : halfSuits)
		{
			int cnt = getNumberInHalfSuit(halfSuit);
			vector<int> distribution;
			if (cnt == NUMBER_MAX / 2)
			{
				for (int j = 0; j < NUMBER_MAX / 2; j++)
				{
					distribution.push_back(ID);
				}
				g->declareFish(halfSuit, distribution);
				return true;
			}
		}
		return false;
	}

	void takeTurn()
	{
		if (fishCheck())
		{
			return;
		}
		//random bot
		vector<int> availableOpponents = getAvailableOpponents();
		int askingID = availableOpponents[rand() % availableOpponents.size()];
		vector<int> availableCards = getAvailableCards();
		int askingCard = availableCards[rand() % availableCards.size()];
		g->processTurn(askingID, askingCard);
	}

	void endGame() //this player is forced to call a fish because it is the end of the game
	{
		vector<int> halfSuits = g->getPlayableHalfSuits();
		int halfSuit = halfSuits[rand() % halfSuits.size()];
		vector<int> teammates = getTeammates();
		vector<int> teammateCardDistribution;
		for (int i = 0; i < teammates.size(); i++)
		{
			for (int j = 0; j < g->players[teammates[i]]->numCards; j++)
			{
				teammateCardDistribution.push_back(teammates[i]);
			}
		}

		vector<int> distribution;
		for (int i = halfSuit * NUMBER_MAX / 2; i < (halfSuit + 1) * NUMBER_MAX / 2; i++)
		{
			if (cards[i] == true) // i have it
			{
				distribution.push_back(ID);
			}
			else
			{
				distribution.push_back(teammateCardDistribution[rand() % teammateCardDistribution.size()]);
			}
		}
		g->declareFish(halfSuit, distribution);
	}
};

/*
A computer player I primarily created to help me debug.
It "cheats" in the fact that it accesses Game.cardLocations to get its information.

Asking Policy: It will find the location of a card it can legally ask for (they have a card in the halfSuit && an opponent has the card).
It will have a 100% hit rate.
Fish Policy: It will declare fish if the entire team has all cards of a halfSuit between them
It has a 100% success rate.
Endgame Policy: It will declare fish correctly. (logically given that all the cards are within the team)
It has a 100% success rate.
*/
struct CheatingPlayer : Player
{
	CheatingPlayer(string name, Game* g) : Player(name, g) {};

	bool fishCheck()
	{
		for (int i = 0; i < SUIT_MAX * 2; i++)
		{
			vector<int> distribution;
			for (int j = i * NUMBER_MAX / 2; j < (i + 1) * NUMBER_MAX / 2; j++)
			{
				if (g->cardLocation[j] != -1
					&& teamID == g->players[g->cardLocation[j]]->teamID) //the person who has this card is on my team
				{
					distribution.push_back(g->cardLocation[j]);
				}
			}
			if (distribution.size() == NUMBER_MAX / 2)
			{
				g->declareFish(i, distribution);
				return true;
			}
		}
		return false;
	}

	void takeTurn()
	{
		if (fishCheck())
		{
			return;
		}
		vector<int> availableOpponents = getAvailableOpponents();
		int askingID = availableOpponents[rand() % availableOpponents.size()];
		vector<int> availableCards = getAvailableCards();
		int askingCard = availableCards[rand() % availableCards.size()];
		for (int i = 0; i < availableCards.size(); i++)
		{
			if (teamID != g->players[g->cardLocation[availableCards[i]]]->teamID)
			{
				askingID = g->cardLocation[availableCards[i]];
				askingCard = availableCards[i];
			}
		}
		g->processTurn(askingID, askingCard);
	}

	void endGame() //this player is "forced" to call fish because it is the end of the game
	{
		for (int i = 0; i < SUIT_MAX * 2; i++)
		{
			if (g->score[i] == 0) //we can still win this halfsuit
			{
				vector<int> distribution;
				for (int j = i * NUMBER_MAX / 2; j < (i + 1) * NUMBER_MAX / 2; j++)
				{
					distribution.push_back(g->cardLocation[j]);
				}
				g->declareFish(i, distribution);
			}
		}
	}
};

/*
Struct for a player that takes user input.
Quite a complex struct, and doesn't have a "policy" like the computer players do
Uses dynamic input (_getch()) meaning the user presses buttons to interact

Just run the program until it is "Player"'s turn and see how it works*/
struct HumanPlayer : Player
{
	int menuOption; //0 is person/fish selection, 1 is fish-halfsuit selection, 2 fish-card selection, 3 is asking-halfsuit selection, 4 is asking-card selection
	int currentSelection;
	int halfSuitSelection;
	int askSelection;
	vector<int> distribution;
	HumanPlayer(string name, Game* g) : Player(name, g) {};

	void takeTurn()
	{
		menuOption = 0;
		currentSelection = 0;
		distribution.clear();
		takeTurnUtil();
	}

	void takeTurnUtil()
	{
		printInfo();
		if (menuOption == 0) //person/fish selection
		{
			cout << "What should you like to do?" << endl << endl;
			vector<int> availableOpponents = getAvailableOpponents();
			for (int i = 0; i < availableOpponents.size(); i++)
			{
				if (currentSelection == i)
				{
					cout << " ---> Ask " << g->players[availableOpponents[i]]->name << " <--- " << endl;
				}
				else
				{
					cout << "      Ask " << g->players[availableOpponents[i]]->name << "      " << endl;
				}
			}
			if (currentSelection == availableOpponents.size())
			{
				cout << " ---> Declare Fish <--- ";
			}
			else
			{
				cout << "      Declare Fish      ";
			}
			cout << endl;
			while (true)
			{
				int MOD = availableOpponents.size() + 1;
				char c = _getch();
				if (c == 72)
				{
					currentSelection = (currentSelection + MOD - 1) % MOD;
					takeTurnUtil();
					break;
				}
				else if (c == 80)
				{
					currentSelection = (currentSelection + 1) % MOD;
					takeTurnUtil();
					break;
				}
				else if (c == ' ')
				{
					if (currentSelection == availableOpponents.size())
					{
						menuOption = 1;
					}
					else
					{
						menuOption = 3;
						askSelection = availableOpponents[currentSelection];
					}
					currentSelection = 0;
					takeTurnUtil();
					break;
				}
			}
		}
		else if (menuOption == 1) //fish-halfsuit selection
		{
			cout << "Which half suit do you want to call fish on?" << endl << endl;
			vector<int> callableHalfSuits;
			for (int i = 0; i < SUIT_MAX * 2; i++)
			{
				if (g->score[i] == 0)
				{
					callableHalfSuits.push_back(i);
				}
			}
			for (int i = 0; i < callableHalfSuits.size(); i++)
			{
				if (currentSelection == i)
				{
					cout << " ---> " << ((callableHalfSuits[i] % 2 == 0) ? "Low " : "High ") << SUIT_NAME[callableHalfSuits[i] / 2] << " <--- " << endl;
				}
				else
				{
					cout << "      " << ((callableHalfSuits[i] % 2 == 0) ? "Low " : "High ") << SUIT_NAME[callableHalfSuits[i] / 2] << "      " << endl;
				}
			}
			if (currentSelection == callableHalfSuits.size())
			{
				cout << " ---> Go Back <--- ";
			}
			else
			{
				cout << "      Go Back      ";
			}
			cout << endl;
			while (true)
			{
				int MOD = callableHalfSuits.size() + 1;
				char c = _getch();
				if (c == 72)
				{
					currentSelection = (currentSelection + MOD - 1) % MOD;
					takeTurnUtil();
					break;
				}
				else if (c == 80)
				{
					currentSelection = (currentSelection + 1) % MOD;
					takeTurnUtil();
					break;
				}
				else if (c == ' ')
				{
					if (currentSelection == callableHalfSuits.size())
					{
						menuOption = 0;
					}
					else
					{
						menuOption = 2;
						halfSuitSelection = callableHalfSuits[currentSelection];
					}
					currentSelection = 0;
					takeTurnUtil();
					break;
				}
			}
		}
		else if (menuOption == 2) //fish-card selection
		{
			cout << "You are declaring Fish on the " << ((halfSuitSelection % 2 == 0) ? "Low " : "High ") << SUIT_NAME[halfSuitSelection / 2] << endl;
			if (distribution.size() != NUMBER_MAX / 2)
			{
				for (int i = 0; i < distribution.size(); i++)
				{
					cout << "You are declaring that " << g->players[distribution[i]]->name << " has " << getCardString(halfSuitSelection * NUMBER_MAX / 2 + i) << endl;
				}
				if (cards[halfSuitSelection * NUMBER_MAX / 2 + distribution.size()] == true) //I have the card
				{
					distribution.push_back(ID);
					takeTurnUtil();
					return;
				}
				else
				{
					cout << "Who do you think has " << getCardString(halfSuitSelection * NUMBER_MAX / 2 + distribution.size()) << "?" << endl;
					vector<int> teammates = getTeammates();
					for (int i = 0; i < teammates.size(); i++)
					{
						if (currentSelection == i)
						{
							cout << " ---> " << g->players[teammates[i]]->name << " <--- " << endl;
						}
						else
						{
							cout << "      " << g->players[teammates[i]]->name << "      " << endl;
						}
					}
					cout << endl;
					while (true)
					{
						int MOD = teammates.size();
						char c = _getch();
						if (c == 72)
						{
							currentSelection = (currentSelection + MOD - 1) % MOD;
							takeTurnUtil();
							break;
						}
						else if (c == 80)
						{
							currentSelection = (currentSelection + 1) % MOD;
							takeTurnUtil();
							break;
						}
						else if (c == ' ')
						{
							distribution.push_back(teammates[currentSelection]);
							currentSelection = 0;
							takeTurnUtil();
							break;
						}
					}
				}
			}
			if(distribution.size() == NUMBER_MAX / 2)
			{
				//now we got the distribution
				g->declareFish(halfSuitSelection, distribution);
				distribution.clear();
				menuOption = 0; //reset turn
			}
		}
		else if (menuOption == 3) //asking-halfsuit selection
		{
			cout << "Which half suit do you want to ask " << g->players[askSelection]->name << " for?" << endl << endl;
			vector<int> availableHalfSuits = getAvailableHalfSuits();
			for (int i = 0; i < availableHalfSuits.size(); i++)
			{
				if (currentSelection == i)
				{
					cout << " ---> " << ((availableHalfSuits[i] % 2 == 0) ? "Low " : "High ") << SUIT_NAME[availableHalfSuits[i] / 2] << " <--- " << endl;
				}
				else
				{
					cout << "      " << ((availableHalfSuits[i] % 2 == 0) ? "Low " : "High ") << SUIT_NAME[availableHalfSuits[i] / 2] << "      " << endl;
				}
			}
			if (currentSelection == availableHalfSuits.size())
			{
				cout << " ---> Go Back <--- ";
			}
			else
			{
				cout << "      Go Back      ";
			}
			cout << endl;
			while (true)
			{
				int MOD = availableHalfSuits.size() + 1;
				char c = _getch();
				if (c == 72)
				{
					currentSelection = (currentSelection + MOD - 1) % MOD;
					takeTurnUtil();
					break;
				}
				else if (c == 80)
				{
					currentSelection = (currentSelection + 1) % MOD;
					takeTurnUtil();
					break;
				}
				else if (c == ' ')
				{
					if (currentSelection == availableHalfSuits.size())
					{
						menuOption = 0;
					}
					else
					{
						menuOption = 4;
						halfSuitSelection = availableHalfSuits[currentSelection];
					}
					currentSelection = 0;
					takeTurnUtil();
					break;
				}
			}
		}
		else //asking-card selection
		{
			cout << "What card do you want to ask " << g->players[askSelection]->name << " for?" << endl << endl;
			vector<int> availableCards = getAvailableCards(halfSuitSelection);
			for (int i = 0; i < availableCards.size(); i++)
			{
				if (currentSelection == i)
				{
					cout << " ---> " << NUMBER[availableCards[i] % NUMBER_MAX] << " of " << SUIT_NAME[availableCards[i] / NUMBER_MAX] << " <--- " << endl;
				}
				else
				{
					cout << "      " << NUMBER[availableCards[i] % NUMBER_MAX] << " of " << SUIT_NAME[availableCards[i] / NUMBER_MAX] << "      " << endl;
				}
			}
			if (currentSelection == availableCards.size())
			{
				cout << " ---> Go Back <--- ";
			}
			else
			{
				cout << "      Go Back      ";
			}
			cout << endl;
			while (true)
			{
				int MOD = availableCards.size() + 1;
				char c = _getch();
				if (c == 72)
				{
					currentSelection = (currentSelection + MOD - 1) % MOD;
					takeTurnUtil();
					break;
				}
				else if (c == 80)
				{
					currentSelection = (currentSelection + 1) % MOD;
					takeTurnUtil();
					break;
				}
				else if (c == ' ')
				{
					if (currentSelection == availableCards.size())
					{
						menuOption = 3;
					}
					else
					{
						g->processTurn(askSelection, availableCards[currentSelection]);
						return;
					}
					currentSelection = 0;
					takeTurnUtil();
					break;
				}
			}
		}
	}

	void printInfo()
	{
		system("CLS");
		cout << "Use the up and down arrow keys to move your selection and press spacebar to make your selection." << endl << endl;
		g->print();
		cout << "MENU OPTION: " << menuOption << endl;
	}

	void endGame() //this player is "forced" to call fish because it is the end of the game
	{
		printInfo();
		cout << "Only your team has cards. Who should be the one to declare fish?" << endl;
		vector<int> teammates = getTeammates();
		for (int i = 0; i < teammates.size(); i++)
		{
			if (currentSelection == i)
			{
				cout << " ---> " << g->players[teammates[i]]->name << " <--- " << endl;
			}
			else
			{
				cout << "      " << g->players[teammates[i]]->name << "      " << endl;
			}
		}
		if (currentSelection == teammates.size())
		{
			cout << " ---> I'll do it myself. <--- ";
		}
		else
		{
			cout << "      I'll do it myself.      ";
		}
		while (true)
		{
			int MOD = teammates.size() + 1;
			char c = _getch();
			if (c == 72)
			{
				currentSelection = (currentSelection + MOD - 1) % MOD;
				endGame();
				break;
			}
			else if (c == 80)
			{
				currentSelection = (currentSelection + 1) % MOD;
				endGame();
				break;
			}
			else if (c == ' ')
			{
				if (currentSelection == teammates.size())
				{
					playerEndGame();
				}
				else
				{
					menuOption = 0;
					g->turnID = teammates[currentSelection];
					g->players[teammates[currentSelection]]->endGame();
					break;
				}
			}
		}
	}

	void playerEndGame() //player decided to call fish himself at the end game
	{
		printInfo();
		if (menuOption == 0) //fish-halfsuit selection
		{
			cout << "Which half suit do you want to call fish on?" << endl << endl;
			vector<int> callableHalfSuits;
			for (int i = 0; i < SUIT_MAX * 2; i++)
			{
				if (g->score[i] == 0)
				{
					callableHalfSuits.push_back(i);
				}
			}
			for (int i = 0; i < callableHalfSuits.size(); i++)
			{
				if (currentSelection == i)
				{
					cout << " ---> " << ((callableHalfSuits[i] % 2 == 0) ? "Low " : "High ") << SUIT_NAME[callableHalfSuits[i] / 2] << " <--- " << endl;
				}
				else
				{
					cout << "      " << ((callableHalfSuits[i] % 2 == 0) ? "Low " : "High ") << SUIT_NAME[callableHalfSuits[i] / 2] << "      " << endl;
				}
			}
			cout << endl;
			while (true)
			{
				int MOD = callableHalfSuits.size();
				char c = _getch();
				if (c == 72)
				{
					currentSelection = (currentSelection + MOD - 1) % MOD;
					playerEndGame();
					break;
				}
				else if (c == 80)
				{
					currentSelection = (currentSelection + 1) % MOD;
					playerEndGame();
					break;
				}
				else if (c == ' ')
				{
					menuOption = 1;
					halfSuitSelection = callableHalfSuits[currentSelection];
					currentSelection = 0;
					playerEndGame();
					break;
				}
			}
		}
		else if (menuOption == 1)
		{
			cout << "You are declaring Fish on the " << ((halfSuitSelection % 2 == 0) ? "Low " : "High ") << SUIT_NAME[halfSuitSelection / 2] << endl;
			if (distribution.size() != NUMBER_MAX / 2)
			{
				for (int i = 0; i < distribution.size(); i++)
				{
					cout << "You are declaring that " << g->players[distribution[i]]->name << " has " << getCardString(halfSuitSelection * NUMBER_MAX / 2 + i) << endl;
				}
				if (cards[halfSuitSelection * NUMBER_MAX / 2 + distribution.size()] == true) //I have the card
				{
					distribution.push_back(ID);
					playerEndGame();
					return;
				}
				else
				{
					cout << "Who do you think has " << getCardString(halfSuitSelection * NUMBER_MAX / 2 + distribution.size()) << "?" << endl;
					vector<int> teammates = getTeammates();
					for (int i = 0; i < teammates.size(); i++)
					{
						if (currentSelection == i)
						{
							cout << " ---> " << g->players[teammates[i]]->name << " <--- " << endl;
						}
						else
						{
							cout << "      " << g->players[teammates[i]]->name << "      " << endl;
						}
					}
					cout << endl;
					while (true)
					{
						int MOD = teammates.size();
						char c = _getch();
						if (c == 72)
						{
							currentSelection = (currentSelection + MOD - 1) % MOD;
							playerEndGame();
							break;
						}
						else if (c == 80)
						{
							currentSelection = (currentSelection + 1) % MOD;
							playerEndGame();
							break;
						}
						else if (c == ' ')
						{
							distribution.push_back(teammates[currentSelection]);
							currentSelection = 0;
							playerEndGame();
							break;
						}
					}
				}
			}
			if (distribution.size() == NUMBER_MAX / 2)
			{
				//now we got the distribution
				g->declareFish(halfSuitSelection, distribution);
				distribution.clear();
				menuOption = 0; //reset turn
			}
		}
	}
};

/*
A computer player that logics its way to see who has what card
Thinking player goes through the last moves and tries to determine who has what
Memory parameter will be how many moves back it can look, and is generally the hyperparameter to tune for "intelligence"

Asking Policy: Will ask for the card that has the least number of people it could belong to
i.e. If it knows only 2 people could possibly have the card, it will try to ask for one of the two people
i.e. If someone has taken a card, they are the only person who could have that card, so it will ask for that card
Fish Policy: Will declare Fish if it is 100% sure that the cards of a HalfSuit are among its team
Endgame Policy: Will declare Fish on the suit that has the least uncertainty
*/
struct ThinkingPlayer : Player
{
	int memory;

	vector<vector<int>> cardLocationKnowledge; //list of players who could have card I
	//starts with everyone and dwindles down with misses
	vector<vector<int>> halfSuitKnowledge; //minimum number of cards player I has in halfSuit J
	
	ThinkingPlayer(string name, Game* g, int memoryInput) : Player(name, g) { memory = memoryInput; };

	void generateKnowledge()
	{
		int pastEpoch = max(0, (int) g->history.size() - memory);
		int lastEpoch = g->history.size() - 1;

		cardLocationKnowledge.clear();
		halfSuitKnowledge.clear();

		cardLocationKnowledge.resize(SUIT_MAX * NUMBER_MAX);
		for (int i = 0; i < cardLocationKnowledge.size(); i++)
		{
			for (int j = 0; j < NUM_PLAYERS; j++)
			{
				cardLocationKnowledge[i].push_back(j);
			}
		}

		halfSuitKnowledge.resize(NUM_PLAYERS);
		for (int i = 0; i < halfSuitKnowledge.size(); i++)
		{
			halfSuitKnowledge[i].resize(SUIT_MAX * 2);
		}

		for (int i = 0; i < g->history[pastEpoch].first.size(); i++)
		{
			if (g->history[pastEpoch].first[i] == ID) //i had this card in the past
			{
				cardLocationKnowledge[i].clear();
				cardLocationKnowledge[i].push_back(ID);
				halfSuitKnowledge[ID][getHalfSuit(i)]++;
			}
			else //i didn't have this card in the past
			{
				cardLocationKnowledge[i].erase(remove(cardLocationKnowledge[i].begin(), cardLocationKnowledge[i].end(), ID), cardLocationKnowledge[i].end());
			}
		}

		for (int i = pastEpoch + 1; i < g->history.size(); i++)
		{
			vector<int> theMove = g->history[i].second;
			if (theMove[1] == 0) //asking for card
			{
				int askingID = theMove[0]; //the person asking
				int targetID = theMove[2]; //the person being asked
				int cardID = theMove[3]; //the card being asked
				bool successful = theMove[4]; //whether or not the ask was successful
				if (g->history[lastEpoch].first[cardID] == -1) //it got declared fish between history and present
				{
					cardLocationKnowledge[cardID].clear();
					continue;
				}
				else
				{
					if (successful == true) //successful ask
					{
						cardLocationKnowledge[cardID].clear();
						cardLocationKnowledge[cardID].push_back(askingID);
						halfSuitKnowledge[askingID][getHalfSuit(cardID)] = max(2, halfSuitKnowledge[askingID][getHalfSuit(cardID)] + 1);
						halfSuitKnowledge[targetID][getHalfSuit(cardID)] = max(0, halfSuitKnowledge[targetID][getHalfSuit(cardID)] - 1);
					}
					else //unsuccessful ask
					{
						cardLocationKnowledge[cardID].erase(remove(cardLocationKnowledge[cardID].begin(), cardLocationKnowledge[cardID].end(), askingID), cardLocationKnowledge[cardID].end());
						cardLocationKnowledge[cardID].erase(remove(cardLocationKnowledge[cardID].begin(), cardLocationKnowledge[cardID].end(), targetID), cardLocationKnowledge[cardID].end());
						halfSuitKnowledge[askingID][getHalfSuit(cardID)] = max(1, halfSuitKnowledge[askingID][getHalfSuit(cardID)]);
					}
				}
			}
			else if (g->history[i].second[1] == 1) //declaring fish
			{
				//we already did this, as every declared on has been removed
			}
		}
		//some deduction we can make from halfSuitKnowledge
		for (int halfSuit = 0; halfSuit < SUIT_MAX * 2; halfSuit++)
		{
			int count = 0;
			int uniquePlayers = 0;
			vector<bool> playersWithHalfSuit;
			playersWithHalfSuit.resize(NUMBER_MAX / 2);
			for (int j = 0; j < NUM_PLAYERS; j++)
			{
				if (halfSuitKnowledge[j][halfSuit] > 0)
				{
					count += halfSuitKnowledge[j][halfSuit];
					playersWithHalfSuit[j] = true;
					uniquePlayers++;
				}
			}
			if (count == NUMBER_MAX / 2) //all cards in halfsuit have been accounted for
			{
				for (int j = halfSuit * NUMBER_MAX / 2; j < (halfSuit + 1) * NUMBER_MAX / 2; j++)
				{
					for (int k = 0; k < playersWithHalfSuit.size(); k++)
					{
						if (playersWithHalfSuit[k] == false) //doesn't have this half suit
						{
							//remove player k from list of people who can have this card, as they don't own the half suit
							cardLocationKnowledge[j].erase(remove(cardLocationKnowledge[j].begin(), cardLocationKnowledge[j].end(), k), cardLocationKnowledge[j].end());
						}
					}
				}
			}
		}
	}

	bool fishCheck()
	{
		vector<int> playableHalfSuits = g->getPlayableHalfSuits();
		for (int halfSuit : playableHalfSuits)
		{
			vector<int> distribution;
			for (int j = halfSuit * NUMBER_MAX / 2; j < (halfSuit + 1) * NUMBER_MAX / 2; j++)
			{
				if (cardLocationKnowledge[j].size() == 1
					&& teamID == g->players[cardLocationKnowledge[j][0]]->teamID) //i know where the card is and my teammate has it
				{
					distribution.push_back(cardLocationKnowledge[j][0]);
				}
				else
				{
					break;
				}
			}
			if (distribution.size() == NUMBER_MAX / 2)
			{
				g->declareFish(halfSuit, distribution);
				return true;
			}
		}
		return false;
	}

	void takeTurn()
	{
		generateKnowledge();

		if (fishCheck())
		{
			return;
		}

		vector<int> availableOpponents = getAvailableOpponents();
		vector<int> availableCards = getAvailableCards();
		vector<int> bestCard; //collection of best cards to ask for
		vector<int> askingID; //associated people to ask for those best cards
		askingID.push_back(availableOpponents[rand() % availableOpponents.size()]);
		bestCard.push_back(availableCards[rand() % availableCards.size()]);
		int bestScore = NUM_PLAYERS + 1;
		for (int i = 0; i < availableCards.size(); i++)
		{
			for (int j = 0; j < cardLocationKnowledge[availableCards[i]].size(); j++)
			{
				if (teamID != g->players[cardLocationKnowledge[availableCards[i]][j]]->teamID) //the person who owns this card is opposite my team
				{
					if (cardLocationKnowledge[availableCards[i]].size() < bestScore) //best score so far
					{
						bestScore = cardLocationKnowledge[availableCards[i]].size();
						askingID.clear();
						askingID.push_back(cardLocationKnowledge[availableCards[i]][j]);
						bestCard.clear();
						bestCard.push_back(availableCards[i]);
					}
					else if (cardLocationKnowledge[availableCards[i]].size() == bestScore) //tied with best score so far
					{
						askingID.push_back(cardLocationKnowledge[availableCards[i]][j]);
						bestCard.push_back(availableCards[i]);
					}
				}
			}
		}
		int randomIndex = rand() % askingID.size();
		g->processTurn(askingID[randomIndex], bestCard[randomIndex]);
	}

	void endGame()
	{
		vector<int> playableHalfSuits = g->getPlayableHalfSuits();
		int bestHalfSuit = playableHalfSuits[0], bestScore = NUM_PLAYERS / 2 * NUMBER_MAX / 2;
		for (int halfSuit : playableHalfSuits)
		{
			int score = 0;
			for (int i = halfSuit * NUMBER_MAX / 2; i < (halfSuit + 1) * NUMBER_MAX / 2; i++)
			{
				score += cardLocationKnowledge[i].size();
			}
			if (score < bestScore)
			{
				bestScore = score;
				bestHalfSuit = halfSuit;
			}
		}

		vector<int> teammates = getTeammates();
		vector<int> teammateCardDistribution;
		for (int i = 0; i < teammates.size(); i++)
		{
			for (int j = 0; j < g->players[teammates[i]]->numCards; j++)
			{
				teammateCardDistribution.push_back(teammates[i]);
			}
		}

		vector<int> distribution;
		for (int i = bestHalfSuit * NUMBER_MAX / 2; i < (bestHalfSuit + 1) * NUMBER_MAX / 2; i++)
		{
			if (cards[i] == true) // i have it
			{
				distribution.push_back(ID);
			}
			else
			{
				distribution.push_back(teammateCardDistribution[rand() % teammateCardDistribution.size()]);
			}
		}
		g->declareFish(bestHalfSuit, distribution);
	}
};

int main()
{
	srand(time(NULL));
	Game g;

	Player* players[NUM_PLAYERS];

	HumanPlayer player1("Player", &g);
	ThinkingPlayer player2("Alice", &g, 1000);
	ThinkingPlayer player3("Bob", &g, 1000);
	ThinkingPlayer player4("Xavier", &g, 1000);
	ThinkingPlayer player5("Yadier", &g, 1000);
	ThinkingPlayer player6("Zachary", &g, 1000);

	players[0] = &player1;
	players[1] = &player2;
	players[2] = &player3;
	players[3] = &player4;
	players[4] = &player5;
	players[5] = &player6;
	g.setPlayers(players);

	g.DealCards();

	while (!g.finished())
	{
		if (g.oneTeamEmpty())
		{
			g.endGame();
		}
		else
		{
			g.takeTurn();
		}
	}
	g.printResult();
	return 0;
}

// @END_OF_SOURCE_CODE
