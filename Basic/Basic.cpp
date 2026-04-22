/*
 * File: Basic.cpp
 * ---------------
 * This file is the starter project for the BASIC interpreter.
 */

#include <cctype>
#include <iostream>
#include <string>
#include <sstream>
#include "exp.hpp"
#include "parser.hpp"
#include "program.hpp"
#include "Utils/error.hpp"
#include "Utils/tokenScanner.hpp"
#include "Utils/strlib.hpp"


/* Function prototypes */

void processLine(std::string line, Program &program, EvalState &state);
bool isNumber(const std::string& str);
bool isValidVariableName(const std::string& str);
bool isKeyword(const std::string& str);
void executeImmediateCommand(TokenScanner& scanner, Program& program, EvalState& state);
void handleNumberedLine(int lineNumber, TokenScanner& scanner, Program& program, EvalState& state);

/* Main program */

int main() {
    EvalState state;
    Program program;
    //cout << "Stub implementation of BASIC" << endl;
    while (true) {
        try {
            std::string input;
            getline(std::cin, input);
            if (input.empty())
                continue;
            processLine(input, program, state);
        } catch (ErrorException &ex) {
            std::string msg = ex.getMessage();
            if (msg == "QUIT") {
                return 0;
            }
            if (!msg.empty()) {
                std::cout << msg << std::endl;
            }
        }
    }
    return 0;
}

/*
 * Function: processLine
 * Usage: processLine(line, program, state);
 * -----------------------------------------
 * Processes a single line entered by the user.  In this version of
 * implementation, the program reads a line, parses it as an expression,
 * and then prints the result.  In your implementation, you will
 * need to replace this method with one that can respond correctly
 * when the user enters a program line (which begins with a number)
 * or one of the BASIC commands, such as LIST or RUN.
 */

void processLine(std::string line, Program &program, EvalState &state) {
    TokenScanner scanner;
    scanner.ignoreWhitespace();
    scanner.scanNumbers();
    scanner.setInput(line);

    if (!scanner.hasMoreTokens()) {
        return;
    }

    std::string firstToken = scanner.nextToken();

    // Check if line starts with a number (program line)
    if (isNumber(firstToken)) {
        int lineNumber = stringToInteger(firstToken);

        // Check if there are more tokens
        if (!scanner.hasMoreTokens()) {
            // Empty line - remove the line from program
            program.removeSourceLine(lineNumber);
        } else {
            // Line with statement - add to program
            handleNumberedLine(lineNumber, scanner, program, state);
        }
    } else {
        // Immediate command
        scanner.saveToken(firstToken);
        executeImmediateCommand(scanner, program, state);
    }
}

bool isNumber(const std::string& str) {
    if (str.empty()) return false;
    for (size_t i = 0; i < str.size(); i++) {
        if (i == 0 && str[i] == '-') continue; // Allow negative
        if (!isdigit(str[i])) return false;
    }
    return true;
}

bool isValidVariableName(const std::string& str) {
    if (str.empty()) return false;

    // Check if it's a keyword
    if (isKeyword(str)) return false;

    // Check if all characters are alphanumeric
    for (char c : str) {
        if (!isalnum(c)) return false;
    }

    return true;
}

bool isKeyword(const std::string& str) {
    static const std::set<std::string> keywords = {
        "REM", "LET", "PRINT", "INPUT", "END", "GOTO", "IF", "THEN",
        "RUN", "LIST", "CLEAR", "QUIT", "HELP"
    };
    return keywords.find(str) != keywords.end();
}

