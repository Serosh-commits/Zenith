#include "zenith/Lex/Preprocessor.h"
#include "llvm/Support/MemoryBuffer.h"

namespace zenith {

Preprocessor::Preprocessor(LangOptions &LangOpts, SourceManager &SourceMgr, DiagnosticsEngine &Diags, HeaderSearch &Headers, FileManager &FileMgr)
    : LangOpts(LangOpts), SourceMgr(SourceMgr), Diags(Diags), Headers(Headers), FileMgr(FileMgr) {
    Identifiers.AddKeywords(LangOpts);
}

Preprocessor::~Preprocessor() {
    for (auto &Entry : Macros) {
        delete Entry.getValue();
    }
}

MacroInfo *Preprocessor::getMacroInfo(IdentifierInfo *II) const {
    if (!II) return nullptr;
    auto It = Macros.find(II->getName());
    if (It != Macros.end())
        return It->second;
    return nullptr;
}

void Preprocessor::setMacroInfo(IdentifierInfo *II, MacroInfo *MI) {
    if (!II) return;
    Macros[II->getName()] = MI;
}

void Preprocessor::EnterMainSourceFile(FileID FID) {
    SourceMgr.setMainFileID(FID);
    const ::llvm::MemoryBuffer *Buffer = SourceMgr.getBuffer(FID);
    if (!Buffer) return;

    auto L = std::make_unique<Lexer>(FID, Buffer, SourceMgr, LangOpts);
    L->setIdentifierTable(&Identifiers);
    CurLexer = L.get();
    IncludeStack.push_back(std::move(L));
}

void Preprocessor::EnterSourceFile(FileID FID, SourceLocation IncludeLoc) {
    const ::llvm::MemoryBuffer *Buffer = SourceMgr.getBuffer(FID);
    if (!Buffer) return;

    auto L = std::make_unique<Lexer>(FID, Buffer, SourceMgr, LangOpts);
    L->setIdentifierTable(&Identifiers);
    CurLexer = L.get();
    IncludeStack.push_back(std::move(L));
}

void Preprocessor::Lex(Token &Result) {
    bool NeedLex = true;
    while (true) {
        if (drainTokenQueue(Result))
            return;
        if (!CurLexer) {
            Result.startToken();
            Result.setKind(tok::eof);
            return;
        }

        if (NeedLex) {
            CurLexer->Lex(Result);
        }
        NeedLex = true;

        if (Result.is(tok::eof)) {
            IncludeStack.pop_back();
            if (IncludeStack.empty()) {
                CurLexer = nullptr;
                return;
            } else {
                CurLexer = IncludeStack.back().get();
                continue;
            }
        }

        if (Result.isAtStartOfLine() && Result.is(tok::hash)) {
            NeedLex = HandleDirective(Result);
            continue;
        }

        if (SkippingUntilDirective)
            continue;

        if (Result.is(tok::identifier)) {
            IdentifierInfo *II = Result.getIdentifierInfo();
            if (MacroInfo *MI = getMacroInfo(II)) {
                if (!MI->isDisabled()) {
                    if (ExpandMacro(Result, II, MI))
                        continue;
                }
            }
        }

        return;
    }
}

bool Preprocessor::HandleDirective(Token &Result) {
    if (!CurLexer) return true;
    CurLexer->Lex(Result);
    ::llvm::StringRef DirName = Lexer::getSpelling(Result, SourceMgr, LangOpts);

    if (DirName == "define") {
        if (!SkippingUntilDirective)
            HandleDefineDirective(Result);
        return false;
    } else if (DirName == "undef") {
        if (!SkippingUntilDirective)
            HandleUndefDirective(Result);
        return false;
    } else if (DirName == "include") {
        if (!SkippingUntilDirective)
            HandleIncludeDirective(Result);
        return true;
    } else if (DirName == "ifdef") {
        HandleIfdefDirective(Result, false);
        return true;
    } else if (DirName == "ifndef") {
        HandleIfdefDirective(Result, true);
        return true;
    } else if (DirName == "if") {
        HandleIfDirective(Result);
        return true;
    } else if (DirName == "else") {
        HandleElseDirective(Result);
        return true;
    } else if (DirName == "elif") {
        HandleElifDirective(Result);
        return true;
    } else if (DirName == "endif") {
        HandleEndifDirective(Result);
        return true;
    }
    return true;
}

void Preprocessor::SkipExcludedConditionalBlock() {
    unsigned Depth = 0;
    Token Tok;
    while (true) {
        if (!CurLexer) break;
        CurLexer->Lex(Tok);
        if (Tok.is(tok::eof)) break;

        if (Tok.isAtStartOfLine() && Tok.is(tok::hash)) {
            CurLexer->Lex(Tok);
            ::llvm::StringRef DirName = Lexer::getSpelling(Tok, SourceMgr, LangOpts);

            if (DirName == "if" || DirName == "ifdef" || DirName == "ifndef") {
                ++Depth;
            } else if (DirName == "endif") {
                if (Depth == 0) {
                    PPConditionalInfo CI;
                    CurLexer->popConditionalLevel(CI);
                    SkippingUntilDirective = CI.WasSkipping;
                    break;
                } else {
                    --Depth;
                }
            } else if (DirName == "else") {
                if (Depth == 0) {
                    PPConditionalInfo *Cond = const_cast<PPConditionalInfo*>(CurLexer->peekConditionalLevel());
                    if (Cond && !Cond->FoundNonSkip) {
                        Cond->FoundNonSkip = true;
                        SkippingUntilDirective = false;
                        break;
                    } else {
                        SkippingUntilDirective = true;
                    }
                }
            } else if (DirName == "elif" || DirName == "elifdef" || DirName == "elifndef") {
                if (Depth == 0) {
                    PPConditionalInfo *Cond = const_cast<PPConditionalInfo*>(CurLexer->peekConditionalLevel());
                    if (Cond && !Cond->FoundNonSkip) {
                        Cond->FoundNonSkip = true;
                        SkippingUntilDirective = false;
                        break;
                    }
                }
            }
        }
    }
}

void Preprocessor::HandleDefineDirective(Token &Result) {
    CurLexer->Lex(Result);
    if (Result.isNot(tok::identifier) && Result.isNot(tok::raw_identifier)) return;

    IdentifierInfo *MacroName = Result.getIdentifierInfo();
    if (!MacroName && Result.is(tok::raw_identifier)) {
        MacroName = &Identifiers.get(Result.getRawIdentifier());
    }

    MacroInfo *MI = new MacroInfo(Result.getLocation());

    CurLexer->Lex(Result);
    if (Result.is(tok::l_paren) && !Result.hasLeadingSpace()) {
        MI->IsObjectLike = false;
        MI->IsFunctionLike = true;

        while (true) {
            CurLexer->Lex(Result);
            if (Result.is(tok::r_paren) || Result.is(tok::eof) || Result.is(tok::eod)) break;

            if (Result.is(tok::identifier) || Result.is(tok::raw_identifier)) {
                IdentifierInfo *PI = Result.getIdentifierInfo();
                if (!PI && Result.is(tok::raw_identifier)) {
                    PI = &Identifiers.get(Result.getRawIdentifier());
                }
                if (PI)
                    MI->Params.push_back(PI);
            }

            if (Result.is(tok::comma)) continue;
        }

        while (true) {
            CurLexer->Lex(Result);
            if (Result.is(tok::eod) || Result.is(tok::eof) || Result.isAtStartOfLine()) break;
            Token Copy = Result;
            if (Copy.is(tok::raw_identifier)) {
                ::llvm::StringRef Raw = Copy.getRawIdentifier();
                IdentifierInfo &II = Identifiers.get(Raw);
                Copy.setKind(tok::identifier);
                Copy.setIdentifierInfo(&II);
            }
            MI->ReplacementTokens.push_back(Copy);
        }
    } else {
        while (true) {
            if (Result.is(tok::eod) || Result.is(tok::eof) || Result.isAtStartOfLine()) break;
            Token Copy = Result;
            if (Copy.is(tok::raw_identifier)) {
                ::llvm::StringRef Raw = Copy.getRawIdentifier();
                IdentifierInfo &II = Identifiers.get(Raw);
                Copy.setKind(tok::identifier);
                Copy.setIdentifierInfo(&II);
            }
            MI->ReplacementTokens.push_back(Copy);
            CurLexer->Lex(Result);
        }
    }

    setMacroInfo(MacroName, MI);
}

void Preprocessor::HandleUndefDirective(Token &Result) {
    CurLexer->Lex(Result);
    if (Result.isNot(tok::identifier) && Result.isNot(tok::raw_identifier)) return;

    ::llvm::StringRef Name = Result.is(tok::identifier) ?
        Result.getIdentifierInfo()->getName() : Result.getRawIdentifier();

    auto It = Macros.find(Name);
    if (It != Macros.end()) {
        delete It->second;
        Macros.erase(It);
    }

    while (true) {
        CurLexer->Lex(Result);
        if (Result.is(tok::eod) || Result.is(tok::eof) || Result.isAtStartOfLine()) break;
    }
}

void Preprocessor::HandleIncludeDirective(Token &Result) {
    CurLexer->Lex(Result);
    bool IsAngled = false;
    std::string Filename;

    if (Result.is(tok::less)) {
        IsAngled = true;
        while (true) {
            CurLexer->Lex(Result);
            if (Result.is(tok::greater) || Result.is(tok::eof) || Result.is(tok::eod)) break;
            if (Result.is(tok::identifier) || Result.is(tok::raw_identifier)) {
                Filename += Result.getRawIdentifier().str();
            } else if (Result.is(tok::period)) {
                Filename += ".";
            } else if (Result.is(tok::slash)) {
                Filename += "/";
            }
        }
    } else if (Result.is(tok::string_literal)) {
        Filename = Result.getLiteralData();
        if (Filename.size() >= 2 && Filename.front() == '"' && Filename.back() == '"') {
            Filename = Filename.substr(1, Filename.size() - 2);
        }
    }

    if (!Filename.empty()) {
        auto FullPath = Headers.LookupFile(Filename, IsAngled);
        if (FullPath) {
            FileEntry *FE = FileMgr.getFileRef(*FullPath);
            if (FE) {
                FileID FID = SourceMgr.createFileID(FE, Result.getLocation());
                EnterSourceFile(FID, Result.getLocation());
            }
        }
    }
}

void Preprocessor::HandleIfdefDirective(Token &Result, bool isIfndef) {
    if (!CurLexer) return;
    CurLexer->Lex(Result);
    if (Result.isNot(tok::identifier) && Result.isNot(tok::raw_identifier)) return;

    IdentifierInfo *II = Result.getIdentifierInfo();
    if (!II && Result.is(tok::raw_identifier)) {
        II = &Identifiers.get(Result.getRawIdentifier());
    }

    MacroInfo *MI = getMacroInfo(II);
    bool Condition = MI != nullptr;
    if (isIfndef) Condition = !Condition;

    CurLexer->pushConditionalLevel(Result.getLocation(), SkippingUntilDirective, Condition, false);
    if (!Condition) {
        SkippingUntilDirective = true;
        SkipExcludedConditionalBlock();
    }
}

void Preprocessor::HandleIfDirective(Token &Result) {
    if (!CurLexer) return;
    CurLexer->pushConditionalLevel(Result.getLocation(), SkippingUntilDirective, false, false);
    SkippingUntilDirective = true;
    SkipExcludedConditionalBlock();
}

void Preprocessor::HandleElseDirective(Token &Result) {
    if (!CurLexer) return;
    PPConditionalInfo *Cond = const_cast<PPConditionalInfo*>(CurLexer->peekConditionalLevel());
    if (Cond) {
        if (Cond->FoundNonSkip) {
            SkippingUntilDirective = true;
            SkipExcludedConditionalBlock();
        } else {
            Cond->FoundNonSkip = true;
            SkippingUntilDirective = false;
        }
    }
}

void Preprocessor::HandleElifDirective(Token &Result) {
    if (!CurLexer) return;
    PPConditionalInfo *Cond = const_cast<PPConditionalInfo*>(CurLexer->peekConditionalLevel());
    if (Cond) {
        if (Cond->FoundNonSkip) {
            SkippingUntilDirective = true;
            SkipExcludedConditionalBlock();
        } else {
            Cond->FoundNonSkip = true;
            SkippingUntilDirective = false;
        }
    }
}

void Preprocessor::HandleEndifDirective(Token &Result) {
    if (!CurLexer) return;
    PPConditionalInfo CI;
    if (!CurLexer->popConditionalLevel(CI)) {
        SkippingUntilDirective = CI.WasSkipping;
    }
}

bool Preprocessor::ExpandMacro(Token &Identifier, IdentifierInfo *II, MacroInfo *MI) {
    if (MI->tokens().empty())
        return false;

    MI->setDisabled(true);
    
    std::vector<std::vector<Token>> Args;
    if (MI->isFunctionLike()) {
        Token Next;
        CurLexer->Lex(Next);
        if (!Next.is(tok::l_paren) || Next.hasLeadingSpace()) {
            
            TokenQueue.push_front(Next);
            MI->setDisabled(false);
            return false;
        }
        if (!parseInvocationArgs(Args)) {
            MI->setDisabled(false);
            return false;
        }
        preExpandArgs(Args);
    }

    
    enqueueReplacementTokens(MI, Identifier, Args);

    MI->setDisabled(false);

    return true;
}

bool Preprocessor::drainTokenQueue(Token &Result) {
    while (!TokenQueue.empty()) {
        Token Front = TokenQueue.front();
        if (Front.is(tok::identifier) || Front.is(tok::raw_identifier)) {
            IdentifierInfo *II = Front.getIdentifierInfo();
            if (!II && Front.is(tok::raw_identifier))
                II = &Identifiers.get(Front.getRawIdentifier());
            if (II) {
                if (MacroInfo *MI = getMacroInfo(II)) {
                    if (!MI->isDisabled() && MI->isObjectLike()) {
                        TokenQueue.pop_front();
                        if (ExpandMacro(Front, II, MI))
                            continue;
                    }
                }
            }
        }

        Result = TokenQueue.front();
        TokenQueue.pop_front();
        return true;
    }
    return false;
}

bool Preprocessor::parseInvocationArgs(std::vector<std::vector<Token>> &Args) {
    unsigned Depth = 0;
    Token Next;
    Args.emplace_back();
    while (true) {
        if (!CurLexer->Lex(Next))
            return false;
        if (Next.is(tok::eof))
            return false;

        if (Next.is(tok::l_paren)) {
            Depth++;
            Args.back().push_back(Next);
            continue;
        }
        if (Next.is(tok::r_paren)) {
            if (Depth == 0) {
                break;
            }
            Depth--;
            Args.back().push_back(Next);
            continue;
        }
        if (Depth == 0 && Next.is(tok::comma)) {
            Args.emplace_back();
            continue;
        }
        Args.back().push_back(Next);
    }
    return true;
}

void Preprocessor::preExpandArgs(std::vector<std::vector<Token>> &Args) {
    for (auto &Arg : Args) {
        std::vector<Token> Expanded;
        for (size_t ti = 0; ti < Arg.size(); ++ti) {
            Token T = Arg[ti];
            // i know this feels a bit of ugly i will split it up to helpers in follow up PRs
            if (T.is(tok::identifier) || T.is(tok::raw_identifier)) {
                IdentifierInfo *AII = T.getIdentifierInfo();
                if (!AII && T.is(tok::raw_identifier))
                    AII = &Identifiers.get(T.getRawIdentifier());
                if (AII) {
                    if (MacroInfo *AMI = getMacroInfo(AII)) {
                        if (!AMI->isDisabled() && AMI->isObjectLike()) {
                            if (ExpandMacro(T, AII, AMI)) {
                                while (!TokenQueue.empty()) {
                                    Expanded.push_back(TokenQueue.front());
                                    TokenQueue.pop_front();
                                }
                                continue;
                            }
                        }
                    }
                }
            }
            Expanded.push_back(T);
        }
        Arg.swap(Expanded);
    }
}

void Preprocessor::enqueueReplacementTokens(MacroInfo *MI, Token &Identifier, const std::vector<std::vector<Token>> &Args) {
    for (size_t i = 0; i < MI->tokens().size(); ++i) {
        const Token &T = MI->tokens()[i];

        if (MI->isFunctionLike() && (T.is(tok::identifier) || T.is(tok::raw_identifier))) {
            IdentifierInfo *PI = T.getIdentifierInfo();
            ::llvm::StringRef RawName;
            if (!PI && T.is(tok::raw_identifier))
                RawName = T.getRawIdentifier();

            bool Substituted = false;
            for (size_t p = 0; p < MI->params().size(); ++p) {
                if ((PI && MI->params()[p] == PI) || (!PI && !RawName.empty() && MI->params()[p]->getName() == RawName)) {
                    if (p < Args.size()) {
                        for (const Token &AT : Args[p]) {
                            Token Copy = AT;
                            Copy.setLocation(Identifier.getLocation());
                            Copy.clearFlag(Token::StartOfLine);
                            Copy.clearFlag(Token::LeadingSpace);
                            TokenQueue.push_back(Copy);
                        }
                    }
                    Substituted = true;
                    break;
                }
            }
            if (Substituted) continue;
        }

        Token Copy = T;
        Copy.setLocation(Identifier.getLocation());

        Copy.clearFlag(Token::StartOfLine);
        Copy.clearFlag(Token::LeadingSpace);

        if (i == 0) {
            if (Identifier.isAtStartOfLine()) Copy.setFlag(Token::StartOfLine);
            if (Identifier.hasLeadingSpace()) Copy.setFlag(Token::LeadingSpace);
        }

        TokenQueue.push_back(Copy);
    }
}

}
