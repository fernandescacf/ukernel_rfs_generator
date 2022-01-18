#pragma once

#include <cstdint>
#include <string>

class Token
{
public:
    typedef enum
    {
        Section, Entry, String, Value, Delimiter, Symbol, Invalid
    }Ttype_t;

    typedef enum
    {
        Header, Script, Ram, Devices, Interrupts, Files,
        Type, Version, Arch, Mach, Exec, Addr, Size, Name, Private, Shared, Source,
        PrivIO, PrivNone, 
        OpenBrace, CloseBrace, EndOfFile, None,
        Hex, Dec, Str,
    }Tsubtype_t;

	Token(const Ttype_t type, const Tsubtype_t subtype, const std::string str = "", const uint32_t value = 0) :
        m_type(type), m_subtype(subtype), m_str(str), m_value(value) { }

	const std::string& getString() const { return m_str; }
	Ttype_t getType() const { return m_type; }
    Tsubtype_t getSubtype() const { return m_subtype; }
	uint32_t getValue() const { return m_value; }

    void setValue(uint32_t value) { m_value = value; }
    void setString(const char *str) { m_str = str; }

    static const uint32_t TokensImplmented { static_cast<uint32_t>(Tsubtype_t::None) + 2 };

private:
    const Ttype_t     m_type;
    const Tsubtype_t  m_subtype;
	std::string       m_str;
	uint32_t          m_value;
};