#include <cstdint>
#include <iostream>
#include <fstream>
#include <string>
#include <array>

#include "token.h"
#include "scanner.h"
#include "symtable.h"
#include "strtable.h"
#include "parser.h"
#include "generator.h"

uint32_t scope = 0;

bool SymbolRule(const Token label, Scanner& scanner, SymTable& symtable)
{
    if(scope > 0)
    {
        std::cout << "Error line " << scanner.getLineNumber() << ": failed to create symbol \"" << label.getString() << "\"\n";
        std::cout << "Info: Symbols can only be define at global scope!\n";
        return false;
    }

    const Token& literal = scanner.getToken();

    if(literal.getSubtype() != Token::Str)
    {
        std::cout << "Error line " << scanner.getLineNumber() << ": failed to create symbol \"" << label.getString() << "\"\n";
        std::cout << "Info: Label \"Value\"\n";
        return false;
    }

    if(!symtable.addSymbol(std::make_shared<Symbol>(label.getString(), literal.getString())))
    {
        std::cout << "Error line " << scanner.getLineNumber() << ": failed to create symbol \"" << label.getString() << "\"\n";
        std::cout << "Info: Label \"" << label.getString() << "\" already defined!\n";
        return false;
    }

    return true;
}

int main(int argc, char **argv)
{
    SymTable symtable;
    StrTable strtable;

    //Scanner scanner("/home/cacf/neok/rfs_generator/script_ve.txt");
    Scanner scanner(argv [1]);

    if(!scanner.isFileValid())
    {
        std::cout << "File: " << argv [1] << " not valid!" << "\n";
        return -1;
    }

    Parser parser(scanner);

    parser.start();

    //GenerateRfs(parser.getRfsTree(), "/home/cacf/neok/rfs_generator/rfs.bin");
    GenerateRfs(parser.getRfsTree(), argv[2]);

    return 0;
}
