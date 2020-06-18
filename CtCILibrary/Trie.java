package CtCILibrary;

import java.util.LinkedList;
import java.util.List;


/* Implements a trie. We store the input list of words in tries so
 * that we can efficiently find words with a given prefix. 
 */ 
public class Trie
{
    // The root of this trie.
    private TrieNode root;

    /* Takes a list of strings as an argument, and constructs a trie that stores these strings. */
    public Trie(List<String> list) {
        root = new TrieNode();
        for (String word : list) {
            root.addWord(word);
        }
    }  
    

    /* Takes a list of strings as an argument, and constructs a trie that stores these strings. */    
    public Trie(String[] list) {
        root = new TrieNode();
        for (String word : list) {
            root.addWord(word);
        }
    }    

    /* Checks whether this trie contains a string with the prefix passed
     * in as argument.
     */
    public boolean contains(String prefix, boolean exact) {
        TrieNode lastNode = root;
        int i = 0;
        for (i = 0; i < prefix.length(); i++) {
            lastNode = lastNode.getChild(prefix.charAt(i));
            if (lastNode == null) {
                return false;
            }
        }
        return !exact || lastNode.terminates();
    }

    public String getLongestWord(String prefix) {
        //traverse through the prefix. this method assumes the prefix has been found and exists. 
        TrieNode current = root;
        int i = 0;
        for (i = 0; i < prefix.length(); i++) {
            current = current.getChild(prefix.charAt(i));
        }

        //current should now be the last node of the prefix. we will do this with a breadth first search queue
        String longestWord = prefix;
        LinkedList<TrieNode> searchQueue = new LinkedList<TrieNode>();
        searchQueue.addAll(current.getChildren());
        //now we want to go through all of the children
        while(!searchQueue.isEmpty()){
            current = searchQueue.removeFirst();
            //check if this isn't a leaf
            if(!current.terminates()){
                searchQueue.addAll(current.getChildren());
            }else if(current.getOriginalWord().length() > longestWord.length()){ //if it is a leaf, then we want to see if its longer than the current longest word
                longestWord = current.getOriginalWord();
                //System.out.println("New longest word: " + longestWord);
            }else if(current.getOriginalWord().length() == longestWord.length()){ //if they are the same length, take the last one alphabetically
                int comparison = current.getOriginalWord().compareTo(longestWord); //if positive, getoriginalword is alphabetically later than previous longest word. 
                if(comparison > 0){
                    longestWord = current.getOriginalWord();
                }
            }
        }
        return longestWord;
    }
    
    public boolean contains(String prefix) {
    	return contains(prefix, false);
    }
    
    public TrieNode getRoot() {
    	return root;
    }
}
