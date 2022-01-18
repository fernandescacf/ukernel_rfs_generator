#pragma once

#include <cstdint>
#include <fstream>
#include <string>
#include <array>
#include "token.h"


class Scanner
{
public:
	Scanner(const std::string& file) :
        m_file(file, std::fstream::in), m_line(""), m_linePos(0), m_lineNumber(0),
        m_tokenHex(Token::Value, Token::Hex), m_tokenDec(Token::Value, Token::Dec), m_tokenStr(Token::String, Token::Str) {}

    bool isFileValid() const;

    const Token& getToken();

    const Token& currentToken();

    bool hasDelimiter();

    bool hasSymbol();

    uint32_t getLineNumber() const;

    void dumpCurrentLine() const;

private:
    std::fstream m_file;
    std::string m_line;
    uint32_t m_linePos;
    uint32_t m_lineNumber;

    const std::array<Token, Token::TokensImplmented> m_lex
    {
        Token(Token::Section,   Token::Header,     "Header"),
        Token(Token::Section,   Token::Script,     "Script"),
        Token(Token::Section,   Token::Ram,        "Ram"),
        Token(Token::Section,   Token::Interrupts, "Interrupts"),
        Token(Token::Section,   Token::Devices,    "Devices"),
        Token(Token::Section,   Token::Files,      "Files"),
        Token(Token::Entry,     Token::Type,       "type"),
        Token(Token::Entry,     Token::Version,    "version"),
        Token(Token::Entry,     Token::Arch,       "arch"),
        Token(Token::Entry,     Token::Mach,       "mach"),
        Token(Token::Entry,     Token::Exec,       "exec"),
        Token(Token::Entry,     Token::Addr,       "address"),
        Token(Token::Entry,     Token::Size,       "size"),
        Token(Token::Entry,     Token::Name,       "name"),
        Token(Token::Entry,     Token::Private,    "private"),
        Token(Token::Entry,     Token::Shared,     "shared"),
        Token(Token::Entry,     Token::Source,     "source"),
        Token(Token::Value,     Token::PrivIO,     "_IO",         0x01),
        Token(Token::Value,     Token::PrivIO,     "_None",       0x00),
        Token(Token::Delimiter, Token::OpenBrace,  "{"),
        Token(Token::Delimiter, Token::CloseBrace, "}"),
        Token(Token::Symbol,    Token::None,       "$"),
        Token(Token::Invalid,   Token::EndOfFile),
        Token(Token::Invalid,   Token::None),
    };

    // Num lexical tokens indexs
    const uint32_t TokenHexIndex { Token::TokensImplmented + 0 };
    const uint32_t TokenDecIndex { Token::TokensImplmented + 1 };
    const uint32_t TokenStrIndex { Token::TokensImplmented + 2 };
    // Num lexical tokens
    Token m_tokenHex;
    Token m_tokenDec;
    Token m_tokenStr;

    uint32_t m_current { static_cast<uint32_t>(-1) };

    bool getNewLine();

    void removeLeadingSpaces();

    void removeTraillingSpaces();

    bool skipSpaces();

    uint32_t getCompareSize();

    bool convert2Hex(const char* str, uint32_t &hex);

    bool convert2Dec(const char* str, uint32_t &dec);

    const Token& matchToken();
};