#ifndef INSTRUCTIONGENERATOR_H
#define INSTRUCTIONGENERATOR_H

#include <string>
#include <vector>
#include <random>

class InstructionGenerator {
private:
    std::mt19937 rng;
    int variableCounter;
    
    std::string generatePrintInstruction(const std::string& processName);
    std::string generateDeclareInstruction();
    std::string generateAddInstruction();
    std::string generateSubtractInstruction();
    std::string generateSleepInstruction();
    std::string generateForInstruction(int nestingLevel = 0);
    
    std::vector<std::string> generateNestedInstructions(int count);
    
    std::string getNextVariableName();
    
    uint16_t getRandomUint16();
    int getRandomSleepTicks();
    int getRandomLoopCount();
    
public:
    InstructionGenerator();
    
    // Main method to generate a complete instruction sequence
    std::vector<std::string> generateRandomInstructions(
        const std::string& processName,
        int minInstructions, 
        int maxInstructions
    );
    
    // Generate a single random instruction
    std::string generateRandomInstruction(
        const std::string& processName,
        bool allowNestedInstructions = true
    );
    
    void resetVariableCounter();
};

#endif
