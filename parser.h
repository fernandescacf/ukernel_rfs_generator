#pragma once

#include <cstdint>
#include <string>

#include "token.h"
#include "scanner.h"
#include "symtable.h"
#include "strtable.h"

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem::v1;

#define ALIGN_UP(m,a)	(((m) + (a - 1)) & (~(a - 1)))

#define EXECUTE_ONCE()              \
    do                              \
    {                               \
        static bool _done = false;  \
        if(_done) return false;     \
        _done = true;               \
    }while(0)

struct Header
{
    const uint32_t RfsType = 0xCACFCACF;
    uint32_t m_shVersion { static_cast<uint32_t>(-1) };
    uint32_t m_shArch { static_cast<uint32_t>(-1) };
    uint32_t m_shMach { static_cast<uint32_t>(-1) };
};

struct Ram
{
    uint32_t m_address { static_cast<uint32_t>(-1) };
    uint32_t m_size { static_cast<uint32_t>(-1) };
};

struct Interrupts
{
    uint32_t m_priv { static_cast<uint32_t>(-1) };
    uint32_t m_shared { static_cast<uint32_t>(-1) };
};

class Device
{
public:
    std::string& getName() { return m_name; }
    uint32_t getNameOffset() const { return m_shName; }
    uint32_t getAddress() const { return m_addr; }
    uint32_t getSize() const { return m_size; }

    void setNameOffset(uint32_t offset) { m_shName = offset; }
    void setAddress(uint32_t address) { m_addr = address; }
    void setSize(uint32_t size) { m_size = size; }

private:
    std::string m_name;
    uint32_t m_shName { static_cast<uint32_t>(-1) };
    uint32_t m_addr { static_cast<uint32_t>(-1) };
    uint32_t m_size { static_cast<uint32_t>(-1) };
};

class File
{
public:
    std::string& getName() { return m_name; }
    std::string& getSource() { return m_source; }
    uint32_t getIndex() const { return m_index; }
    uint32_t getType() const { return m_type; }
    uint32_t getSize() const { return m_size; }
    uint32_t getAlignSize() const { return m_alignSize; }
    uint32_t getNameOffset() const { return m_shName; }

    void setType(uint32_t type) { m_type = type; }
    void setIndex(uint32_t index) { m_index = index; }
    void setNameOffset(uint32_t offset) { m_shName = offset; }

    bool configFile()
    {
        m_size = fs::file_size(m_source);
        m_alignSize = ALIGN_UP(m_size, 0x4);

        return true;
    }

private:
    uint32_t m_index { static_cast<uint32_t>(-1) };
    uint32_t m_type { static_cast<uint32_t>(-1) };
    uint32_t m_size { static_cast<uint32_t>(-1) };
    uint32_t m_alignSize { static_cast<uint32_t>(-1) };
    std::string m_name;
    uint32_t m_shName { static_cast<uint32_t>(-1) };
    std::string m_source;
};

class Cmd
{
public:
    enum { Exec = 1, Load = 2 };

    Cmd(uint32_t type) : m_type(type) {}

    std::string& getName() { return m_name; }
    std::string& getArgs() { return m_args; }

    uint32_t getType() const { return m_type; }
    uint32_t getPriority() const { return m_prio; }
    uint32_t getPrivilege() const { return m_priv; }
    uint32_t getNameOffset() const { return m_shName; }
    uint32_t getArgsOffset() const { return m_shArgs; }
    std::shared_ptr<File> getFile() const { return m_file; }

    void setPriority(uint16_t prio) { m_prio = prio; }
    void setPrivilege(uint16_t priv) { m_priv = priv; }
    void setNameOffset(uint32_t offset) { m_shName = offset; }
    void setArgsOffset(uint32_t offset) { m_shArgs = offset; }
    void setFile(std::shared_ptr<File> file) { m_file = file; }

private:
    uint32_t m_type { 0 };
    uint32_t m_shName { 0 };
    uint32_t m_shArgs { 0 };
    uint16_t m_prio { 0 };
    uint16_t m_priv { 0 };
    std::string m_name;
    std::string m_args;
    std::shared_ptr<File> m_file { nullptr };
};


