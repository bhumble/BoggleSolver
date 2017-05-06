#include <iostream>
#include <fstream>
#include <unordered_set>
#include <string>
#include <list>
#include <vector>
#include <functional>


using namespace std;


class Dictionary
{
private:
    unordered_set<string> mWords;
    string mShortestWord;
    string mLongestWord;

public:
    // Load dictionary from line-delimited file and return number of words
    ssize_t loadDictionaryFile(const char* path = "/usr/share/dict/words")
    {
        ifstream dictionaryFile(path);
        if (!dictionaryFile.good())
        {
            dictionaryFile.close();
            return -1;
        }
        string line;
        while (getline(dictionaryFile, line))
        {
            mWords.insert(line);
            if (mLongestWord.empty() || line.length() > mLongestWord.length())
            {
                mLongestWord = line;
            }
            if (mShortestWord.empty() || line.length() < mShortestWord.length())
            {
                mShortestWord = line;
            }
        }
        dictionaryFile.close();
        return size();
    }

    size_t size() { return mWords.size(); }
    string getShortestWord() { return mShortestWord; }
    string getLongestWord() { return mLongestWord; }
    bool contains(string& word) { return mWords.find(word) != mWords.end(); }
};


template <class T>
class Matrix
{
private:
    size_t mWidth;
    size_t mHeight;
    vector<T> mData;
public:
    Matrix(size_t width, size_t height) : mWidth(width), mHeight(height), mData(width * height) {}
    T& operator()(size_t x, size_t y) { return mData[y * mWidth + x]; }
    T operator()(size_t x, size_t y) const { return mData[y * mWidth + x]; }
    size_t width() const { return mWidth; }
    size_t height() const { return mHeight; }
    T get(size_t x, size_t y) { return mData[y * mWidth + x]; }
    void set(const vector<T>& data) { mData = data; }
    void set(size_t x, size_t y, const T& value) { mData[y * mWidth + x] = value; }
    void set(const T value)
    {
        for (int i = 0; i < mData.size(); ++i)
        {
            mData[i] = value;
        }
    }
    friend ostream& operator<<(ostream& os, const Matrix<T>& rhs)
    {
        for (size_t y = 0; y < rhs.mHeight; ++y)
        {
            os << "[ ";
            for (size_t x = 0; x < rhs.mWidth; ++x)
            {
                os << rhs.mData[y * rhs.mWidth + x];
                if (x != rhs.mWidth - 1)
                {
                    os << ", ";
                }
            }
            os << " ]" << endl;
        }
        return os;
    }
};


struct Rules
{
    Dictionary& dictionary;
    size_t minWordLength;
    size_t maxWordLength;
    bool allowReuse;

    Rules(Dictionary& _dictionary, size_t _minWordLength = 0, size_t _maxWordLength = size_t(-1), bool _allowReuse = false)
        : dictionary(_dictionary), minWordLength(_minWordLength), maxWordLength(_maxWordLength), allowReuse(_allowReuse)
    {}
};


list<string> findWordsRecursive(const Matrix<char>& letterGrid, Rules& rules, size_t x, size_t y, Matrix<bool> visited, string letterChain, function<void(const char*)> foundWordCallback)
{
    list<string> words;

    // Mark this position as visited
    if (!rules.allowReuse)
    {
        visited.set(x, y, true);
    }

    // Append char at position x,y to letterChain and see if it forms a word
    letterChain.push_back(letterGrid(x, y));
    cout << x << "," << y << "," << letterChain << endl;
    if (letterChain.length() >= rules.minWordLength && rules.dictionary.contains(letterChain))
    {
        foundWordCallback(letterChain.c_str());
        words.push_back(letterChain);
    }

    // Keep looking in adjacent cells
    if (letterChain.length() < rules.maxWordLength)
    {
        for (ssize_t adjacentY = y - 1; adjacentY <= y + 1; ++adjacentY) // above, middle, below
        {
            for (ssize_t adjacentX = x - 1; adjacentX <= x + 1; ++adjacentX) // left, middle, right
            {
                cout << adjacentX << "," << adjacentY << endl;
                if (adjacentX == x && adjacentY == y)
                {
                    continue; // don't recurse on own position
                }
                else if (adjacentX < 0 || adjacentX >= letterGrid.width() || adjacentY < 0 || adjacentY >= letterGrid.height())
                {
                    continue; // out of bounds
                }
                else if (!rules.allowReuse && visited.get(adjacentX, adjacentY))
                {
                    continue; // don't allow re-use of letters in the same word if rules forbid it
                }
                else
                {
                    words.merge(findWordsRecursive(letterGrid, rules, adjacentX, adjacentY, visited, letterChain, foundWordCallback)); // recurse on adjacent letter
                }
            }
        }
    }

    return words;
}


list<string> findWords(const Matrix<char>& letterGrid, Rules& rules, function<void(const char*)> foundWordCallback)
{
    Matrix<bool> visited(letterGrid.width(), letterGrid.height());
    visited.set(false);

    list<string> words;
    string letterChain;
    for (size_t y = 0; y < letterGrid.height(); ++y)
    {
        for (size_t x = 0; x < letterGrid.width(); ++x)
        {
            words.merge(findWordsRecursive(letterGrid, rules, x, y, visited, letterChain, foundWordCallback));
        }
    }

    return words;
}


int main(int argc, const char* argv[])
{
    Dictionary dictionary;
    if (dictionary.loadDictionaryFile("./dictionary.txt") < 0) // try local dictionary first
    {
        if (dictionary.loadDictionaryFile() < 0) // try system default dictionary
        {
            return 1; // failed to load dictionary file
        }
    }
    cout << "Succesfully loaded " << dictionary.size() << " words: shortest = \"" << dictionary.getShortestWord() << "\", longest = \"" << dictionary.getLongestWord() << "\"" << endl;

    // Optional test of dictionary load
    if (argc == 2)
    {
        string arg = argv[1];
        cout << arg + " is" + (dictionary.contains(arg) ? "" : " not") + " a word" << endl;
    }

    // Set rules
    Rules rules(dictionary);
    rules.minWordLength = 7;
    rules.maxWordLength = 7;
    rules.allowReuse = false;

    // Initialize boggle grid
    Matrix<char> letterGrid(5, 5);
    vector<char> init = { 'f', 'h', 'n', 'm', 'w', 'l', 't', 'v', 'o', 'x', 'y', 'c', 'a', 'e', 'b', 'q', 's', 'i', 'r', 'g', 'p', 'u', 'd', 'z', 'k' };
    letterGrid.set(init);

    // Find words
    list<string> words = findWords(letterGrid, rules, [](const char* word){cout << word << endl;});

    cout << endl << "Boggle: " << endl << letterGrid << endl;
    cout << "Found " << words.size() << " words:" << endl;
    for (string word : words)
    {
        cout << "    " << word << endl;
    }

    return 0;
}
