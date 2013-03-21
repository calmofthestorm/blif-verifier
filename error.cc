#include "error.h"

#include <iostream>

namespace blifverifier {

using std::endl;
using std::ostream;
using std::string;

ParseError::ParseError(int line)
  : mLine(line) { }

void BadInputStreamError::describe(ostream& os) const  {
  os << "Bad input stream provided to parser." << endl;
}

DuplicateBlockError::DuplicateBlockError(int line, const string& block)
  : ParseError(line), mBlock(block) { }

void DuplicateBlockError::describe(ostream& os) const  {
  os << "Parse error: only one " << mBlock << " block per circuit. "
     << "Second definition at line " << mLine << endl;
}

NamesBlockBeforeHeadersError::NamesBlockBeforeHeadersError(int line,
                                                           const string& block)
  : ParseError(line), mMissing(block) { }

void NamesBlockBeforeHeadersError::describe(ostream& os) const  {
  os << "Parse error: Must read model, inputs, and outputs blocks before "
     << "any truth tables may be defined. Missing " << mMissing << " block, "
     << "but truth tables are defined at line " << mLine << endl;
}

DuplicateTruthTableError::DuplicateTruthTableError(int line,
                                                   const string& name)
  : ParseError(line), mName(name) { }

void DuplicateTruthTableError::describe(ostream& os) const {
  os << "Parse error: Redefinition of truth table named " << mName
     << " at line " << mLine << endl;
}

UnrecognizedSectionError::UnrecognizedSectionError(int line, const string& sect)
  : ParseError(line), mSection(sect) { }

void UnrecognizedSectionError::describe(ostream& os) const  {
  os << "Unrecognized section type '" << mSection << "' at line " << mLine
     << endl;
}

MissingLogicDependencyError::MissingLogicDependencyError(const string& tt,
                                                         const string& name)
  : mTruthtable(tt), mInput(name) { }

void MissingLogicDependencyError::describe(ostream& os) const  {
  os << "Input " << mInput << " to truth table defining " << mTruthtable
     << " is neither defined nor a primary input." << endl;
}

UndefinedPrimaryOutputError::UndefinedPrimaryOutputError(const string& name)
  : mOutput(name) { }

void UndefinedPrimaryOutputError::describe(ostream& os) const  {
  os << "Primary output " << mOutput << " is never defined." << endl;
}

}  // namespace blifverifier