class RfsTree
{
public:
    bool setVersion(std::string& version)
    {
        if(m_header.m_shVersion != static_cast<uint32_t>(-1))
        {
            return false;
        }
        m_header.m_shVersion = registerString(version);

        return true;
    }

    bool setArch(std::string& arch)
    {
        if(m_header.m_shArch != static_cast<uint32_t>(-1))
        {
            return false;
        }
        m_header.m_shArch = registerString(arch);

        return true;
    }

    bool setMach(std::string& mach)
    {
        if(m_header.m_shMach != static_cast<uint32_t>(-1))
        {
            return false;
        }
        m_header.m_shMach = registerString(mach);

        return true;
    }

    bool setRamAddress(uint32_t addr)
    {
        if(m_ram.m_address != static_cast<uint32_t>(-1))
        {
            return false;
        }
        m_ram.m_address = addr;

        return true;
    }

    bool setRamSize(uint32_t size)
    {
        if(m_ram.m_size != static_cast<uint32_t>(-1))
        {
            return false;
        }
        m_ram.m_size = size;

        return true;
    }

    bool setIntrPriv(uint32_t priv)
    {
        if(m_intr.m_priv != static_cast<uint32_t>(-1))
        {
            return false;
        }
        m_intr.m_priv = priv;

        return true;
    }

    bool setIntrShared(uint32_t shared)
    {
        if(m_intr.m_shared != static_cast<uint32_t>(-1))
        {
            return false;
        }
        m_intr.m_shared = shared;

        return true;
    }

    uint32_t registerString(std::string& str)
    {
        return m_strTable.addString(str);
    }

    void registerCmd(std::shared_ptr<Cmd> cmd)
    {
        m_ScriptCmds.push_back(cmd);
    }

    void registerDevice(std::shared_ptr<Device> dev)
    {
        m_Devices.push_back(dev);
    }

    void registerFile(std::shared_ptr<File> file)
    {
        file->setIndex(m_Files.size());
        m_Files.push_back(file);
    }

    bool resolveDependencies()
    {
        for (uint32_t i = 0; i < m_ScriptCmds.size(); i++)
		{
			for (uint32_t j = 0; j < m_Files.size(); j++)
			{
				if (m_ScriptCmds.at(i)->getName() == m_Files.at(j)->getName())
				{
					m_ScriptCmds.at(i)->setFile(m_Files.at(j));
					break;
				}

                if((j + 1) == m_Files.size())
                {
                    return false;
                }
			}
		}

		return true;
    }

    const StrTable& getStrTable() const { return m_strTable; }
    const Header& getHeader() const { return m_header; }
    const Ram& getRam() const { return m_ram; }
    const Interrupts& getInterrupts() const { return m_intr; }
    const std::vector<std::shared_ptr<Cmd>>& getCmds() const { return m_ScriptCmds; }
    const std::vector<std::shared_ptr<Device>>& getDevices() const { return m_Devices; }
    const std::vector<std::shared_ptr<File>>& getFiles() const { return m_Files; }

private:
    StrTable m_strTable;
    Header m_header;
    Ram m_ram;
    Interrupts m_intr;
    std::vector<std::shared_ptr<Cmd>> m_ScriptCmds;
    std::vector<std::shared_ptr<Device>> m_Devices;
    std::vector<std::shared_ptr<File>> m_Files;
};

class Parser
{
public:
    Parser(Scanner& scanner) : m_scanner(scanner) {}

    bool start()
    {
        enum
        {
            Running, End, Fail
        } state = Running;

        while(state == Running)
        {
            const Token& token = m_scanner.getToken();

            if(token.getType() == Token::Invalid)
            {
                state = ((token.getSubtype() == Token::EndOfFile) ? (End): (Fail));
                
                continue;
            }

            if(token.getType() == Token::String)
            {
                if(parseSymbol(token) == false)
                {
                    state = Fail;
                }
                continue;
            }

            if(token.getType() == Token::Section)
            {
                if(parseSection(token) == false)
                {
                    state = Fail;
                }
                continue;
            }
            
            state = Fail;
        }

        if(state == End)
        {
            return m_rfsTree.resolveDependencies();
        }
        else
        {
            std::cout << "Error at line " << m_scanner.getLineNumber() << "\n";
        }
        

        return false;
    }

