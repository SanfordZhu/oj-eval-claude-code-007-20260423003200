/*
 * File: program.cpp
 * -----------------
 * This file is a stub implementation of the program.h interface
 * in which none of the methods do anything beyond returning a
 * value of the correct type.  Your job is to fill in the bodies
 * of each of these methods with an implementation that satisfies
 * the performance guarantees specified in the assignment.
 */

#include "program.hpp"
#include <iostream>
#include <sstream>
#include <map>
#include <set>

Program::Program() : currentLine(-1), running(false) {}

Program::~Program() {
    clear();
}

void Program::clear() {
    // Clear all parsed statements
    for (auto& pair : parsedStatements) {
        delete pair.second;
    }
    sourceLines.clear();
    parsedStatements.clear();
    lineNumbers.clear();
    currentLine = -1;
    running = false;
}

void Program::addSourceLine(int lineNumber, const std::string &line) {
    // Remove existing parsed statement if any
    if (parsedStatements.find(lineNumber) != parsedStatements.end()) {
        delete parsedStatements[lineNumber];
        parsedStatements.erase(lineNumber);
    }

    // Add or update source line
    sourceLines[lineNumber] = line;
    lineNumbers.insert(lineNumber);
}

void Program::removeSourceLine(int lineNumber) {
    // Remove parsed statement if exists
    if (parsedStatements.find(lineNumber) != parsedStatements.end()) {
        delete parsedStatements[lineNumber];
        parsedStatements.erase(lineNumber);
    }

    // Remove source line
    sourceLines.erase(lineNumber);
    lineNumbers.erase(lineNumber);
}

std::string Program::getSourceLine(int lineNumber) {
    auto it = sourceLines.find(lineNumber);
    if (it != sourceLines.end()) {
        return it->second;
    }
    return "";
}

void Program::setParsedStatement(int lineNumber, Statement *stmt) {
    if (sourceLines.find(lineNumber) == sourceLines.end()) {
        throw ErrorException("LINE NUMBER ERROR");
    }

    // Delete existing statement if any
    if (parsedStatements.find(lineNumber) != parsedStatements.end()) {
        delete parsedStatements[lineNumber];
    }

    parsedStatements[lineNumber] = stmt;
}

Statement *Program::getParsedStatement(int lineNumber) {
   auto it = parsedStatements.find(lineNumber);
   if (it != parsedStatements.end()) {
       return it->second;
   }
   return nullptr;
}

int Program::getFirstLineNumber() {
    if (lineNumbers.empty()) {
        return -1;
    }
    return *lineNumbers.begin();
}

int Program::getNextLineNumber(int lineNumber) {
    auto it = lineNumbers.upper_bound(lineNumber);
    if (it != lineNumbers.end()) {
        return *it;
    }
    return -1;
}

// Additional methods
void Program::listProgram(std::ostream& os) const {
    for (int lineNum : lineNumbers) {
        os << sourceLines.at(lineNum) << std::endl;
    }
}

void Program::runProgram(EvalState& state) {
    if (lineNumbers.empty()) {
        return;
    }

    running = true;
    currentLine = getFirstLineNumber();

    while (running && currentLine != -1) {
        Statement* stmt = getParsedStatement(currentLine);
        if (stmt) {
            try {
                int lineBeforeExecute = currentLine;
                stmt->execute(state, *this);

                // If statement didn't change the current line (i.e., not a GOTO or IF),
                // advance to next line
                if (running && currentLine == lineBeforeExecute) {
                    currentLine = getNextLineNumber(currentLine);
                }
            } catch (ErrorException& e) {
                running = false;
                throw e;
            }
        } else {
            // Move to next line if no statement (shouldn't happen)
            currentLine = getNextLineNumber(currentLine);
        }
    }

    running = false;
    currentLine = -1;
}

bool Program::hasLine(int lineNumber) const {
    return lineNumbers.find(lineNumber) != lineNumbers.end();
}

int Program::getCurrentLine() const {
    return currentLine;
}

void Program::setCurrentLine(int lineNumber) {
    if (hasLine(lineNumber)) {
        currentLine = lineNumber;
    } else {
        throw ErrorException("LINE NUMBER ERROR");
    }
}

void Program::resetCurrentLine() {
    currentLine = -1;
    running = false;
}