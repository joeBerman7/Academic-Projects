package bguspl.set.ex;
import java.util.Vector;
import java.util.concurrent.ThreadLocalRandom;

import bguspl.set.Env;
/**
 * This class manages the players' threads and data
 *
 * @inv id >= 0
 * @inv score >= 0
 */
public class Player implements Runnable {

    /**
     * The game environment object.
     */
    private final Env env;

    /**
     * Game entities.
     */
    private final Table table;

    /**
     * The id of the player (starting from 0).
     */
    public final int id;

    /**
     * The thread representing the current player.
     */
    private Thread playerThread;

    /**
     * The thread of the AI (computer) player (an additional thread used to generate key presses).
     */
    private Thread aiThread;

    /**
     * True iff the player is human (not a computer player).
     */
    private final boolean human;

    /**
     * True iff game should be terminated.
     */
    private volatile boolean terminate;

    /**
     * The current score of the player.
     */
    private int score;

    private Dealer dealer;

    /**
     * The class constructor.
     *
     * @param env    - the environment object.
     * @param dealer - the dealer object.
     * @param table  - the table object.
     * @param id     - the id of the player.
     * @param human  - true iff the player is a human player (i.e. input is provided manually, via the keyboard).
     */

    // our vars
    private Vector<Integer> futureDemolition = new Vector<>();
    private boolean shouldRemoveToken = false;
    private int tokenToBeRemoved = -1;
    private boolean waitingForResponseFromDealer = false;
    private boolean clearEraseCardsTokensFromQueueFlag = false;
    boolean penalty = false;
    boolean reward = false;
    public boolean play = false;
    private long supposeToFinishPenaltyTime = -1;
    private boolean shouldHandleKeyPress = false;
    private BQ keyQueue= new BQ(3);

    private int key = -1;

    public Player(Env env, Dealer dealer, Table table, int id, boolean human) {
        this.env = env;
        this.table = table;
        this.id = id;
        this.human = human;
        // added
        this.dealer = dealer;
    }

    /**
     * The main player thread of each player starts here (main loop for the player thread).
     */
    @Override
    public void run() {
        playerThread = Thread.currentThread();
        env.logger.info("thread " + Thread.currentThread().getName() + " starting.");

        if (!human)
            createArtificialIntelligence();

        while (!terminate) {
            // TODO implement main player loop
            synchronized (this) {
                try {
                    wait();
                } catch (InterruptedException e1) {
                    e1.printStackTrace();
                }
                if(penalty) {// if the dealer punish the player for 3 tokens not a set
                    waitingForResponseFromDealer = false;

                    // create a thread that changed the time of the player in the ui
                    new Thread(() -> {
                        long count = env.config.penaltyFreezeMillis;
                        while(count>=0) {
                            try {
                                env.ui.setFreeze(id, count);
                                Thread.sleep(1000);
                                count = count - 1000;
                            } catch (InterruptedException e) {
                                // TODO Auto-generated catch block
                                e.printStackTrace();
                            }
                        }

                    }).start();

                    try {
                        supposeToFinishPenaltyTime = System.currentTimeMillis() + env.config.penaltyFreezeMillis;
                        // check if wake up time has passed
                        while(System.currentTimeMillis() < supposeToFinishPenaltyTime)
                        {
                            // if the wake up time is still in the future go to sleep again
                            shouldHandleKeyPress = false;
                            wait(supposeToFinishPenaltyTime - System.currentTimeMillis());
                        }

                    } catch (InterruptedException e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    }

                    // clean the key queue
                    if(!human) {
                        clearQueue();
                        for (int i=0; i<12; i++) {
                            table.removeToken(id, i);
                        }
                    }
                    penalty = false;
                }

                if(reward) {
                    waitingForResponseFromDealer = false;
                    // thread that create the counter decrease
                    new Thread(() -> {
                        long count = env.config.pointFreezeMillis;
                        while(count>=0) {
                            try {
                                env.ui.setFreeze(id, count);
                                Thread.sleep(1000);
                                count = count - 1000;
                            } catch (InterruptedException e) {
                                // TODO Auto-generated catch block
                                e.printStackTrace();
                            }
                        }
                    }).start();

                    try {
                        long supposeToFinishPointTime = System.currentTimeMillis() + env.config.pointFreezeMillis;
                        // check if wake up time has passed
                        while(System.currentTimeMillis() < supposeToFinishPointTime)
                        {
                            // if the wake up time is still in the future go to sleep again
                            shouldHandleKeyPress = false;
                            wait(supposeToFinishPointTime - System.currentTimeMillis());
                        }

                    } catch (InterruptedException e) {
                        // TODO Auto-generated catch block
                        e.printStackTrace();
                    }

                    clearQueue();
                    reward = false;
                }
                if(shouldRemoveToken) {
                    keyQueue.remove(tokenToBeRemoved);
                    shouldRemoveToken = false;
                }
                if(shouldHandleKeyPress) {
                    keyPressHandleByPlayer(key);
                    shouldHandleKeyPress = false;
                }
                if(clearEraseCardsTokensFromQueueFlag) {
                    clearEraseCardsTokensFromQueueFlag = false;
                    clearEraseCardsTokensFromQueue();
                }

            }
        }
        if (!human) try { aiThread.join(); } catch (InterruptedException ignored) {}
        env.logger.info("thread " + Thread.currentThread().getName() + " terminated.");
    }

