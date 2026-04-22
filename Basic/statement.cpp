/*
 * File: statement.cpp
 * -------------------
 * This file implements the constructor and destructor for
 * the Statement class itself.  Your implementation must do
 * the same for the subclasses you define for each of the
 * BASIC statements.
 */

#include "statement.hpp"
#include <iostream>

/* Implementation of the Statement class */

int stringToInt(std::string str);

Statement::Statement() = default;

Statement::~Statement() = default;

// RemStatement implementation
void RemStatement::execute(EvalState &state, Program &program) {
    // REM statements do nothing
}

// LetStatement implementation
LetStatement::LetStatement(const std::string& varName, Expression* expr) : var(varName), exp(expr) {}

LetStatement::~LetStatement() {
    delete exp;
}

void LetStatement::execute(EvalState &state, Program &program) {
    int value = exp->eval(state);
    state.setValue(var, value);
}

// PrintStatement implementation
PrintStatement::PrintStatement(Expression* expr) : exp(expr) {}

PrintStatement::~PrintStatement() {
    delete exp;
}

void PrintStatement::execute(EvalState &state, Program &program) {
    int value = exp->eval(state);
    std::cout << value << std::endl;
}

// InputStatement implementation
InputStatement::InputStatement(const std::string& varName) : var(varName) {}

void InputStatement::execute(EvalState &state, Program &program) {
    std::string input;
    while (true) {
        std::cout << "? ";
        std::cout.flush();
        std::getline(std::cin, input);
        if (input.empty()) continue;

        // Check if input is a valid integer
        bool valid = true;
        for (size_t i = 0; i < input.size(); i++) {
            if (i == 0 && input[i] == '-') continue; // Allow negative sign
            if (!isdigit(input[i])) {
                valid = false;
                break;
            }
        }

        if (valid) {
            try {
                int value = std::stoi(input);
                state.setValue(var, value);
                break;
            } catch (std::out_of_range&) {
                // Value out of range, ask again
            }
        }
        // If invalid, ask again (no error message per spec)
    }
}

// EndStatement implementation
void EndStatement::execute(EvalState &state, Program &program) {
    // End program execution
    throw ErrorException(""); // Empty string to signal end without error message
}

// GotoStatement implementation
GotoStatement::GotoStatement(int lineNum) : lineNumber(lineNum) {}

void GotoStatement::execute(EvalState &state, Program &program) {
    program.setCurrentLine(lineNumber);
}

// IfStatement implementation
IfStatement::IfStatement(Expression* expr1, Expression* expr2, const std::string& operation, int lineNum)
    : exp1(expr1), exp2(expr2), op(operation), lineNumber(lineNum) {}

IfStatement::~IfStatement() {
    delete exp1;
    delete exp2;
}

void IfStatement::execute(EvalState &state, Program &program) {
    int val1 = exp1->eval(state);
    int val2 = exp2->eval(state);

    bool condition = false;
    if (op == "=") {
        condition = (val1 == val2);
    } else if (op == "<") {
        condition = (val1 < val2);
    } else if (op == ">") {
        condition = (val1 > val2);
    } else {
        throw ErrorException("SYNTAX ERROR");
    }

    if (condition) {
        program.setCurrentLine(lineNumber);
    }
}