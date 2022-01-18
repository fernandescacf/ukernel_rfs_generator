#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

#include "symbol.h"

class SymTable
{
public:
    bool addSymbol(std::shared_ptr<Symbol> symbol)
    {
        if(checkForSymbol(symbol->getLabel()))
        {
            return false;
        }

        m_symbols.push_back(symbol);

        return true;
    }

    std::shared_ptr<Symbol> getSymbol(const std::string& label) const
    {
        for(auto const symbol : m_symbols)
        {
            if(symbol->isSymbol(label) == true)
            {
                return symbol;
            }
        }

        return nullptr;
    }

private:
    std::vector<std::shared_ptr<Symbol>> m_symbols;

    bool checkForSymbol(const std::string& label)
    {
        for(auto const symbol : m_symbols)
        {
            if(symbol->isSymbol(label) == true)
            {
                return true;
            }
        }
        return false;
    }
};