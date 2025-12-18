#pragma once
#include <string>
#include <memory>

enum class IROpcode {
    // Arithmetic
    ADD, SUB, MUL, DIV, MOD,
    
    // Unary
    NEG, NOT,
    
    // Assignment
    ASSIGN, LOAD_CONST,
    
    // Comparison
    LT, GT, LTE, GTE, EQ, NEQ,
    
    // Logical
    AND, OR,
    
    // Control Flow
    LABEL, GOTO, IF_FALSE, IF_TRUE,
    
    // Functions
    PARAM, CALL, RETURN, FUNC_BEGIN, FUNC_END,
    
    // Misc
    PRINT, NOP
};

class IRInstruction {
public:
    IROpcode opcode;
    
    std::string result;      // مثال: t1, x
    std::string arg1;        // مثال: t2, 10
    std::string arg2;        // مثال: t3, 20
    
    std::string label;       // للـ labels: L1, L2
    std::string funcName;    // للـ function calls
    
    // Constructors
    static std::shared_ptr<IRInstruction> createBinary(
        IROpcode op, 
        const std::string& result,
        const std::string& arg1,
        const std::string& arg2
    );
    
    static std::shared_ptr<IRInstruction> createUnary(
        IROpcode op,
        const std::string& result,
        const std::string& arg1
    );
    
    static std::shared_ptr<IRInstruction> createAssign(
        const std::string& result,
        const std::string& value
    );
    
    static std::shared_ptr<IRInstruction> createLoadConst(
        const std::string& result,
        const std::string& value
    );
    
    static std::shared_ptr<IRInstruction> createLabel(
        const std::string& label
    );
    
    static std::shared_ptr<IRInstruction> createGoto(
        const std::string& label
    );
    
    static std::shared_ptr<IRInstruction> createIfFalse(
        const std::string& condition,
        const std::string& label
    );
    
    static std::shared_ptr<IRInstruction> createParam(
        const std::string& param
    );
    
    static std::shared_ptr<IRInstruction> createCall(
        const std::string& result,
        const std::string& funcName,
        int numParams
    );
    
    static std::shared_ptr<IRInstruction> createReturn(
        const std::string& value
    );
    
    static std::shared_ptr<IRInstruction> createPrint(
        const std::string& value
    );
    
    // للطباعة
    std::string toString() const;
    
private:
    IRInstruction() = default;
};