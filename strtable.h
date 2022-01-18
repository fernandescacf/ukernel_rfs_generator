#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

class StrTable
{
public:
    uint32_t addString(const std::string& str)
    {
        uint32_t offset = 0;

        for(auto const temp : m_strings)
        {
            if(*temp == str)
            {
                return offset;
            }
            offset += temp->size() + 1;
        }

        m_strings.push_back(std::make_shared<const std::string>(str));
        m_size += str.size() + 1;

        return offset;
    }

    const std::shared_ptr<const std::string> getString(uint32_t i) const
    {
        if(i < m_strings.size())
        {
            return m_strings.at(i);
        }

        return nullptr;
    }

    uint32_t getSize() const { return m_size; }

    const std::vector<std::shared_ptr<const std::string>> getStrings() const { return m_strings; }

private:
    std::vector<std::shared_ptr<const std::string>> m_strings;
    uint32_t m_size {0};
};