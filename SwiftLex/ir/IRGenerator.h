#pragma once
#include "IR.h"
#include "../allNodes.h"
#include <memory>
#include <string>

class IRGenerator {
private:
    IR ir;
    
    // Generate IR from different node types
    std::string generateExpr(ExprNode* expr);
    void generateStmt(StmtNode* stmt);
    void generateStmtList(StmtListNode* stmtList);
    
    // Helper for binary operations
    std::string generateBinaryOp(ExprNode* expr);
    
    // Helper for unary operations
    std::string generateUnaryOp(ExprNode* expr);
    
    // Control flow helpers
    void generateIfElse(IfElseNode* ifElse);
    void generateLoop(LoopNode* loop);
    void generateWhileLoop(LoopNode* loop);
    void generateForLoop(LoopNode* loop);
    
    // Function helpers
    void generateFuncDecl(FuncDeclNode* funcDecl);
    void generateFuncCall(ExprNode* expr, std::string& result);
    
    // Assignment helper
    void generateAssignment(StmtNode* stmt);
    
    // Variable declaration helper
    void generateVarDeclaration(VarDeclarationListNode* varDeclList);
    
public:
    IRGenerator();
    
    // Main generation function
    bool generate(StmtListNode* root);
    
    // Get the generated IR
    IR& getIR();
    
    // Print IR
    void printIR() const;
};