#pragma once

#include <cstdint>
#include <string>
#include <iostream>


class Symbol
{
public:
    Symbol(const std::string& label, const std::string& literal) : m_label(label), m_literal(literal) {}

    static bool toNum(const std::string& str, uint32_t& value)
    {
        char *p = nullptr;

        if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
        {
            value = std::strtol(str.c_str(), &p, 16);
        }
        else
        {
            value = std::strtol(str.c_str(), &p, 10);
        }

        if (*p != '\0')
        {
            return false;
        }
        
        return true;
    }

    bool isSymbol(const std::string& label) const
    {
        return label == m_label;
    }

    const std::string& getLiteral() const
    {
        return m_literal;
    }

    const std::string& getLabel() const
    {
        return m_label;
    }

private:
    const std::string m_label;
    const std::string m_literal;
};
