#pragma once

namespace zenith {

struct LangOptions {
    bool CPlusPlus = false;
    bool CPlusPlus11 = false;
    bool CPlusPlus17 = false;
    bool CPlusPlus20 = false;
    bool C99 = true;
    bool C11 = false;
    bool Trigraphs = false;
    bool LineComment = true;
    bool Bool = false;
    bool HexFloats = true;
    bool Digraphs = false;

    LangOptions() = default;

    bool isCPlusPlus() const { return CPlusPlus || CPlusPlus11 || CPlusPlus17 || CPlusPlus20; }
    bool isCPlusPlus11() const { return CPlusPlus11 || CPlusPlus17 || CPlusPlus20; }
    bool isCPlusPlus17() const { return CPlusPlus17 || CPlusPlus20; }
    bool isCPlusPlus20() const { return CPlusPlus20; }
};

}
