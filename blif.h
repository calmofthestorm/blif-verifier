#ifndef BLIF_VERIFIER_BLIF_H
#define BLIF_VERIFIER_BLIF_H

#include <iosfwd>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "truthtable.h"

namespace blifverifier {

class BLIF {
  public:
    // Constructs an empty BLIF (no inputs, outputs, names, logic, etc).
    BLIF();

    // Reads a BLIF from a stream.
    explicit BLIF(std::istream& input);
    explicit BLIF(std::istream&& input);

    // Writes out a C function that implements the BLIF's logic to the
    // provided ostream.
    void writeEvaluator(std::ostream& output,
                        const std::string& fxn_name) const;

    // Returns a set of the names of the node's inputs.
    const std::vector<std::string>& getPrimaryInputs() const;

    // Returns a set of the names of the node's outputs.
    const std::vector<std::string>& getPrimaryOutputs() const;

    // Returns true if the circuits are trivially not equivalent
    // (eg, different inputs) and prints warning messages
    // to the provided stream.
    bool triviallyNotEquivalent(const BLIF& other,
                                std::ostream& warnings) const;

  private:
    // Reads a single line from the stream, and returns a vector of strings.
    // Will strip comments denoted by #, and any lines with no tokens.
    static std::vector<std::string> readLineAsTokens(std::istream& input);

    // Generate a unique name for this user node (to resolve naming conflicts)
    int registerLiteral(const std::string& lit);

    // Name of the model
    std::string mModel;

    // A vector of the primary inputs names.
    std::vector<std::string> mPrimaryInputs;

    // A vector of the primary outputs names.
    std::vector<std::string> mPrimaryOutputs;

    // Bimap from user supplied names to the ones we'll use.
    std::unordered_map<std::string, int> mLiterals;
    std::unordered_map<int, std::string> mLiteralsReverse;

    // Next integer to use for unique generated names.
    int mNextLiteralIndex;

    // Map from node names (truth tables) to the truth tables that represent
    // them.
    std::unordered_map<int, TruthTable> mTruthTables;
};

}  // namespace blifverifier

#endif  // BLIF_VERIFIER_BLIF_H
