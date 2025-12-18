#include "IR.h"

IR::IR() : tempCounter(0), labelCounter(0) {
}

void IR::addInstruction(std::shared_ptr<IRInstruction> inst) {
    instructions.push_back(inst);
}

std::string IR::newTemp() {
    return "t" + std::to_string(tempCounter++);
}

std::string IR::newLabel() {
    return "L" + std::to_string(labelCounter++);
}

const std::vector<std::shared_ptr<IRInstruction>>& IR::getInstructions() const {
    return instructions;
}

void IR::print() const {
    std::cout << "\n=== IR Code ===" << std::endl;
    for (const auto& inst : instructions) {
        std::cout << inst->toString() << std::endl;
    }
    std::cout << "===============\n" << std::endl;
}

void IR::clear() {
    instructions.clear();
    tempCounter = 0;
    labelCounter = 0;
}