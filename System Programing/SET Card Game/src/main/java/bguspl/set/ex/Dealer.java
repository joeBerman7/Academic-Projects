package bguspl.set.ex;
import bguspl.set.Env;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;
import java.util.Vector;
import java.util.stream.Collectors;
import java.util.stream.IntStream;
import java.util.logging.Level;

/**
 * This class manages the dealer's threads and data
 */
public class Dealer implements Runnable {

    /**
     * The game environment object.
     */
    private final Env env;

    /**
     * Game entities.
     */
    private final Table table;
    private final Player[] players;

    /**
     * The list of card ids that are left in the dealer's deck.
     */
    private final List<Integer> deck;

    /**
     * True iff game should be terminated.
     */
    private volatile boolean terminate;

    /**
     * The time when the dealer needs to reshuffle the deck due to turn timeout.
     */
    private long reshuffleTime = Long.MAX_VALUE;

    // our  //////////////////////////////////////////////
    private int displayCounterTime;
    long[] decreaseTimerArr = new long[60];
    int decreaseTimerArrIndex = 0;
    private Vector<Integer> requestsFromPlayresToCheck3Cards = new Vector<>();
    private Player chosenPlayer= null;
    private boolean givePoint = false;
    private boolean givePenalty = false;
    ////////////////////////////////////////////////////////

    public Dealer(Env env, Table table, Player[] players) {
        this.env = env;
        this.table = table;
        this.players = players;
        deck = IntStream.range(0, env.config.deckSize).boxed().collect(Collectors.toList());
    }

    /**
     * The dealer thread starts here (main loop for the dealer thread).
     */

    @Override
    public void run() {
        // run all the players' threads
        for(Player p : players)
            new Thread(p).start();

        env.logger.log(Level.INFO, "Thread " + Thread.currentThread().getName() + " starting.");
        while (!shouldFinish()) {

            displayCounterTime = (int)(env.config.turnTimeoutMillis/1000);
            placeAllCardsOnTable();

            // notify all robot player they can start generate
            for(Player player : players)
                player.play = true;

            timerLoop();

            // tell all players the round is over, so they don't keep generating slots
            for(Player player : players)
                player.play = false;

            //updateTimerDisplay(false);
            removeAllCardsFromTable();
        }
        announceWinners();
        env.logger.log(Level.INFO, "Thread " + Thread.currentThread().getName() + " terminated.");
    }

    /**
     * The inner loop of the dealer thread that runs as long as the countdown did not time out.
     */

    private void timerLoop() {
        resetTimer();
        while (!terminate && System.currentTimeMillis() < reshuffleTime) {
            sleepUntilWokenOrTimeout();
            updateTimerDisplay(false);
            removeCardsFromTable();
            placeCardsOnTable();
            long remainingMilli = reshuffleTime - System.currentTimeMillis() ;
            if(remainingMilli < 0) {break;}
        }
    }

    /**
     * Called when the game should be terminated.
     */
    public void terminate() {
        for(Player player : players) {
            player.terminate();
        }
        terminate = true;
    }

    /**
     * Check if the game should be terminated or the game end conditions are met.
     *
     * @return true iff the game should be finished.
     */
    private boolean shouldFinish() {
        return terminate || env.util.findSets(deck, 1).size() == 0;
    }

    /**
     * Checks cards should be removed from the table and removes them.
     */
    private void removeCardsFromTable() {
        int requestsSize = requestsFromPlayresToCheck3Cards.size();
        while(requestsSize>0) {
            //get the id of the player to check
            int idToCheck = requestsFromPlayresToCheck3Cards.elementAt(0);
            requestsFromPlayresToCheck3Cards.remove(0);
            boolean[][] playerBoard = table.playerTokens[idToCheck];
            int[] slotSet = new int[3];
            int setIndex = 0;
            int rows = playerBoard.length;
            for (int j=0; j<rows; j++) {
                int cols = playerBoard[j].length;
                for (int k=0; k< cols; k++) {
                    if (playerBoard[j][k]) {
                        slotSet[setIndex] = j*4 + k;
                        setIndex++;
                    }
                }
            }

            // check if there are 3 tokens by this player
            if (setIndex==3){handle3Cards(slotSet, players[idToCheck]);}
            else {players[idToCheck].setWaitingForResponseFromDealer();}

            // feedback for player///////////////////////////////////////////
            if(givePoint) {
                givePoint = false;
                chosenPlayer.point();
            }

            if(givePenalty) {
                givePenalty = false;
                chosenPlayer.penalty();
            }
            ////////////////////////////////////////////////////////////////
            requestsSize = requestsFromPlayresToCheck3Cards.size();
        }
    }


    /**
     * Check if any cards can be removed from the deck and placed on the table.
     */
    private void placeCardsOnTable() {

        if(deck.size() == 0){throw new IllegalStateException("The deck is empty, but the game is not finished.");}
        while (table.getEmptySlot() != -1) {
            //get the first card from the deck
            int card= deck.remove(0);
            //put the card on the table
            table.placeCard(card,table.getEmptySlot());
        }
        // now all the cards are back on the table so tell all the players that they don't need to be afraid to put cards wherever they want
        for(Player player : players) {
            player.clearFutureDemolition();
        }
    }