    const RfsTree& getRfsTree() { return m_rfsTree; }

private:
    uint32_t m_scope {0};
    Scanner& m_scanner;
    SymTable m_symtable;
    RfsTree m_rfsTree;

    bool parseSymbol(const Token label)
    {
        if(m_scope > 0)
        {
            return false;
        }

        const Token& literal = m_scanner.getToken();

        if(literal.getSubtype() != Token::Str)
        {
            return false;
        }

        if(!m_symtable.addSymbol(std::make_shared<Symbol>(label.getString(), literal.getString())))
        {
            return false;
        }

        return true;
    }

    bool parseSection(const Token& section)
    {
        switch (section.getSubtype())
        {
        case Token::Header:
            return parseHeader();
        case Token::Script:
            return parseScript();
        case Token::Ram:
            return parseRam();
        case Token::Devices:
            return parseDevices();
        case Token::Interrupts:
            return parseInterrupts();
        case Token::Files:
            return parseFiles();
        default:
            return false;
        }
    }

    bool parseHeader()
    {
        EXECUTE_ONCE();

        if(m_scanner.getToken().getSubtype() != Token::OpenBrace)
        {
            return false;
        }
        m_scope++;

        bool okay = true;

        while(okay)
        {
            const Token& token = m_scanner.getToken();

            if(token.getSubtype() == Token::CloseBrace)
            {
                break;
            }

            switch (token.getSubtype())
            {
            case Token::Type:
                okay = parseType();
                break;
            case Token::Version:
                okay = parseVersion();
                break;
            case Token::Arch:
                okay = parseArch();
                break;
            case Token::Mach:
                okay = parseMach();
                break;
            default:
                okay = false;
            }
        }

        m_scope--;

        return okay;
    }

    bool parseScript()
    {
        EXECUTE_ONCE();

        if(m_scanner.getToken().getSubtype() != Token::OpenBrace)
        {
            return false;
        }
        m_scope++;

        bool okay = true;

        while(okay)
        {
            const Token& token = m_scanner.getToken();

            if(token.getSubtype() == Token::CloseBrace)
            {
                break;
            }

            switch (token.getSubtype())
			{
			case Token::Exec:
				okay = parseExecCmd();
				break;
			default:
				okay = false;
			}
        }

        m_scope--;

        return okay;
    }

    bool parseRam()
    {
        EXECUTE_ONCE();

        if(m_scanner.getToken().getSubtype() != Token::OpenBrace)
        {
            return false;
        }
        m_scope++;

        bool okay = true;

        while(okay)
        {
            const Token& token = m_scanner.getToken();

            if(token.getSubtype() == Token::CloseBrace)
            {
                break;
            }

            if(token.getSubtype() == Token::Addr)
            {
                const Token& addrToken = m_scanner.getToken();

                if(addrToken.getType() == Token::Value)
                {
                    okay = m_rfsTree.setRamAddress(addrToken.getValue());
                    continue;
                }
                okay = false;
                continue;
            }

            if(token.getSubtype() == Token::Size)
            {
                const Token& sizeToken = m_scanner.getToken();

                if(sizeToken.getType() == Token::Value)
                {
                    okay = m_rfsTree.setRamSize(sizeToken.getValue());
                    continue;
                }
                okay = false;
                continue;
            }

            okay = false;
        }

        m_scope--;

        return okay;
    }

    bool parseInterrupts()
    {
        EXECUTE_ONCE();

        if(m_scanner.getToken().getSubtype() != Token::OpenBrace)
        {
            return false;
        }
        m_scope++;

        bool okay = true;

        while(okay)
        {
            const Token& token = m_scanner.getToken();

            if(token.getSubtype() == Token::CloseBrace)
            {
                break;
            }

            if(token.getSubtype() == Token::Shared)
            {
                const Token& sharedToken = m_scanner.getToken();

                if(sharedToken.getType() == Token::Value)
                {
                    okay = m_rfsTree.setIntrShared(sharedToken.getValue());
                    continue;
                }
                okay = false;
                continue;
            }

            if(token.getSubtype() == Token::Private)
            {
                const Token& privToken = m_scanner.getToken();

                if(privToken.getType() == Token::Value)
                {
                    okay = m_rfsTree.setIntrPriv(privToken.getValue());
                    continue;
                }
                okay = false;
                continue;
            }

            okay = false;
        }

        m_scope--;

        return okay;
    }

