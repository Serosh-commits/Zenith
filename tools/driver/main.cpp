#include "zenith/Parse/Parser.h"
#include "zenith/Sema/Sema.h"
#include "zenith/AST/ASTContext.h"
#include "zenith/Basic/SourceManager.h"
#include "zenith/Basic/Diagnostic.h"
#include "zenith/Lex/Preprocessor.h"
#include "zenith/Basic/TargetInfo.h"
#include "llvm/Support/raw_ostream.h"
#include <iostream>
#include <string>

using namespace zenith;

void DumpStmt(Stmt *S, unsigned Indent) {
    if (!S) return;
    std::string Ind(Indent * 2, ' ');
    switch (S->getKind()) {
    case StmtKind::Compound: {
        auto *CS = static_cast<CompoundStmt*>(S);
        std::cout << Ind << "CompoundStmt\n";
        for (auto *Sub : CS->body()) DumpStmt(Sub, Indent + 1);
        break;
    }
    case StmtKind::Return: {
        auto *RS = static_cast<ReturnStmt*>(S);
        std::cout << Ind << "ReturnStmt\n";
        if (RS->getRetValue()) DumpStmt(RS->getRetValue(), Indent + 1);
        break;
    }
    case StmtKind::If: {
        auto *IS = static_cast<IfStmt*>(S);
        std::cout << Ind << "IfStmt\n";
        DumpStmt(IS->getCond(), Indent + 1);
        DumpStmt(IS->getThen(), Indent + 1);
        if (IS->getElse()) DumpStmt(IS->getElse(), Indent + 1);
        break;
    }
    case StmtKind::DeclStmt: {
        auto *DS = static_cast<DeclStmt*>(S);
        std::cout << Ind << "DeclStmt\n";
        break;
    }
    case StmtKind::BinaryOperatorKind: {
        auto *BO = static_cast<BinaryOperator*>(S);
        std::cout << Ind << "BinaryOperator\n";
        DumpStmt(BO->getLHS(), Indent + 1);
        DumpStmt(BO->getRHS(), Indent + 1);
        break;
    }
    case StmtKind::IntegerLiteralKind: {
        auto *IL = static_cast<IntegerLiteral*>(S);
        std::cout << Ind << "IntegerLiteral " << IL->getValue() << "\n";
        break;
    }
    case StmtKind::DeclRefExprKind: {
        auto *DRE = static_cast<DeclRefExpr*>(S);
        std::cout << Ind << "DeclRefExpr " << (DRE->getDecl() ? DRE->getDecl()->getName().str() : "") << "\n";
        break;
    }
    default:
        std::cout << Ind << "Stmt\n";
        break;
    }
}

void DumpDecl(Decl *D, unsigned Indent) {
    if (!D) return;
    std::string Ind(Indent * 2, ' ');
    if (auto *FD = dynamic_cast<FunctionDecl*>(D)) {
        std::cout << Ind << "FunctionDecl " << FD->getName().str() << "\n";
        if (FD->getBody()) DumpStmt(FD->getBody(), Indent + 1);
    } else if (auto *VD = dynamic_cast<VarDecl*>(D)) {
        std::cout << Ind << "VarDecl " << VD->getName().str() << "\n";
        if (VD->getInit()) DumpStmt(VD->getInit(), Indent + 1);
    }
}

int main(int argc, char **argv) {
    bool dumpAST = false;
    bool dumpTokens = false;
    std::string filename;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--dump-ast") dumpAST = true;
        else if (arg == "--dump-tokens") dumpTokens = true;
        else if (arg == "--help") {
            std::cout << "Usage: zenithc [options] <filename>\n";
            std::cout << "Options:\n";
            std::cout << "  --dump-tokens  Print token stream\n";
            std::cout << "  --dump-ast     Print Abstract Syntax Tree\n";
            return 0;
        } else if (arg[0] != '-') {
            filename = arg;
        }
    }

    if (filename.empty()) {
        std::cerr << "zenithc: error: no input files\n";
        return 1;
    }

    FileManager FM;
    FileEntry *FE = FM.getFileRef(filename);
    if (!FE) {
        std::cerr << "zenithc: error: cannot open '" << filename << "'\n";
        return 1;
    }

    TextDiagnosticPrinter Printer(::llvm::errs());
    DiagnosticsEngine Diags;
    Diags.setConsumer(&Printer);

    SourceManager SM(FM, Diags);
    FileID MainFID = SM.createFileID(FE, SourceLocation());
    SM.setMainFileID(MainFID);

    LangOptions LangOpts;
    HeaderSearch Headers(FM);
    Preprocessor PP(LangOpts, SM, Diags, Headers, FM);
    PP.EnterMainSourceFile(MainFID);

    if (dumpTokens) {
        std::cout << "=== Token Stream ===\n";
        Token Tok;
        do {
            PP.Lex(Tok);
            std::cout << tok::getTokenName(Tok.getKind());
            if (Tok.is(tok::identifier)) {
                std::cout << " (" << Tok.getIdentifierInfo()->getName().str() << ")";
            } else if (Tok.is(tok::raw_identifier)) {
                std::cout << " (" << Tok.getRawIdentifier().str() << ")";
            } else if (Tok.isLiteral()) {
                std::cout << " (" << Lexer::getSpelling(Tok, SM, LangOpts).str() << ")";
            }
            std::cout << "\n";
        } while (Tok.isNot(tok::eof));
        return 0;
    }

    auto TargetPtr = TargetInfo::CreateTargetInfo();
    TargetInfo &Target = *TargetPtr;
    ASTContext Context(Target);
    Sema Actions(Context, Diags, SM, PP);
    Parser P(PP, Actions, Diags);

    std::cout << "=== Parsing Translation Unit ===\n";
    P.ParseTranslationUnit();

    if (dumpAST || true) {
        std::cout << "=== AST Dump ===\n";
        TranslationUnitDecl *TU = Context.getTranslationUnitDecl();
        std::cout << "Decls count: " << TU->decls().size() << "\n";
        for (Decl *D : TU->decls()) {
            DumpDecl(D, 0);
        }
    }

    return 0;
}
