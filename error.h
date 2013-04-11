#ifndef BLIF_VERIFIER_ERROR_H
#define BLIF_VERIFIER_ERROR_H

#include <iosfwd>
#include <string>

namespace blifverifier {

// Top-level error for the namespace/project.
struct Error {
  // Generate a human-readable description of the error.
  virtual void describe(std::ostream& os) const = 0;
};

// Abstract errors to simplify catching classes of errors;
struct IOError : public Error { };

struct ParseError : public Error {
  explicit ParseError(int line);
  int mLine;
};

struct LogicError : public Error { };

// Actual errors
struct BadInputStreamError : IOError {
  void describe(std::ostream& os) const override;
};

struct DuplicateBlockError : ParseError {
  DuplicateBlockError(int line, const std::string& block);
  void describe(std::ostream& os) const override;
  const std::string mBlock;
};

struct NamesBlockBeforeHeadersError : ParseError {
  NamesBlockBeforeHeadersError(int line, const std::string& block);
  const std::string mMissing;
  void describe(std::ostream& os) const override;
};

struct DuplicateTruthTableError : ParseError {
  DuplicateTruthTableError(int line, const std::string& name);
  const std::string mName;
  void describe(std::ostream& os) const override;
};

struct UnrecognizedSectionError : ParseError {
  UnrecognizedSectionError(int line, const std::string& section);
  void describe(std::ostream& os) const override;
  const std::string mSection;
};

struct MissingLogicDependencyError : LogicError {
  MissingLogicDependencyError(const std::string& tt, const std::string& name);
  const std::string mTruthtable;
  const std::string mInput;
  void describe(std::ostream& os) const override;
};

struct UndefinedPrimaryOutputError : LogicError {
  explicit UndefinedPrimaryOutputError(const std::string& name);
  const std::string mOutput;
  void describe(std::ostream& os) const override;
};

struct CircularDependencyError : LogicError {
  explicit CircularDependencyError(const std::string& name1,
                                   const std::string& name2);
  void describe(std::ostream& os) const override;
  const std::string mName1, mName2;
};

}  // namespace blifverifier

#endif
