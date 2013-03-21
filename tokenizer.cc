#include <algorithm>
#include <iterator>
#include <sstream>

#include "tokenizer.h"

namespace Tokenizer {

using std::string;
using std::vector;
using std::stack;
using std::istream;
using std::istringstream;

LineTokenReader::LineTokenReader()
  : mGood(false) { }

LineTokenReader::LineTokenReader(istream& is)
  : mStream(&is), mGood(true), mRawLineNumber(0) { }

void LineTokenReader::readLine(vector<string>& result) {
  result.clear();
  string line;

  // If anything has been pushed back, return that first. Note that we
  // intentionally do not check whether the entries are empty/comments only
  // -- users are responsible for what they push.
  if (!mPushbacks.empty()) {
    result = std::move(mPushbacks.top());
    mPushbacks.pop();
    return;
  }

  // Read until we fail to read a line or get a non-empty line. Lines with just
  // whitespace or a comment are ignored as empty.
  while (result.empty()) {
    // Read failure. Return empty result.
    if (!std::getline(*mStream, line)) {
      mGood = false;
      return;
    }

    ++mRawLineNumber;

    // Tokenize the line and store the tokens in the vector.
    istringstream tokenizer(line);
    std::copy(std::istream_iterator<string>(tokenizer),
              std::istream_iterator<string>(),
              std::back_inserter(result));


    // Remove any comments (tokens that begin with #) and any tokens after them
    // if a comment is found.
    auto comment = std::find_if(result.begin(), result.end(),
                                [](const string& tok) {return tok[0] == '#';});
    result.erase(comment, result.end());

    // The vector may now be empty (eg, if the entire line was a comment, blank,
    // etc), in which case we'll try again.
  }
}

vector<string> LineTokenReader::readLine() {
  vector<string> rval;
  readLine(rval);
  return rval;
}

void LineTokenReader::putBack(const vector<string>& tokens) {
  mPushbacks.push(tokens);
}

void LineTokenReader::putBack(vector<string>&& tokens) {
  mPushbacks.push(std::move(tokens));
}

int LineTokenReader::getRawLineNumber() const {
  return mRawLineNumber;
}

bool LineTokenReader::isGood() const {
  return mGood && (*mStream);
}

}  // namespace Tokenizer