void executeImmediateCommand(TokenScanner& scanner, Program& program, EvalState& state) {
    std::string command = scanner.nextToken();

    if (command == "RUN") {
        try {
            program.runProgram(state);
        } catch (ErrorException& e) {
            // End statement throws empty error to signal end
            std::string msg = e.getMessage();
            if (!msg.empty()) {
                throw e;
            }
        }
    } else if (command == "LIST") {
        program.listProgram(std::cout);
    } else if (command == "CLEAR") {
        program.clear();
        state.Clear();
    } else if (command == "QUIT") {
        throw ErrorException("QUIT");
    } else if (command == "HELP") {
        // Optional help command
        std::cout << "BASIC Interpreter Commands:" << std::endl;
        std::cout << "  RUN - Execute the program" << std::endl;
        std::cout << "  LIST - List program lines" << std::endl;
        std::cout << "  CLEAR - Clear program and variables" << std::endl;
        std::cout << "  QUIT - Exit interpreter" << std::endl;
        std::cout << "  HELP - Show this help" << std::endl;
    } else if (command == "LET") {
        // Immediate LET command
        if (!scanner.hasMoreTokens()) {
            throw ErrorException("SYNTAX ERROR");
        }

        std::string var = scanner.nextToken();
        if (!isValidVariableName(var)) {
            throw ErrorException("SYNTAX ERROR");
        }

        if (!scanner.hasMoreTokens() || scanner.nextToken() != "=") {
            throw ErrorException("SYNTAX ERROR");
        }

        std::string rest;
        while (scanner.hasMoreTokens()) {
            rest += scanner.nextToken();
            if (scanner.hasMoreTokens()) rest += " ";
        }

        if (rest.empty()) {
            throw ErrorException("SYNTAX ERROR");
        }

        try {
            TokenScanner expScanner;
            expScanner.ignoreWhitespace();
            expScanner.scanNumbers();
            expScanner.setInput(rest);
            Expression* exp = parseExp(expScanner);
            try {
                int value = exp->eval(state);
                state.setValue(var, value);
                delete exp;
            } catch (...) {
                delete exp;
                throw;
            }
        } catch (ErrorException& e) {
            throw ErrorException("SYNTAX ERROR");
        }
    } else if (command == "PRINT") {
        // Immediate PRINT command
        std::string rest;
        while (scanner.hasMoreTokens()) {
            rest += scanner.nextToken();
            if (scanner.hasMoreTokens()) rest += " ";
        }

        if (rest.empty()) {
            throw ErrorException("SYNTAX ERROR");
        }

        try {
            TokenScanner expScanner;
            expScanner.ignoreWhitespace();
            expScanner.scanNumbers();
            expScanner.setInput(rest);
            Expression* exp = parseExp(expScanner);
            try {
                int value = exp->eval(state);
                std::cout << value << std::endl;
                delete exp;
            } catch (...) {
                delete exp;
                throw;
            }
        } catch (ErrorException& e) {
            throw ErrorException("SYNTAX ERROR");
        }
    } else if (command == "INPUT") {
        // Immediate INPUT command
        if (!scanner.hasMoreTokens()) {
            throw ErrorException("SYNTAX ERROR");
        }

        std::string var = scanner.nextToken();
        if (!isValidVariableName(var)) {
            throw ErrorException("SYNTAX ERROR");
        }

        if (scanner.hasMoreTokens()) {
            throw ErrorException("SYNTAX ERROR");
        }

        InputStatement stmt(var);
        stmt.execute(state, program);
    } else {
        // Not a valid immediate command
        throw ErrorException("SYNTAX ERROR");
    }
}