    /**
     * Creates an additional thread for an AI (computer) player. The main loop of this thread repeatedly generates
     * key presses. If the queue of key presses is full, the thread waits until it is not full.
     */
    private void createArtificialIntelligence() {
        // note: this is a very, very smart AI (!)
        aiThread = new Thread(() -> {
            env.logger.info("thread " + Thread.currentThread().getName() + " starting.");
            boolean shouldClearQueue = true; // added
            while (!terminate) {
                // TODO implement player key press simulator
                if (play){
                    try {
                        // time that takes to ai to generate new key press
                        long sleepTime = ThreadLocalRandom.current().nextLong(20, 40);
                        synchronized (this) {
                            wait(sleepTime);
                        }
                    } catch (InterruptedException ignored) {}
                    // generate a key
                    int randSlot = ThreadLocalRandom.current().nextInt(12);
                    // generate a key that is not the same as the the keys in the keyQueue
                    while( keyQueue.checkIfTokenExist(randSlot) ) {
                        randSlot = ThreadLocalRandom.current().nextInt(12);
                    }
                    // tell the player about the new slot
                    keyPressed(randSlot);
                }
                else if(shouldClearQueue)
                {
                    shouldClearQueue = false;
                    clearQueue();
                }

            }
            env.logger.info("thread " + Thread.currentThread().getName() + " terminated.");
        }, "computer-" + id);
        aiThread.start();
    }

    /**
     * Called when the game should be terminated.
     */
    public void terminate() {
        // TODO implement
        terminate = true;
    }

    /**
     * This method is called when a key is pressed.
     *
     * @param slot - the slot corresponding to the key pressed.
     */
    public void keyPressed(int slot) {
        // TODO implement
        // if the player has 3 cards that are waiting for evaluation from the dealer dont do anything
        if(waitingForResponseFromDealer ) {
            return;
        }
        shouldHandleKeyPress = true;
        key = slot;
        synchronized (this) {
            notify();
        }
    }

    /**
     * Award a point to a player and perform other related actions.
     *
     * @post - the player's score is increased by 1.
     * @post - the player's score is updated in the ui.
     */
    public void point() {
        // TODO implement
        synchronized (this) {
            reward = true;
            notify();
            int ignored = table.countCards(); // this part is just for demonstration in the unit tests
            env.ui.setScore(id, ++score);
        }
    }

    /**
     * Penalize a player and perform other related actions.
     * freezing his ability of removing or placing his
     tokens for a specified time period.
     */
    public void penalty() {
        synchronized (this) {
            penalty = true;
            notifyAll();
        }
        // finished
    }

    public int score() {
        return score;
    }

    /////////////////// Getters and Setters ///////////////////
    public int id() {
        return id;
    }


    public void setScore(int score) {
        this.score = score;
    }

    public synchronized void clearFutureDemolition() {
        futureDemolition.clear();
    }

    public synchronized void removeSlotFromKeyQueue(int slot) {
        shouldRemoveToken = true;
        tokenToBeRemoved = slot;
        futureDemolition.add(slot);
        notify();
    }

    public synchronized void setWaitingForResponseFromDealer() {
        waitingForResponseFromDealer = false;
        clearEraseCardsTokensFromQueueFlag = true;
        notify();
    }

    public void clearQueue() {
        // clearEraseCardsTokensFromQueueFlag = true;
        keyQueue.clear();
    }

    // this method will be called if some of the cards that holds my tokens were erased
    private void clearEraseCardsTokensFromQueue() {
        for(int i = 0 ;i < keyQueue.size() ;i++) {
            table.removeToken(id, keyQueue.get(i));
        }
        keyQueue.clear();
    }

    private void keyPressHandleByPlayer(int slot) {

        // check if the token already exist on the table in this slot
        if(keyQueue.checkIfTokenExist(slot)) {
            // remove the slot from the blocked queue
            keyQueue.remove(slot);
            // remove the slot from the table
            table.removeToken(id, slot);
            return;
        }

        // check if the slot is in the future demolition
        synchronized (this) {
            for(Integer integer : futureDemolition)
                if(integer.intValue() == slot) {
                    return;
                }
        }

        // the new slot is checked and waont found in the blocked queue
        int size = keyQueue.size();
        if (size==3) {
            return;
        }

         if(table.getCardBySlot(slot)==-1) {
         	return;
         }

        //placeToken
        table.placeToken(id, slot);
        //adding to the queue of tokens
        keyQueue.put(slot);
        if (keyQueue.size()==3) {
            waitingForResponseFromDealer = true;
            dealer.notifyEvalutationFromPlayer(id);
        }

    }

}
