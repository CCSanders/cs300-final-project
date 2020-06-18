package edu.cs300;

public class SearchResponse {

    int prefixID;
    String prefix;
    int passageIndex;
    String passageName;
    String longestWord;
    int passageCount;
    int present;

    @Override
    public String toString() {
        return "SearchResponse [longestWord=" + longestWord + ", passageCount=" + passageCount + ", passageIndex="
                + passageIndex + ", passageName=" + passageName + ", prefix=" + prefix + ", prefixID=" + prefixID
                + ", present=" + present + "]";
    }

    public SearchResponse(int prefixID, String prefix, int passageIndex, String passageName, String longestWord,
            int passageCount, int present) {
        this.prefixID = prefixID;
        this.prefix = prefix;
        this.passageIndex = passageIndex;
        this.passageName = passageName;
        this.longestWord = longestWord;
        this.passageCount = passageCount;
        this.present = present;
    }
}