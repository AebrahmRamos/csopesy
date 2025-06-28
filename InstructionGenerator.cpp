#include "InstructionGenerator.h"
#include <sstream>
#include <algorithm>
#include <ctime>

InstructionGenerator::InstructionGenerator() : rng(std::time(nullptr)), variableCounter(0) {
}

std::vector<std::string> InstructionGenerator::generateRandomInstructions(
    const std::string& processName,
    int minInstructions, 
    int maxInstructions
) {
    resetVariableCounter();

    std::uniform_int_distribution<int> countDist(minInstructions, maxInstructions);
    int instructionCount = countDist(rng);
    
    std::vector<std::string> instructions;
    instructions.reserve(instructionCount);
    
    for (int i = 0; i < instructionCount; ++i) {
        instructions.push_back(generateRandomInstruction(processName, true));
    }
    
    return instructions;
}

std::string InstructionGenerator::generateRandomInstruction(
    const std::string& processName,
    bool allowNestedInstructions
) {
    // Instruction types: PRINT, DECLARE, ADD, SUBTRACT, SLEEP, FOR
    std::uniform_int_distribution<int> typeDist(0, allowNestedInstructions ? 5 : 3);
    int instructionType = typeDist(rng);
    
    switch (instructionType) {
        case 0: return generatePrintInstruction(processName);
        case 1: return generateDeclareInstruction();
        case 2: return generateAddInstruction();
        case 3: return generateSubtractInstruction();
        case 4: return generateSleepInstruction();
        case 5: return generateForInstruction();
        default: return generatePrintInstruction(processName);
    }
}

std::string InstructionGenerator::generatePrintInstruction(const std::string& processName) {
    // To comply with the reqquirment na mag print ng Hello World from <process_name>
    return "PRINT(\"Hello world from " + processName + "!\")";
}

std::string InstructionGenerator::generateDeclareInstruction() {
    std::string varName = getNextVariableName();
    uint16_t value = getRandomUint16();
    return "DECLARE(" + varName + ", " + std::to_string(value) + ")";
}

std::string InstructionGenerator::generateAddInstruction() {
    std::string target = getNextVariableName();
    std::string source1 = getNextVariableName();
    
    // 50 50 randomized chance to use variable or value
    std::uniform_int_distribution<int> choiceDist(0, 1);
    std::string source2;
    if (choiceDist(rng) == 0) {
        source2 = getNextVariableName();
    } else {
        source2 = std::to_string(getRandomUint16());
    }
    
    return "ADD(" + target + ", " + source1 + ", " + source2 + ")";
}

std::string InstructionGenerator::generateSubtractInstruction() {
    std::string target = getNextVariableName();
    std::string source1 = getNextVariableName();
    
    // 50 50 randomized chance to use variable or value
    std::uniform_int_distribution<int> choiceDist(0, 1);
    std::string source2;
    if (choiceDist(rng) == 0) {
        source2 = getNextVariableName();
    } else {
        source2 = std::to_string(getRandomUint16());
    }
    
    return "SUBTRACT(" + target + ", " + source1 + ", " + source2 + ")";
}

std::string InstructionGenerator::generateSleepInstruction() {
    int ticks = getRandomSleepTicks();
    return "SLEEP(" + std::to_string(ticks) + ")";
}

std::string InstructionGenerator::generateForInstruction(int nestingLevel) {
    // Limit nesting to 3 levels as specified
    if (nestingLevel >= 3) {
        return generatePrintInstruction("nested");
    }
    
    int loopCount = getRandomLoopCount();
    int nestedInstructionCount = std::uniform_int_distribution<int>(1, 3)(rng);
    
    std::vector<std::string> nestedInstructions = generateNestedInstructions(nestedInstructionCount);
    
    // Join instructions with semicolon
    std::string instructionBlock;
    for (size_t i = 0; i < nestedInstructions.size(); ++i) {
        if (i > 0) instructionBlock += ";";
        instructionBlock += nestedInstructions[i];
    }
    
    return "FOR(" + instructionBlock + ", " + std::to_string(loopCount) + ")";
}

std::vector<std::string> InstructionGenerator::generateNestedInstructions(int count) {
    std::vector<std::string> instructions;
    instructions.reserve(count);
    
    for (int i = 0; i < count; ++i) {
        // Only allow PRINT, DECLARE, ADD, SUBTRACT in nested blocks (no FOR or SLEEP)
        instructions.push_back(generateRandomInstruction("nested", false));
    }
    
    return instructions;
}

std::string InstructionGenerator::getNextVariableName() {
    return "var" + std::to_string(variableCounter++);
}

uint16_t InstructionGenerator::getRandomUint16() {
    std::uniform_int_distribution<uint16_t> dist(0, 65535);
    return dist(rng);
}

int InstructionGenerator::getRandomSleepTicks() {
    std::uniform_int_distribution<int> dist(1, 10);
    return dist(rng);
}

int InstructionGenerator::getRandomLoopCount() {
    std::uniform_int_distribution<int> dist(1, 5);
    return dist(rng);
}

void InstructionGenerator::resetVariableCounter() {
    variableCounter = 0;
}
