package edu.cs300;

import CtCILibrary.*;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;
import java.util.Scanner;
import java.util.concurrent.*;

class Worker extends Thread {

  Trie textTrieTree;
  ArrayBlockingQueue prefixRequestArray;
  ArrayBlockingQueue resultsOutputArray;
  int id;
  String passageName;

  public Worker(String[] words, int id, ArrayBlockingQueue prefixes, ArrayBlockingQueue results) {
    this.textTrieTree = new Trie(words);
    this.prefixRequestArray = prefixes;
    this.resultsOutputArray = results;
    this.id = id;
    this.passageName = "Passage-" + Integer.toString(id) + ".txt";// put name of passage here
  }

  // instead of taking in words, take in the passage file name and build it with
  // the correct regex.
  public Worker(String passageName, int id, ArrayBlockingQueue prefixes, ArrayBlockingQueue results) {
    this.prefixRequestArray = prefixes;
    this.resultsOutputArray = results;
    this.id = id;
    this.passageName = passageName;// put name of passage here
    buildTrie();
  }

  public void run() {
    System.out.println("Worker-" + this.id + " (" + this.passageName + ") thread started ...");
    while (true) {
      try {
        SearchRequest request = (SearchRequest) this.prefixRequestArray.take();

        // if newest request has an id of 0, exit the thread for proper memory
        // management.
        if (request.requestID == 0) {
          return;
        }

        boolean found = this.textTrieTree.contains(request.prefix);

        // print out results
        if (!found) {
          System.out.println("Worker-" + this.id + " " + request.requestID + ":" + request.prefix + " ==> not found ");
          resultsOutputArray.put(new SearchResponse(request.requestID, request.prefix, id, passageName, "----", 0, 0));
        } else {
          String foundWord = this.textTrieTree.getLongestWord(request.prefix);
          System.out
              .println("Worker-" + this.id + " " + request.requestID + ":" + request.prefix + " ==> " + foundWord);
          resultsOutputArray
              .put(new SearchResponse(request.requestID, request.prefix, id, passageName, foundWord, 0, 1));

        }
      } catch (InterruptedException e) {
        System.err.println(e.getMessage());
      }
    }
  }

  // this is moved to the runnable section of the thread, such that every thread
  // can do this simulaneously. assumes this is run after the constructor, such
  // that the passage name is initialized
  public void buildTrie() {
    List<String> words = new ArrayList<String>();
    try {
      Path passagePath = Paths.get(this.passageName);
      Scanner sc = new Scanner(passagePath);
      sc.useDelimiter("[^a-zA-Z\'-]");

      while (sc.hasNext()) {
        // have the scanner break on any character that isn't alphabetic, an apostrophe,
        // or dash
        String s = sc.next();
        s = s.toLowerCase();

        // now if the string is at least 3 characters and only contains letters, we can
        // add it to the list.
        if (s.matches("[a-z]{3,100}")) {
          words.add(s);
        }
      }
    } catch (IOException exception) {
      exception.printStackTrace();
    }
    this.textTrieTree = new Trie(words);
  }
}
