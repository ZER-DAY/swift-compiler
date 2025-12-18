#pragma once
#include "IRInstruction.h"
#include <vector>
#include <string>
#include <memory>
#include <iostream>

class IR {
private:
    std::vector<std::shared_ptr<IRInstruction>> instructions;
    int tempCounter;
    int labelCounter;
    
public:
    IR();
    
    // Add instruction
    void addInstruction(std::shared_ptr<IRInstruction> inst);
    
    // Generate temporary variable
    std::string newTemp();
    
    // Generate label
    std::string newLabel();
    
    // Get all instructions
    const std::vector<std::shared_ptr<IRInstruction>>& getInstructions() const;
    
    // Print all IR code
    void print() const;
    
    // Clear all
    void clear();
};