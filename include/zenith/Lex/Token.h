#pragma once

#include "zenith/Basic/SourceLocation.h"
#include "zenith/Lex/TokenKinds.h"
#include "llvm/ADT/StringRef.h"
#include <cassert>

namespace zenith {

class IdentifierInfo;

class Token {
    SourceLocation::UIntTy Loc = 0;
    SourceLocation::UIntTy UintData = 0;
    void *PtrData = nullptr;
    tok::TokenKind Kind = tok::unknown;
    unsigned short Flags = 0;

public:
    enum TokenFlags {
        StartOfLine                 = 0x01,
        LeadingSpace                = 0x02,
        DisableExpand               = 0x04,
        NeedsCleaning               = 0x08,
        LeadingEmptyMacro           = 0x10,
        HasUDSuffix                 = 0x20,
        HasUCN                      = 0x40,
        IgnoredComma                = 0x80,
        StringifiedInMacro          = 0x100,
        CommaAfterElided            = 0x200,
        IsEditorPlaceholder         = 0x400,
        IsReinjected                = 0x800,
        HasSeenNoTrivialPPDirective = 0x1000,
        PhysicalStartOfLine         = 0x2000
    };

    void startToken() {
        Kind = tok::unknown;
        Flags = 0;
        PtrData = nullptr;
        UintData = 0;
        Loc = 0;
    }

    tok::TokenKind getKind() const { return Kind; }
    void setKind(tok::TokenKind K) { Kind = K; }

    bool is(tok::TokenKind K) const { return Kind == K; }
    bool isNot(tok::TokenKind K) const { return Kind != K; }

    template <typename... Ts>
    bool isOneOf(Ts... Ks) const {
        static_assert(sizeof...(Ts) > 0, "requires at least one tok::TokenKind");
        return (is(Ks) || ...);
    }

    template <typename... Ts>
    bool isNoneOf(Ts... Ks) const {
        static_assert(sizeof...(Ts) > 0, "requires at least one tok::TokenKind");
        return (isNot(Ks) && ...);
    }

    bool isAnyIdentifier() const { return tok::isAnyIdentifier(getKind()); }
    bool isLiteral() const { return tok::isLiteral(getKind()); }
    bool isAnnotation() const { return tok::isAnnotation(getKind()); }

    SourceLocation getLocation() const {
        return SourceLocation::getFromRawEncoding(Loc);
    }
    void setLocation(SourceLocation L) {
        Loc = L.getRawEncoding();
    }

    unsigned getLength() const {
        assert(!isAnnotation() && "Annotation tokens have no length field");
        return UintData;
    }
    void setLength(unsigned Len) {
        assert(!isAnnotation() && "Annotation tokens have no length field");
        UintData = Len;
    }

    SourceLocation getAnnotationEndLoc() const {
        assert(isAnnotation() && "Cannot get annotation end loc of non-annotation");
        return SourceLocation::getFromRawEncoding(UintData ? UintData : Loc);
    }
    void setAnnotationEndLoc(SourceLocation L) {
        assert(isAnnotation() && "Cannot set annotation end loc of non-annotation");
        UintData = L.getRawEncoding();
    }

    SourceLocation getLastLoc() const {
        return isAnnotation() ? getAnnotationEndLoc() : getLocation();
    }

    SourceLocation getEndLoc() const {
        return isAnnotation() ? getAnnotationEndLoc()
                              : getLocation().getLocWithOffset(getLength());
    }

    IdentifierInfo *getIdentifierInfo() const {
        assert(isNot(tok::raw_identifier) && "Use getRawIdentifier() instead");
        assert(!isAnnotation() && "Cannot get identifier info of annotation token");
        if (isLiteral() || is(tok::eof)) return nullptr;
        return reinterpret_cast<IdentifierInfo*>(PtrData);
    }
    void setIdentifierInfo(IdentifierInfo *II) {
        assert(isNot(tok::raw_identifier));
        assert(!isAnnotation());
        PtrData = II;
    }

    ::llvm::StringRef getRawIdentifier() const {
        assert(is(tok::raw_identifier));
        return ::llvm::StringRef(reinterpret_cast<const char*>(PtrData), getLength());
    }
    void setRawIdentifierData(const char *Data) {
        assert(is(tok::raw_identifier));
        PtrData = const_cast<char*>(Data);
    }

    const char *getLiteralData() const {
        assert(isLiteral() && "Cannot get literal data of non-literal");
        return reinterpret_cast<const char*>(PtrData);
    }
    void setLiteralData(const char *Data) {
        assert(isLiteral());
        PtrData = const_cast<char*>(Data);
    }

    void *getAnnotationValue() const {
        assert(isAnnotation() && "Cannot get annotation value of non-annotation");
        return PtrData;
    }
    void setAnnotationValue(void *Val) {
        assert(isAnnotation());
        PtrData = Val;
    }

    const void *getEofData() const {
        assert(is(tok::eof));
        return PtrData;
    }
    void setEofData(const void *D) {
        assert(is(tok::eof));
        PtrData = const_cast<void*>(D);
    }

    ::llvm::StringRef getName() const;

    unsigned getFlags() const { return Flags; }
    void setFlag(TokenFlags Flag) { Flags |= Flag; }
    void clearFlag(TokenFlags Flag) { Flags &= ~Flag; }
    bool getFlag(TokenFlags Flag) const { return (Flags & Flag) != 0; }

    void setFlagValue(TokenFlags Flag, bool Val) {
        if (Val) setFlag(Flag); else clearFlag(Flag);
    }

    bool isAtStartOfLine() const { return getFlag(StartOfLine); }
    bool isAtPhysicalStartOfLine() const { return getFlag(PhysicalStartOfLine); }
    bool hasLeadingSpace() const { return getFlag(LeadingSpace); }
    bool isExpandDisabled() const { return getFlag(DisableExpand); }
    bool needsCleaning() const { return getFlag(NeedsCleaning); }
    bool hasLeadingEmptyMacro() const { return getFlag(LeadingEmptyMacro); }
    bool hasUDSuffix() const { return getFlag(HasUDSuffix); }
    bool hasUCN() const { return getFlag(HasUCN); }
    bool stringifiedInMacro() const { return getFlag(StringifiedInMacro); }
    bool commaAfterElided() const { return getFlag(CommaAfterElided); }
    bool isEditorPlaceholder() const { return getFlag(IsEditorPlaceholder); }
};

struct PPConditionalInfo {
    SourceLocation IfLoc;
    bool WasSkipping = false;
    bool FoundNonSkip = false;
    bool FoundElse = false;
};

}