    /**
     * Sleep for a fixed amount of time or until the thread is awakened for some purpose.
     */
    private void sleepUntilWokenOrTimeout() {
        long sleepDuration = 10;
        // go to sleep
        try {
            synchronized (this) {
                if(requestsFromPlayresToCheck3Cards.size()>0) {

                }
                else {
                    // dealer wakes up (maybe by the player, maybe because sleep time is over
                    wait(sleepDuration);
                }
            }
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
    }

    /**
     * Reset and/or update the countdown and the countdown display.
     */
    private void updateTimerDisplay(boolean reset) {

        long currTime = System.currentTimeMillis();
        // check if the time to decrease the clock has passed
        long remainingMilliSeconds = reshuffleTime - currTime ;
        // warning
        if(remainingMilliSeconds<= env.config.turnTimeoutWarningMillis ) {
            if(remainingMilliSeconds<0) {
                env.ui.setCountdown(0, true);
            }
            else {env.ui.setCountdown(remainingMilliSeconds, true);}
        }
        // noraml state - this shoots 100 times in a second
        else if( currTime >= decreaseTimerArr[decreaseTimerArrIndex]	) {
            // decrease the clock
            displayCounterTime--;
            if(displayCounterTime == -1)
                displayCounterTime = (int)(env.config.turnTimeoutMillis/1000);
            // increase the next time that the clock needs to be decreased
            decreaseTimerArrIndex++;
            // update the ui
            env.ui.setCountdown(displayCounterTime*1000, false);
        }
    }

    /**
     * Returns all the cards from the table to the deck.
     */
    private void removeAllCardsFromTable() {
        int cardSlot;
        while ((cardSlot=table.getCardSlot())!=-1) {
            int card= table.getCardBySlot(cardSlot);//get the card
            table.removeCard(cardSlot);// remove the card from the table and it's tokens
            deck.add(card);// add the card back into the deck
        }
    }

    /**
     * Check who is/are the winner/s and displays them.
     */
    private void announceWinners() {
        removeAllCardsFromTable();
        Player[] winners = players;
        List<Integer> winnersId = new ArrayList<>();
        Arrays.sort(winners, Comparator.comparingInt(Player::score).reversed());
        int maxScore = winners[0].score();
        System.out.print("The winner(s) is/are: ");
        for (Player player : winners) {
            if (player.score() == maxScore) {
                winnersId.add(player.id());
                System.out.print(player.id() + " ,");
            } else {
                break;
            }
        }
        System.out.println("");
        int[] winnersIdArray = Arrays.stream(winnersId.toArray(new Integer[winnersId.size()])).mapToInt(Integer::intValue).toArray();
        env.ui.announceWinner(winnersIdArray);
    }

    //////////////////////// our func //////////////////////////

    public void handle3Cards(int [] slotArray, Player player) {
        //convert slot array to a cards array
        int [] cardsToCheck= table.convertSlotArrToCard(slotArray);
        //check if the set is legal
        boolean isSet= env.util.testSet(cardsToCheck);
        //award and freeze
        if (isSet) {
            // remove all 3 cards
            for(int slot : slotArray) {
                // before removing card : remove the tokens from the players queues
                for(Player p: players)
                    //if(p.id != player.id)/////////////////////////////////////////////////////
                    p.removeSlotFromKeyQueue(slot);
                // remove single card
                table.removeCard(slot);
            }
            chosenPlayer = player;
            givePoint = true;
            // reset the time to 60
            resetTimer();

        }
        else {
            // make the player freeze for 3 seconds
            chosenPlayer = player;
            givePenalty = true;
        }
    }

    private void resetTimer() {
        displayCounterTime = (int)(env.config.turnTimeoutMillis/1000);
        long currTime = System.currentTimeMillis();
        reshuffleTime = currTime + env.config.turnTimeoutMillis;
        // array of times to decrease clock during the minutes
        decreaseTimerArr = new long[displayCounterTime+1];
        for(int i = 0 ;i < displayCounterTime+1 ;i++)
            decreaseTimerArr[i] = currTime + (i+1)*1000;
        decreaseTimerArrIndex = 0;

        env.ui.setCountdown(env.config.turnTimeoutMillis, false);

    }

    private void placeAllCardsOnTable() {
        // TODO implement
        //check if there is a legal set in the deck
        List<int[]> exsistSet= env.util.findSets(deck,1);
        shuffleCards();

        int firstEmptySlot;
        //get all the empty slots on the table
        while ((firstEmptySlot=table.getEmptySlot()) != -1) {
            //get the first card from the deck
            int card= deck.remove(0);
            //put the card on the table
            table.placeCard(card,firstEmptySlot);
        }
    }

    //shuffle the cards
    private void shuffleCards() {
        Collections.shuffle(deck);
    }

    // this is called by the player after generating the 3rd card
    public synchronized void notifyEvalutationFromPlayer(int id) {
        // wake up the dealer
        requestsFromPlayresToCheck3Cards.add(id);
        notifyAll();
    }

}



