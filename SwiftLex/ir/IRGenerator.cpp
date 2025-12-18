#include "IRGenerator.h"
#include <iostream>

IRGenerator::IRGenerator() {
}

bool IRGenerator::generate(StmtListNode* root) {
    if (!root) {
        std::cerr << "Error: Empty AST" << std::endl;
        return false;
    }
    
    std::cout << "\n=== Starting IR Generation ===" << std::endl;
    
    try {
        generateStmtList(root);
        std::cout << "=== IR Generation Complete ===" << std::endl;
        return true;
    } catch (const std::exception& e) {
        std::cerr << "IR Generation Error: " << e.what() << std::endl;
        return false;
    }
}

IR& IRGenerator::getIR() {
    return ir;
}

void IRGenerator::printIR() const {
    ir.print();
}

void IRGenerator::generateStmtList(StmtListNode* stmtList) {
    if (!stmtList) return;
    
    for (auto stmt : stmtList->_vec) {
        if (stmt) {
            generateStmt(stmt);
        }
    }
}

void IRGenerator::generateStmt(StmtNode* stmt) {
    if (!stmt) return;
    
    switch (stmt->_type) {
        case StmtType::Expr:
            if (stmt->_expr) {
                generateExpr(stmt->_expr);
            }
            break;
            
        case StmtType::Assignment:
            generateAssignment(stmt);
            break;
            
        case StmtType::VarDeclarationList:
            if (stmt->_varDeclList) {
                generateVarDeclaration(stmt->_varDeclList);
            }
            break;
            
        case StmtType::Return:
            if (stmt->_return) {
                if (stmt->_return->_expr) {
                    std::string retVal = generateExpr(stmt->_return->_expr);
                    ir.addInstruction(IRInstruction::createReturn(retVal));
                } else {
                    ir.addInstruction(IRInstruction::createReturn(""));
                }
            }
            break;
            
        case StmtType::IfElse:
            if (stmt->_ifElse) {
                generateIfElse(stmt->_ifElse);
            }
            break;
            
        case StmtType::Loop:
            if (stmt->_loop) {
                generateLoop(stmt->_loop);
            }
            break;
            
        case StmtType::FuncDecl:
            if (stmt->_funcDecl) {
                generateFuncDecl(stmt->_funcDecl);
            }
            break;
            
        default:
            // Other statement types
            break;
    }
}

std::string IRGenerator::generateExpr(ExprNode* expr) {
    if (!expr) {
        return "";
    }
    
    switch (expr->_type) {
        case ExprType::Int: {
            std::string temp = ir.newTemp();
            ir.addInstruction(IRInstruction::createLoadConst(temp, std::to_string(expr->_intValue)));
            return temp;
        }
        
        case ExprType::Float: {
            std::string temp = ir.newTemp();
            ir.addInstruction(IRInstruction::createLoadConst(temp, std::to_string(expr->_floatValue)));
            return temp;
        }
        
        case ExprType::String: {
            std::string temp = ir.newTemp();
            ir.addInstruction(IRInstruction::createLoadConst(temp, "\"" + expr->_stringValue + "\""));
            return temp;
        }
        
        case ExprType::Bool: {
            std::string temp = ir.newTemp();
            ir.addInstruction(IRInstruction::createLoadConst(temp, expr->_boolValue ? "true" : "false"));
            return temp;
        }
        
        case ExprType::Id: {
            // Just return the variable name
            return expr->_stringValue;
        }
        
        case ExprType::Sum:
        case ExprType::Sub:
        case ExprType::Mul:
        case ExprType::Div:
        case ExprType::Modulus:
        case ExprType::LT:
        case ExprType::GT:
        case ExprType::GTE:
        case ExprType::LTE:
        case ExprType::EQ:
        case ExprType::NEQ:
        case ExprType::LogAnd:
        case ExprType::LogOr:
            return generateBinaryOp(expr);
            
        case ExprType::UnaryMinus:
        case ExprType::LogNot:
            return generateUnaryOp(expr);
            
        case ExprType::FuncCall: {
            std::string result = ir.newTemp();
            generateFuncCall(expr, result);
            return result;
        }
        
        default:
            return "";
    }
}

std::string IRGenerator::generateBinaryOp(ExprNode* expr) {
    if (!expr || !expr->_left || !expr->_right) {
        return "";
    }
    
    // Generate code for left operand
    std::string left = generateExpr(expr->_left);
    
    // Generate code for right operand
    std::string right = generateExpr(expr->_right);
    
    // Create temporary for result
    std::string result = ir.newTemp();
    
    // Map ExprType to IROpcode
    IROpcode opcode;
    switch (expr->_type) {
        case ExprType::Sum:      opcode = IROpcode::ADD; break;
        case ExprType::Sub:      opcode = IROpcode::SUB; break;
        case ExprType::Mul:      opcode = IROpcode::MUL; break;
        case ExprType::Div:      opcode = IROpcode::DIV; break;
        case ExprType::Modulus:  opcode = IROpcode::MOD; break;
        case ExprType::LT:       opcode = IROpcode::LT; break;
        case ExprType::GT:       opcode = IROpcode::GT; break;
        case ExprType::GTE:      opcode = IROpcode::GTE; break;
        case ExprType::LTE:      opcode = IROpcode::LTE; break;
        case ExprType::EQ:       opcode = IROpcode::EQ; break;
        case ExprType::NEQ:      opcode = IROpcode::NEQ; break;
        case ExprType::LogAnd:   opcode = IROpcode::AND; break;
        case ExprType::LogOr:    opcode = IROpcode::OR; break;
        default:                 opcode = IROpcode::NOP; break;
    }
    
    // Add instruction
    ir.addInstruction(IRInstruction::createBinary(opcode, result, left, right));
    
    return result;
}

