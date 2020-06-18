package edu.cs300;

import java.io.File;
import java.io.IOException;
import java.nio.file.*;
import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;
import java.util.concurrent.*;

public class PassageProcessor {

    // write longest word response msg:
    // prefix id, prefix, passage id, passage, longest word, total num passages, and
    // was it found?
    public static void main(String[] args) {

        // list of file names for the passage titles. will be opened in parallel within
        // the threadsa
        List<String> passageTitles = new ArrayList<String>();
        // file io
        try {
            Path passagesFile = Paths.get("passages.txt");
            passageTitles = Files.readAllLines(passagesFile);
        } catch (IOException exception) {
            exception.printStackTrace();
            System.exit(0);
        }

        //remove invalid passages
        List<String> invalidPassages = new ArrayList<String>();
        for(String passage : passageTitles){
            if(!Files.exists(Paths.get(passage))){
                invalidPassages.add(passage);
            }
        }
        passageTitles.removeAll(invalidPassages);

        if (passageTitles.size() == 0) {
            System.err.println("No valid paths to passages found in passages.txt. Exiting...");
            System.exit(-1);
        }

        // each worker input queue is assigned to a worker thread.
        // the threads will process the input queues as things are put
        ArrayBlockingQueue[] workerInputQueues = new ArrayBlockingQueue[passageTitles.size()];
        ArrayBlockingQueue resultsOutputArray = new ArrayBlockingQueue<>(passageTitles.size() * 10);
        List<Worker> threads = new ArrayList<Worker>();

        // each input queue can have 10 prefixes waiting to be processed, as they could
        // be recieved asynchronously
        for (int i = 0; i < passageTitles.size(); i++) {
            workerInputQueues[i] = new ArrayBlockingQueue<>(10);
        }

        // may not need to create the workers independently here from the input queues
        // but ??
        for (int i = 0; i < passageTitles.size(); i++) {
            Worker w = new Worker(passageTitles.get(i), i, workerInputQueues[i], resultsOutputArray);
            threads.add(w);
            w.start();
        }

        while (true) {
            // get the request
            SearchRequest newRequest = MessageJNI.readPrefixRequestMsg();
            System.out.println("**prefix(" + newRequest.requestID + ") " + newRequest.prefix + " received");

            for (int i = 0; i < passageTitles.size(); i++) {
                try {
                    workerInputQueues[i].put(newRequest);
                } catch (InterruptedException e) {
                    System.err.println("Error putting prefix request onto worker queue. Thread: " + i + ", Request: "
                            + newRequest);
                }
                ;
            }

            // check if new request is an exit message
            if (newRequest.requestID == 0) {
                System.out.println("Terminating...");
                for (Thread t : threads) {
                    try {
                        t.join();
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
                System.exit(1);
            }

            int counter = 0;
            while (counter < passageTitles.size()) {
                try {
                    SearchResponse results = (SearchResponse) resultsOutputArray.take();
                    results.passageCount = passageTitles.size();
                    MessageJNI.writeLongestWordResponseMsg(results.prefixID, results.prefix, results.passageIndex,
                            results.passageName, results.longestWord, results.passageCount, results.present);
                    counter++;
                } catch (InterruptedException e) {
                    System.err.println("Error taking prefix response from results queue.");
                    e.printStackTrace();
                }
                ;
            }
        }
    }
}