void handleNumberedLine(int lineNumber, TokenScanner& scanner, Program& program, EvalState& state) {
    std::string command = scanner.nextToken();

    // Store the original line
    std::stringstream originalLine;
    originalLine << lineNumber << " " << command;
    while (scanner.hasMoreTokens()) {
        originalLine << " " << scanner.nextToken();
    }
    program.addSourceLine(lineNumber, originalLine.str());

    // Reset scanner for parsing
    scanner.setInput(originalLine.str());
    scanner.nextToken(); // Skip line number

    command = scanner.nextToken();
    Statement* stmt = nullptr;

    try {
        if (command == "REM") {
            stmt = new RemStatement();
        } else if (command == "LET") {
            if (!scanner.hasMoreTokens()) {
                throw ErrorException("SYNTAX ERROR");
            }

            std::string var = scanner.nextToken();
            if (!isValidVariableName(var)) {
                throw ErrorException("SYNTAX ERROR");
            }

            if (!scanner.hasMoreTokens() || scanner.nextToken() != "=") {
                throw ErrorException("SYNTAX ERROR");
            }

            std::string rest;
            while (scanner.hasMoreTokens()) {
                rest += scanner.nextToken();
                if (scanner.hasMoreTokens()) rest += " ";
            }

            if (rest.empty()) {
                throw ErrorException("SYNTAX ERROR");
            }

            TokenScanner expScanner;
            expScanner.ignoreWhitespace();
            expScanner.scanNumbers();
            expScanner.setInput(rest);
            Expression* exp = nullptr;
            try {
                exp = parseExp(expScanner);
                stmt = new LetStatement(var, exp);
                // Statement now owns the expression
            } catch (ErrorException& e) {
                delete exp;
                throw;
            }
        } else if (command == "PRINT") {
            std::string rest;
            while (scanner.hasMoreTokens()) {
                rest += scanner.nextToken();
                if (scanner.hasMoreTokens()) rest += " ";
            }

            if (rest.empty()) {
                throw ErrorException("SYNTAX ERROR");
            }

            TokenScanner expScanner;
            expScanner.ignoreWhitespace();
            expScanner.scanNumbers();
            expScanner.setInput(rest);
            Expression* exp = nullptr;
            try {
                exp = parseExp(expScanner);
                stmt = new PrintStatement(exp);
                // Statement now owns the expression
            } catch (ErrorException& e) {
                delete exp;
                throw;
            }
        } else if (command == "INPUT") {
            if (!scanner.hasMoreTokens()) {
                throw ErrorException("SYNTAX ERROR");
            }

            std::string var = scanner.nextToken();
            if (!isValidVariableName(var)) {
                throw ErrorException("SYNTAX ERROR");
            }

            if (scanner.hasMoreTokens()) {
                throw ErrorException("SYNTAX ERROR");
            }

            stmt = new InputStatement(var);
        } else if (command == "END") {
            if (scanner.hasMoreTokens()) {
                throw ErrorException("SYNTAX ERROR");
            }
            stmt = new EndStatement();
        } else if (command == "GOTO") {
            if (!scanner.hasMoreTokens()) {
                throw ErrorException("SYNTAX ERROR");
            }

            std::string lineStr = scanner.nextToken();
            if (!isNumber(lineStr)) {
                throw ErrorException("SYNTAX ERROR");
            }

            if (scanner.hasMoreTokens()) {
                throw ErrorException("SYNTAX ERROR");
            }

            int targetLine = stringToInteger(lineStr);
            stmt = new GotoStatement(targetLine);
        } else if (command == "IF") {
            // Parse IF statement: IF exp1 op exp2 THEN line
            // First, parse exp1 by collecting tokens until we find an operator
            std::string exp1Str;
            std::string token;
            bool foundOperator = false;
            std::string op;

            while (scanner.hasMoreTokens()) {
                token = scanner.nextToken();
                if (token == "=" || token == "<" || token == ">") {
                    op = token;
                    foundOperator = true;
                    break;
                }
                exp1Str += token;
                if (scanner.hasMoreTokens()) {
                    // Peek at next token
                    std::string nextToken = scanner.nextToken();
                    scanner.saveToken(nextToken);
                    // Add space unless next token is an operator
                    if (nextToken != "=" && nextToken != "<" && nextToken != ">") {
                        exp1Str += " ";
                    }
                }
            }

            if (!foundOperator) {
                throw ErrorException("SYNTAX ERROR");
            }

            // Parse exp2 by collecting tokens until we find "THEN"
            std::string exp2Str;
            bool foundThen = false;

            while (scanner.hasMoreTokens()) {
                token = scanner.nextToken();
                if (token == "THEN") {
                    foundThen = true;
                    break;
                }
                exp2Str += token;
                if (scanner.hasMoreTokens()) {
                    // Peek at next token
                    std::string nextToken = scanner.nextToken();
                    scanner.saveToken(nextToken);
                    // Add space unless next token is "THEN"
                    if (nextToken != "THEN") {
                        exp2Str += " ";
                    }
                }
            }

            if (!foundThen) {
                throw ErrorException("SYNTAX ERROR");
            }

            // Parse line number
            if (!scanner.hasMoreTokens()) {
                throw ErrorException("SYNTAX ERROR");
            }

            std::string lineStr = scanner.nextToken();
            if (!isNumber(lineStr)) {
                throw ErrorException("SYNTAX ERROR");
            }

            if (scanner.hasMoreTokens()) {
                throw ErrorException("SYNTAX ERROR");
            }

            int targetLine = stringToInteger(lineStr);

            // Parse expressions
            TokenScanner exp1Scanner;
            exp1Scanner.ignoreWhitespace();
            exp1Scanner.scanNumbers();
            exp1Scanner.setInput(exp1Str);
            Expression* exp1 = nullptr;
            Expression* exp2 = nullptr;

            try {
                exp1 = parseExp(exp1Scanner);

                TokenScanner exp2Scanner;
                exp2Scanner.ignoreWhitespace();
                exp2Scanner.scanNumbers();
                exp2Scanner.setInput(exp2Str);
                exp2 = parseExp(exp2Scanner);

                stmt = new IfStatement(exp1, exp2, op, targetLine);
                // Statement now owns the expressions
                exp1 = nullptr;
                exp2 = nullptr;
            } catch (ErrorException& e) {
                delete exp1;
                delete exp2;
                throw;
            }
        } else {
            throw ErrorException("SYNTAX ERROR");
        }

        program.setParsedStatement(lineNumber, stmt);
    } catch (ErrorException& e) {
        // Clean up and rethrow
        delete stmt;
        throw e;
    }
}