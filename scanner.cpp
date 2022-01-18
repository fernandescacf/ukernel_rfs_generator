#include <cstring>
#include <memory>
#include <algorithm>
#include <iostream>
#include "scanner.h"


bool Scanner::isFileValid() const
{
    return m_file.is_open();
}

const Token& Scanner::getToken() 
{
    if(m_lineNumber == 0 || m_linePos == m_line.size())
    {
        if(!getNewLine())
        {
            return m_lex[Token::TokensImplmented - 2];
        }
    }
    return matchToken();
}

const Token& Scanner::currentToken()
{
    if(m_current < Token::TokensImplmented)
    {
        return m_lex[m_current];
    }
    else if(m_current == TokenHexIndex)
    {
        return m_tokenHex;
    }
    else if(m_current == TokenHexIndex)
    {
        return m_tokenDec;
    }
    else if(m_current == TokenHexIndex)
    {
        return m_tokenStr;
    }
    else
    {
        return m_lex[Token::TokensImplmented - 1];
    }
}

bool Scanner::hasDelimiter()
{
    if(m_linePos < m_line.size())
    {
        if(m_line.data()[m_linePos] == ' ' || m_line.data()[m_linePos] == '\t')
        {
            return true;
        }

        if(m_line.data()[m_linePos] == '}' || m_line.data()[m_linePos] == '{')
        {
            return true;
        }

        return false;
    }

    return true;
}

bool Scanner::hasSymbol()
{
    if(m_linePos < m_line.size() && m_line.data()[m_linePos] == '$')
    {
        return true;
    }

    return false;
}

uint32_t Scanner::getLineNumber() const
{
    return m_lineNumber;
}

void Scanner::dumpCurrentLine() const 
{
    std::cout << "line " << m_lineNumber << "[" << std::dec << m_line.size() << "]" << ": " << m_line << "\n";
}

bool Scanner::getNewLine()
{
    if(!isFileValid())
    {
        return false;
    }

    m_linePos = 0;
    m_lineNumber++;

    while(std::getline(m_file, m_line) && m_line.size() == 0)
    {
        m_lineNumber++;
    }

    if(m_file.eof() == true)
    {
        return false;
    }

    removeLeadingSpaces();
    removeTraillingSpaces();

    dumpCurrentLine();
    
    return true;
}

void Scanner::removeLeadingSpaces()
{
    auto it = std::find_if(m_line.begin(), m_line.end(),
                    [](char c) {
                        return !std::isspace<char>(c, std::locale::classic());
                    });
    m_line.erase(m_line.begin(), it);
}

void Scanner::removeTraillingSpaces()
{
    auto it = std::find_if(m_line.rbegin(), m_line.rend(),
                    [](char c) {
                        return !std::isspace<char>(c, std::locale::classic());
                    });
    m_line.erase(it.base(), m_line.end());
}

bool Scanner::skipSpaces()
{
    // Skip spaces and tabs
    for(; m_linePos < m_line.size(); m_linePos++)
    {
        if(m_line.data()[m_linePos] != ' ' && m_line.data()[m_linePos] != '\t')
        {
            return true;
        }
    }

    // If we got here we consumed the whole line, so we need to fetch a new one
    if(getNewLine() == false)
    {
        return false;
    }

    // Recursively call skipSpaces
    return skipSpaces();
}

uint32_t Scanner::getCompareSize()
{
    uint32_t tokenSize = 0;
    for(; (m_linePos + tokenSize) < m_line.size(); tokenSize++)
    {
        if(m_line.data()[m_linePos + tokenSize] == ' ' || m_line.data()[m_linePos + tokenSize] == '\t')
        {
            break;
        }

        if(   m_line.data()[m_linePos + tokenSize] == '}' || m_line.data()[m_linePos + tokenSize] == '{'
            || m_line.data()[m_linePos + tokenSize] == '$')
        {
            if(tokenSize == 0)
            {
                tokenSize++;
            }
            break;
        }
    }

    return tokenSize;
}

bool Scanner::convert2Hex(const char* str, uint32_t &hex)
{
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
    {
        char *p = nullptr;
        hex = std::strtoul(str, &p, 16);

        if (*p != '\0')
        {
            return false;
        }

        return true;
    }

    return false;
}

bool Scanner::convert2Dec(const char* str, uint32_t &dec)
{
    char *p = nullptr;
    dec = std::strtoul(str, &p, 10);

    if (*p != '\0')
    {
        return false;
    }

    return true;
}

const Token& Scanner::matchToken()
{
    if(skipSpaces() == false)
    {
        return m_lex[Token::TokensImplmented - 1];
    }

    uint32_t tokenSize = getCompareSize();

    for(uint32_t i = 0; i < Token::TokensImplmented; i++)
    {
        if((m_lex[i].getString().size() == tokenSize) && (std::memcmp(m_lex[i].getString().c_str(), &m_line.c_str()[m_linePos], m_lex[i].getString().size()) == 0))
        {
            m_linePos += tokenSize;
            m_current = i;
            return m_lex[i];
        }
    }

    // It may be a value or a string
    uint32_t value = 0;

    // Extract string to be converted
    std::unique_ptr<char[]> temp = std::make_unique<char[]>(tokenSize + 1);
    std::memcpy(temp.get(), &m_line.c_str()[m_linePos], tokenSize);
    temp.get()[tokenSize] = '\0';

    // We can already skip this token
    m_linePos += tokenSize;

    if(convert2Hex(temp.get(), value) == true)
    {
        m_tokenHex.setValue(value);
        m_current = TokenHexIndex;
        return m_tokenHex;
    }

    if(convert2Dec(temp.get(), value) == true)
    {
        m_tokenDec.setValue(value);
        m_current = TokenDecIndex;
        return m_tokenDec;
    }

    m_tokenStr.setString(temp.get());
    m_current = TokenStrIndex;
    return m_tokenStr;
}