    bool parseDevices()
    {
        EXECUTE_ONCE();

        m_scope++;

        if(m_scanner.getToken().getSubtype() != Token::OpenBrace)
        {
            return false;
        }

        bool okay = true;
        while(okay)
        {
            if(parseDev() == false)
            {
                if(m_scanner.currentToken().getSubtype() != Token::CloseBrace)
                {
                    okay = false;
                }
                break;
            }
        }

        m_scope--;

        return okay;
    }

    bool parseFiles()
    {
        EXECUTE_ONCE();

        m_scope++;

        if(m_scanner.getToken().getSubtype() != Token::OpenBrace)
        {
            return false;
        }

        bool okay = true;
        while(okay)
        {
            if(parseFile() == false)
            {
                if(m_scanner.currentToken().getSubtype() != Token::CloseBrace)
                {
                    okay = false;
                }
                break;
            }
        }

        m_scope--;

        return okay;
    }

    bool parseType()
    {
        EXECUTE_ONCE();

        std::string type;
        if(parseName(type) == false)
        {
            return false;
        }

        // Not supported will be removed

        return true;
    }

    bool parseVersion()
    {
        EXECUTE_ONCE();

        std::string version;
        if(parseName(version) == false)
        {
            return false;
        }

        return m_rfsTree.setVersion(version);
    }

    bool parseArch()
    {
        EXECUTE_ONCE();

        std::string arch;
        if(parseName(arch) == false)
        {
            return false;
        }

        return m_rfsTree.setArch(arch);
    }

    bool parseMach()
    {
        EXECUTE_ONCE();

        std::string mach;
        if(parseName(mach) == false)
        {
            return false;
        }

        return m_rfsTree.setMach(mach);
    }

    bool parseExecCmd()
    {
        std::shared_ptr<Cmd> cmdPtr = std::make_shared<Cmd>(Cmd::Exec);

        if(parseName(cmdPtr->getName()) == false)
        {
            return false;
        }

        cmdPtr->getArgs() = cmdPtr->getName();

        while(true)
        {
            const Token& token = m_scanner.getToken();

            if(token.getSubtype() == Token::OpenBrace)
            {
                break;
            }

            std::string temp;
            if(parseName(token, temp) == false)
            {
                return false;
            }

            if(cmdPtr->getArgs().size() > 0) cmdPtr->getArgs() += " ";
            cmdPtr->getArgs() += temp;
        }

        const Token& PrioToken = m_scanner.getToken();
        if(PrioToken.getType() != Token::Value)
        {
            return false;
        }
        cmdPtr->setPriority(static_cast<uint16_t>(PrioToken.getValue()));

        const Token& PrivToken = m_scanner.getToken();
        if(PrivToken.getSubtype() != Token::PrivIO)
        {
            return false;
        }
        cmdPtr->setPrivilege(static_cast<uint16_t>(PrioToken.getValue()));

        if( m_scanner.getToken().getSubtype() != Token::CloseBrace)
        {
            return false;
        }

        cmdPtr->setNameOffset(m_rfsTree.registerString(cmdPtr->getName()));
        cmdPtr->setArgsOffset(m_rfsTree.registerString(cmdPtr->getArgs()));
        
        m_rfsTree.registerCmd(cmdPtr);

        return true;
    }

