//--------------------------------------------------------------------*- C++ -*-
// CLING - the C++ LLVM-based InterpreterG :)
// version: $Id$
// author:  Vassil Vassilev <vasil.georgiev.vasilev@cern.ch>
//------------------------------------------------------------------------------

#ifndef CLING_DECL_COLLECTOR_H
#define CLING_DECL_COLLECTOR_H

#include "clang/AST/ASTConsumer.h"

namespace clang {
  class ASTContext;
  class CodeGenerator;
  class Decl;
  class DeclGroupRef;
}

namespace cling {

  class Interpreter;
  class Transaction;

  ///\brief Collects declarations and fills them in cling::Transaction.
  ///
  /// cling::Transaction becomes is a main building block in the interpreter. 
  /// cling::DeclCollector is responsible for appending all the declarations 
  /// seen by clang.
  ///
  class DeclCollector: public clang::ASTConsumer {
  private:
    Transaction* m_CurTransaction;

    ///\brief This is the fast path for the declarations which do not need 
    /// special handling. Eg. deserialized declarations.
    clang::CodeGenerator* m_CodeGen; // we do not own.

    ///\brief Interpreter feeding into the DeclCollector. Only used for
    /// a temporary workaround until modules work.
    /// FIXME: remove once modules work.
    cling::Interpreter* m_Interp; // we do not own.

    ///\brief Test whether the first decl of the DeclGroupRef comes from an AST
    /// file.
    bool comesFromASTReader(clang::DeclGroupRef DGR) const;
    /// \brief Return true if this decl (which comes from an AST file) should
    // not be sent to CodeGen. The module is assumed to describe the contents of
    // a library; symbols inside the library must thus not be reemitted /
    // duplicated by CodeGen.
    bool shouldIgnoreDeclFromASTReader(const clang::Decl* D) const;

  public:
    DeclCollector() :
      m_CurTransaction(0), m_CodeGen(0), m_Interp(0) {}
    virtual ~DeclCollector();

    // FIXME: Gross hack, which should disappear when we move some of the 
    // initialization happening in the IncrementalParser to the CIFactory.
    void setCodeGen(clang::CodeGenerator* codeGen) { m_CodeGen = codeGen; }

    // FIXME: Gross hack, which should disappear when m_Interp can go.
    void setInterpreter(cling::Interpreter* Interp) { m_Interp = Interp; }

    /// \{
    /// \name ASTConsumer overrides

    virtual bool HandleTopLevelDecl(clang::DeclGroupRef DGR);
    virtual void HandleInterestingDecl(clang::DeclGroupRef DGR);
    virtual void HandleTagDeclDefinition(clang::TagDecl* TD);
    virtual void HandleVTable(clang::CXXRecordDecl* RD,
                              bool DefinitionRequired);
    virtual void CompleteTentativeDefinition(clang::VarDecl* VD);
    virtual void HandleTranslationUnit(clang::ASTContext& Ctx);
    virtual void HandleCXXImplicitFunctionInstantiation(clang::FunctionDecl *D);
    virtual void HandleCXXStaticMemberVarInstantiation(clang::VarDecl *D);
    /// \}

    /// \{
    /// \name Transaction Support

    Transaction* getTransaction() { return m_CurTransaction; }
    const Transaction* getTransaction() const { return m_CurTransaction; }
    void setTransaction(Transaction* curT) { m_CurTransaction = curT; }
    void setTransaction(const Transaction* curT) { 
      m_CurTransaction = const_cast<Transaction*>(curT); 
    }

    /// \}

    // dyn_cast/isa support
    static bool classof(const clang::ASTConsumer*) { return true; }
  };
} // namespace cling

#endif // CLING_DECL_COLLECTOR_H
