#ifndef BLIF_VERIFIER_TOKENIZER_H
#define BLIF_VERIFIER_TOKENIZER_H

#include <iosfwd>
#include <vector>
#include <stack>
#include <string>

namespace Tokenizer {


// Simple class to return lines one at a time as tokens and discard comments
// and empty lines while allowing pushback capability.
class LineTokenReader {
  public:
    // Default ctor so class can be moved.
    LineTokenReader();

    // Construct a LineTokenReader bound to an input stream.
    LineTokenReader(std::istream& is);

    // Read the next line as a token list.
    void readLine(std::vector<std::string>& tokens);
    std::vector<std::string> readLine();

    // Push a vector of tokens back onto the reader. It will then be read before
    // subsequent reads from file. Supports move semantics too!
    void putBack(const std::vector<std::string>& tokens);
    void putBack(std::vector<std::string>&& tokens);

    // Query whether data can be read
    bool isGood() const;

  private:
    // All pushed back values to be read back before reading from file.
    std::stack<std::vector<std::string>> mPushbacks;

    // The bound istream, since istreams are not concrete types and thus cannot
    // be moved, the best we can do is hold a pointer to it.
    std::istream* mStream;

    // Whether data can be read.
    bool mGood;
};

} // namespace Tokenizer

#endif // BLIF_VERIFIER_TOKENIZER_H