std::string IRGenerator::generateUnaryOp(ExprNode* expr) {
    if (!expr || !expr->_unary) {
        return "";
    }
    
    // Generate code for operand
    std::string operand = generateExpr(expr->_unary);
    
    // Create temporary for result
    std::string result = ir.newTemp();
    
    // Map ExprType to IROpcode
    IROpcode opcode;
    switch (expr->_type) {
        case ExprType::UnaryMinus: opcode = IROpcode::NEG; break;
        case ExprType::LogNot:     opcode = IROpcode::NOT; break;
        default:                   opcode = IROpcode::NOP; break;
    }
    
    // Add instruction
    ir.addInstruction(IRInstruction::createUnary(opcode, result, operand));
    
    return result;
}

void IRGenerator::generateAssignment(StmtNode* stmt) {
    if (!stmt || !stmt->_assignLeft || !stmt->_assignRight) {
        return;
    }
    
    // Generate right side
    std::string rightValue = generateExpr(stmt->_assignRight);
    
    // Get left side variable name
    std::string leftVar;
    if (stmt->_assignLeft->_type == ExprType::Id) {
        leftVar = stmt->_assignLeft->_stringValue;
    } else {
        // For more complex left sides, we'd need more handling
        leftVar = generateExpr(stmt->_assignLeft);
    }
    
    // Create assignment
    ir.addInstruction(IRInstruction::createAssign(leftVar, rightValue));
}

void IRGenerator::generateVarDeclaration(VarDeclarationListNode* varDeclList) {
    if (!varDeclList) return;
    
    for (auto varDecl : varDeclList->_vec) {
        if (!varDecl) continue;
        
        std::string varName = varDecl->_varName;
        
        // If has initial value
        if (varDecl->_valueNode) {
            std::string value = generateExpr(varDecl->_valueNode);
            ir.addInstruction(IRInstruction::createAssign(varName, value));
        }
        // If no initial value, we could initialize to 0 or skip
    }
}

void IRGenerator::generateIfElse(IfElseNode* ifElse) {
    if (!ifElse) return;
    
    // Generate labels
    std::string elseLabel = ir.newLabel();
    std::string endLabel = ir.newLabel();
    
    // Generate condition
    if (ifElse->_conditions && !ifElse->_conditions->_vec.empty()) {
        std::string condition = generateExpr(ifElse->_conditions->_vec[0]);
        
        // If condition is false, jump to else
        ir.addInstruction(IRInstruction::createIfFalse(condition, elseLabel));
    }
    
    // Generate then block
    if (ifElse->_ifTrue) {
        generateStmtList(ifElse->_ifTrue);
    }
    
    // Jump to end
    ir.addInstruction(IRInstruction::createGoto(endLabel));
    
    // Else label
    ir.addInstruction(IRInstruction::createLabel(elseLabel));
    
    // Generate else block
    if (ifElse->_hasElse) {
        if (ifElse->_type == IfElseNodeType::ifElseSimple && ifElse->_elseSimple) {
            generateStmtList(ifElse->_elseSimple);
        } else if (ifElse->_type == IfElseNodeType::ifElseComplex && ifElse->_elseComplex) {
            generateIfElse(ifElse->_elseComplex);
        }
    }
    
    // End label
    ir.addInstruction(IRInstruction::createLabel(endLabel));
}

void IRGenerator::generateLoop(LoopNode* loop) {
    if (!loop) return;
    
    // Simplified: detect loop type by checking what fields are set
    if (loop->_condition && !loop->_condition->_vec.empty()) {
        // Has condition = while/repeat-while loop
        generateWhileLoop(loop);
    } else {
        // No condition = for-in or simple body
        if (loop->_body) {
            generateStmtList(loop->_body);
        }
    }
}

void IRGenerator::generateWhileLoop(LoopNode* loop) {
    // Generate labels
    std::string startLabel = ir.newLabel();
    std::string endLabel = ir.newLabel();
    
    // Start label
    ir.addInstruction(IRInstruction::createLabel(startLabel));
    
    // Generate condition
    if (loop->_condition && !loop->_condition->_vec.empty()) {
        std::string condition = generateExpr(loop->_condition->_vec[0]);
        
        // If condition is false, jump to end
        ir.addInstruction(IRInstruction::createIfFalse(condition, endLabel));
    }
    
    // Generate body
    if (loop->_body) {
        generateStmtList(loop->_body);
    }
    
    // Jump back to start
    ir.addInstruction(IRInstruction::createGoto(startLabel));
    
    // End label
    ir.addInstruction(IRInstruction::createLabel(endLabel));
}

void IRGenerator::generateForLoop(LoopNode* loop) {
    // For-in loop: for i in expr { body }
    // Simplified: just generate the body (full implementation would handle iteration)
    
    if (loop->_body) {
        generateStmtList(loop->_body);
    }
}

void IRGenerator::generateFuncDecl(FuncDeclNode* funcDecl) {
    if (!funcDecl) return;
    
    std::string funcName = funcDecl->_idName;
    
    // Use label to mark function start
    ir.addInstruction(IRInstruction::createLabel("func_" + funcName));
    
    // Generate function body
    if (funcDecl->_hasBody && funcDecl->_body) {
        generateStmtList(funcDecl->_body);
    }
    
    // Use label to mark function end
    ir.addInstruction(IRInstruction::createLabel("func_end_" + funcName));
}

void IRGenerator::generateFuncCall(ExprNode* expr, std::string& result) {
    if (!expr || !expr->_funcCall) return;
    
    std::string funcName = expr->_funcCall->_funcName;
    
    // For now, simplified function call
    // Full implementation would handle parameters properly
    int numParams = 0;
    
    // Generate call
    ir.addInstruction(IRInstruction::createCall(result, funcName, numParams));
}
