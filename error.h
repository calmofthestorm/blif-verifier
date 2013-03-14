#ifndef BLIF_VERIFIER_ERROR_H
#define BLIF_VERIFIER_ERROR_H

#include <iosfwd>

namespace blifverifier {

// Top-level error for the namespace/project.
struct Error {
  // Generate a human-readable description of the error.
  virtual void describe(std::ostream& os) const = 0;
};

// Abstract errors to simplify catching classes of errors;
struct IOError : public Error { };

struct ParseError : public Error {
  ParseError(int line);
  int mLine;
};

struct LogicError : public Error { };

// Actual errors
struct BadInputStreamError : IOError {
  void describe(std::ostream& os) const override;
};

struct DuplicateBlockError : ParseError {
  DuplicateBlockError(int line, const char* block);
  void describe(std::ostream& os) const override;
  const char* mBlock;
};

struct NamesBlockBeforeHeadersError : ParseError {
  NamesBlockBeforeHeadersError(int line, const char* block);
  const char* mMissing;
  void describe(std::ostream& os) const override;
};

struct MalformedTruthTableError : ParseError {
  MalformedTruthTableError(int line);
  void describe(std::ostream& os) const override;
};

struct UnrecognizedSectionError : ParseError {
  UnrecognizedSectionError(int line, const char* section);
  void describe(std::ostream& os) const override;
  const char* mSection;
};

struct MissingLogicDependencyError : LogicError {
  MissingLogicDependencyError(const char* tt, const char* name);
  const char* mTruthtable;
  const char* mInput;
  void describe(std::ostream& os) const override;
};

struct UndefinedPrimaryOutputError : LogicError {
  UndefinedPrimaryOutputError(const char* name);
  const char* mOutput;
  void describe(std::ostream& os) const override;
};

} // namespace blifverifier

#endif