    bool parseDev()
    {
        if(m_scanner.getToken().getSubtype() != Token::OpenBrace)
        {
            return false;
        }

        std::shared_ptr<Device> dev = std::make_shared<Device>();

        bool okay = true;
        while(okay)
        {
            const Token& token = m_scanner.getToken();

            if(token.getSubtype() == Token::Name)
            {
                if(!dev->getName().empty())
                {
                    okay = false;
                    continue;
                }
                okay = parseName(dev->getName());
                dev->setNameOffset(m_rfsTree.registerString(dev->getName()));
                continue;
            }

            if(token.getSubtype() == Token::Addr)
            {
                if(dev->getAddress() != static_cast<uint32_t>(-1))
                {
                    okay = false;
                    continue;
                }
                const Token& addrToken = m_scanner.getToken();

                if(addrToken.getType() == Token::Value)
                {
                    dev->setAddress(addrToken.getValue());
                    continue;
                }
                okay = false;
                continue;
            }

            if(token.getSubtype() == Token::Size)
            {
                if(dev->getSize() != static_cast<uint32_t>(-1))
                {
                    okay = false;
                    continue;
                }
                const Token& addrToken = m_scanner.getToken();

                if(addrToken.getType() == Token::Value)
                {
                    dev->setSize(addrToken.getValue());
                    continue;
                }
                okay = false;
                continue;
            }

            if(token.getSubtype() == Token::CloseBrace)
            {
                break;
            }

            okay = false;
        }

        if(okay == true)
        {
            m_rfsTree.registerDevice(dev);
        }

        return okay;
    }

    bool parseFile()
    {
        if(m_scanner.getToken().getSubtype() != Token::OpenBrace)
        {
            return false;
        }

        std::shared_ptr<File> file = std::make_shared<File>();

        bool okay = true;
        while(okay)
        {
            const Token& token = m_scanner.getToken();

            if(token.getSubtype() == Token::Name)
            {
                if(!file->getName().empty())
                {
                    okay = false;
                    continue;
                }
                okay = parseName(file->getName());
                file->setNameOffset(m_rfsTree.registerString(file->getName()));
                continue;
            }

            if(token.getSubtype() == Token::Type)
            {
                if(file->getType() != static_cast<uint32_t>(-1))
                {
                    okay = false;
                    continue;
                }
                const Token& typeToken = m_scanner.getToken();

                if(typeToken.getType() == Token::String)
                {
                    std::string type;
                    if(parseName(typeToken, type))
                    {
                        if(type == "elf")
                        {
                            file->setType(0x1);
                            continue;
                        }
                    }
                }
                okay = false;
                continue;
            }

            if(token.getSubtype() == Token::Source)
            {
                if(!file->getSource().empty())
                {
                    okay = false;
                    continue;
                }
                okay = parseName(file->getSource());

                if(okay)
                {
                    file->configFile();
                }

                continue;
            }

            if(token.getSubtype() == Token::CloseBrace)
            {
                break;
            }

            okay = false;
        }

        if(okay == true)
        {
            m_rfsTree.registerFile(file);
        }

        return okay;
    }

    std::shared_ptr<Symbol> resolveSymbol()
    {
        if(m_scanner.getToken().getSubtype() != Token::OpenBrace)
        {
            return nullptr;
        }

        Token label = m_scanner.getToken();

        if(m_scanner.getToken().getSubtype() != Token::CloseBrace)
        {
            return nullptr;
        }

        return m_symtable.getSymbol(label.getString());
    }

    std::string getName()
    {
        const Token& token = m_scanner.getToken();
        std::string name;

        switch(token.getType())
        {
        case Token::String:
            name = token.getString();
            if(m_scanner.hasSymbol())
            {
                name += getName();
            }
            break;
        case Token::Symbol:
        {
            auto sym = resolveSymbol();
            if(sym == nullptr) break;
            name = sym->getLiteral();
            if(!m_scanner.hasDelimiter())
            {
                name += getName();
            }
            break;
        }
        default:
            break;
        }

        return name;
    }

    std::string getName(const Token& token)
    {
        std::string name;

        switch(token.getType())
        {
        case Token::String:
            name = token.getString();
            if(m_scanner.hasSymbol())
            {
                name += getName();
            }
            break;
        case Token::Symbol:
            name = resolveSymbol()->getLiteral();
            if(!m_scanner.hasDelimiter())
            {
                name += getName();
            }
            break;
        default:
            break;
        }

        return name;
    }

    bool parseName(std::string& name)
    {
       name = getName();
       return (name.empty() == false);
    }

    bool parseName(const Token& token, std::string& name)
    {
        name = getName(token);
        return (name.empty() == false);
    }

    bool resolveDependencies();
};