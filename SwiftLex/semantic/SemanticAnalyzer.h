#pragma once
#include <string>
#include <vector>
#include <memory>
#include "SymbolTable.h"

// Forward declarations
class StmtNode;
class StmtListNode;
class ExprNode;
class VarDeclarationNode;
class FuncDeclNode;
class TypeNode;

class SemanticError {
public:
    enum class ErrorType {
        UndeclaredVariable,
        RedeclaredVariable,
        TypeMismatch,
        InvalidOperation,
        UndeclaredFunction,
        BreakOutsideLoop,
        ContinueOutsideLoop,
        ConstantReassignment,
        Other
    };
    
    ErrorType type;
    std::string message;
    
    SemanticError(ErrorType type, const std::string& msg)
        : type(type), message(msg) {}
    
    std::string toString() const;
};

class SemanticAnalyzer {
private:
    SymbolTable symbolTable;
    std::vector<SemanticError> errors;
    std::vector<std::string> warnings;
    
    std::shared_ptr<FunctionSymbol> currentFunction;
    
public:
    SemanticAnalyzer();
    
    // Main analysis method
    bool analyze(StmtListNode* root);
    
    // Statement analysis
    void analyzeStmtList(StmtListNode* stmtList);
    void analyzeStmt(StmtNode* stmt);
    
    // Expression analysis
    TypeInfo analyzeExpr(ExprNode* expr);
    TypeInfo analyzeBinaryOp(ExprNode* expr);
    TypeInfo analyzeUnaryOp(ExprNode* expr);
    
    // Type inference
    TypeInfo inferType(ExprNode* expr);
    TypeInfo getTypeFromNode(TypeNode* typeNode);
    
    // Type checking
    bool checkTypeCompatibility(const TypeInfo& expected, const TypeInfo& actual);
    TypeInfo getCommonType(const TypeInfo& type1, const TypeInfo& type2);
    
    // Error handling
    void addError(SemanticError::ErrorType type, const std::string& message);
    void addWarning(const std::string& message);
    
    // Results
    const std::vector<SemanticError>& getErrors() const { return errors; }
    const std::vector<std::string>& getWarnings() const { return warnings; }
    bool hasErrors() const { return !errors.empty(); }
    
    void printErrors() const;
    void printWarnings() const;
    void printSymbolTable() const;
};