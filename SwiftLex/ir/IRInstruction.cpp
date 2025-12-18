#include "IRInstruction.h"
#include <sstream>

std::shared_ptr<IRInstruction> IRInstruction::createBinary(
    IROpcode op,
    const std::string& result,
    const std::string& arg1,
    const std::string& arg2) 
{
    auto inst = std::shared_ptr<IRInstruction>(new IRInstruction());
    inst->opcode = op;
    inst->result = result;
    inst->arg1 = arg1;
    inst->arg2 = arg2;
    return inst;
}

std::shared_ptr<IRInstruction> IRInstruction::createUnary(
    IROpcode op,
    const std::string& result,
    const std::string& arg1)
{
    auto inst = std::shared_ptr<IRInstruction>(new IRInstruction());
    inst->opcode = op;
    inst->result = result;
    inst->arg1 = arg1;
    return inst;
}

std::shared_ptr<IRInstruction> IRInstruction::createAssign(
    const std::string& result,
    const std::string& value)
{
    auto inst = std::shared_ptr<IRInstruction>(new IRInstruction());
    inst->opcode = IROpcode::ASSIGN;
    inst->result = result;
    inst->arg1 = value;
    return inst;
}

std::shared_ptr<IRInstruction> IRInstruction::createLoadConst(
    const std::string& result,
    const std::string& value)
{
    auto inst = std::shared_ptr<IRInstruction>(new IRInstruction());
    inst->opcode = IROpcode::LOAD_CONST;
    inst->result = result;
    inst->arg1 = value;
    return inst;
}

std::shared_ptr<IRInstruction> IRInstruction::createLabel(
    const std::string& labelName)
{
    auto inst = std::shared_ptr<IRInstruction>(new IRInstruction());
    inst->opcode = IROpcode::LABEL;
    inst->label = labelName;
    return inst;
}

std::shared_ptr<IRInstruction> IRInstruction::createGoto(
    const std::string& labelName)
{
    auto inst = std::shared_ptr<IRInstruction>(new IRInstruction());
    inst->opcode = IROpcode::GOTO;
    inst->label = labelName;
    return inst;
}

std::shared_ptr<IRInstruction> IRInstruction::createIfFalse(
    const std::string& condition,
    const std::string& labelName)
{
    auto inst = std::shared_ptr<IRInstruction>(new IRInstruction());
    inst->opcode = IROpcode::IF_FALSE;
    inst->arg1 = condition;
    inst->label = labelName;
    return inst;
}

std::shared_ptr<IRInstruction> IRInstruction::createParam(
    const std::string& param)
{
    auto inst = std::shared_ptr<IRInstruction>(new IRInstruction());
    inst->opcode = IROpcode::PARAM;
    inst->arg1 = param;
    return inst;
}

std::shared_ptr<IRInstruction> IRInstruction::createCall(
    const std::string& result,
    const std::string& funcName,
    int numParams)
{
    auto inst = std::shared_ptr<IRInstruction>(new IRInstruction());
    inst->opcode = IROpcode::CALL;
    inst->result = result;
    inst->funcName = funcName;
    inst->arg1 = std::to_string(numParams);
    return inst;
}

std::shared_ptr<IRInstruction> IRInstruction::createReturn(
    const std::string& value)
{
    auto inst = std::shared_ptr<IRInstruction>(new IRInstruction());
    inst->opcode = IROpcode::RETURN;
    inst->arg1 = value;
    return inst;
}

std::shared_ptr<IRInstruction> IRInstruction::createPrint(
    const std::string& value)
{
    auto inst = std::shared_ptr<IRInstruction>(new IRInstruction());
    inst->opcode = IROpcode::PRINT;
    inst->arg1 = value;
    return inst;
}

std::string IRInstruction::toString() const {
    std::stringstream ss;
    
    switch (opcode) {
        // Binary operations
        case IROpcode::ADD:
            ss << result << " = " << arg1 << " + " << arg2;
            break;
        case IROpcode::SUB:
            ss << result << " = " << arg1 << " - " << arg2;
            break;
        case IROpcode::MUL:
            ss << result << " = " << arg1 << " * " << arg2;
            break;
        case IROpcode::DIV:
            ss << result << " = " << arg1 << " / " << arg2;
            break;
        case IROpcode::MOD:
            ss << result << " = " << arg1 << " % " << arg2;
            break;
            
        // Unary operations
        case IROpcode::NEG:
            ss << result << " = -" << arg1;
            break;
        case IROpcode::NOT:
            ss << result << " = !" << arg1;
            break;
            
        // Assignment
        case IROpcode::ASSIGN:
            ss << result << " = " << arg1;
            break;
        case IROpcode::LOAD_CONST:
            ss << result << " = " << arg1;
            break;
            
        // Comparison
        case IROpcode::LT:
            ss << result << " = " << arg1 << " < " << arg2;
            break;
        case IROpcode::GT:
            ss << result << " = " << arg1 << " > " << arg2;
            break;
        case IROpcode::LTE:
            ss << result << " = " << arg1 << " <= " << arg2;
            break;
        case IROpcode::GTE:
            ss << result << " = " << arg1 << " >= " << arg2;
            break;
        case IROpcode::EQ:
            ss << result << " = " << arg1 << " == " << arg2;
            break;
        case IROpcode::NEQ:
            ss << result << " = " << arg1 << " != " << arg2;
            break;
            
        // Logical
        case IROpcode::AND:
            ss << result << " = " << arg1 << " && " << arg2;
            break;
        case IROpcode::OR:
            ss << result << " = " << arg1 << " || " << arg2;
            break;
            
        // Control flow
        case IROpcode::LABEL:
            ss << label << ":";
            break;
        case IROpcode::GOTO:
            ss << "goto " << label;
            break;
        case IROpcode::IF_FALSE:
            ss << "if_false " << arg1 << " goto " << label;
            break;
        case IROpcode::IF_TRUE:
            ss << "if_true " << arg1 << " goto " << label;
            break;
            
        // Functions
        case IROpcode::PARAM:
            ss << "param " << arg1;
            break;
        case IROpcode::CALL:
            ss << result << " = call " << funcName << ", " << arg1;
            break;
        case IROpcode::RETURN:
            if (!arg1.empty()) {
                ss << "return " << arg1;
            } else {
                ss << "return";
            }
            break;
        case IROpcode::FUNC_BEGIN:
            ss << "func_begin " << funcName;
            break;
        case IROpcode::FUNC_END:
            ss << "func_end " << funcName;
            break;
            
        // Misc
        case IROpcode::PRINT:
            ss << "print " << arg1;
            break;
        case IROpcode::NOP:
            ss << "nop";
            break;
            
        default:
            ss << "unknown";
            break;
    }
    
    return ss.str();
}