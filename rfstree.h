#pragma once

#include <cstdint>
#include <string>

class File
{
public:
    enum { Elf, Lib, Text };

private:
    uint32_t m_type { 0 };
    uint32_t m_index { 0 };
    uint32_t m_size { 0 };
    uint32_t m_alignSize { 0 };
    uint32_t m_sh_name { 0 };
    std::string m_name;
    std::string m_source;
};

class Cmd
{
public:
    enum { Exec = 1, Load = 2 };

private:
    uint32_t type { 0 };
    uint32_t sh_name { 0 };
    uint32_t sh_cmd { 0 };
    uint16_t prio { 0 };
    uint16_t priv { 0 };
    std::string name;
    std::string cmd;
//    File* file { nullptr };
};

class RfsTree
{
public:

private:

};