#include "SemanticAnalyzer.h"
#include "../allNodes.h"
#include <iostream>
#include <sstream>

std::string SemanticError::toString() const {
    std::stringstream ss;
    ss << "Semantic Error: " << message;
    return ss.str();
}

SemanticAnalyzer::SemanticAnalyzer() : currentFunction(nullptr) {
}

bool SemanticAnalyzer::analyze(StmtListNode* root) {
    if (!root) {
        addError(SemanticError::ErrorType::Other, "Empty program");
        return false;
    }

    std::cout << "\n=== Starting Semantic Analysis ===" << std::endl;

    analyzeStmtList(root);

    std::cout << "\n=== Semantic Analysis Complete ===" << std::endl;
    std::cout << "Errors: " << errors.size() << std::endl;
    std::cout << "Warnings: " << warnings.size() << std::endl;

    if (hasErrors()) {
        printErrors();
    }

    if (!warnings.empty()) {
        printWarnings();
    }

    return !hasErrors();
}

void SemanticAnalyzer::analyzeStmtList(StmtListNode* stmtList) {
    if (!stmtList) return;

    for (auto stmt : stmtList->_vec) {
        if (stmt) {
            analyzeStmt(stmt);
        }
    }
}

void SemanticAnalyzer::analyzeStmt(StmtNode* stmt) {
    if (!stmt) return;

    switch (stmt->_type) {
    case StmtType::Expr:
        if (stmt->_expr) {
            analyzeExpr(stmt->_expr);
        }
        break;

    case StmtType::Assignment:
        if (stmt->_assignLeft && stmt->_assignRight) {
            if (stmt->_assignLeft->_type == ExprType::Id) {
                std::string varName = stmt->_assignLeft->_stringValue;
                auto symbol = symbolTable.lookupSymbol(varName);

                if (!symbol) {
                    addError(SemanticError::ErrorType::UndeclaredVariable,
                        "Use of undeclared identifier '" + varName + "'");
                }
                else if (symbol->isConstant) {
                    addError(SemanticError::ErrorType::ConstantReassignment,
                        "Cannot assign to 'let' constant '" + varName + "'");
                }
                else {
                    symbol->isInitialized = true;

                    TypeInfo leftType = analyzeExpr(stmt->_assignLeft);
                    TypeInfo rightType = analyzeExpr(stmt->_assignRight);

                    if (!checkTypeCompatibility(leftType, rightType)) {
                        addError(SemanticError::ErrorType::TypeMismatch,
                            "Cannot assign value of type '" + rightType.toString() +
                            "' to type '" + leftType.toString() + "'");
                    }
                }
            }
        }
        break;

    case StmtType::VarDeclarationList:
        if (stmt->_varDeclList) {
            for (auto varDecl : stmt->_varDeclList->_vec) {
                if (!varDecl) continue;

                std::string varName = varDecl->_varName;
                bool isConstant = varDecl->_isConstant;  //  الآن يقرأ من VarDeclarationNode!

                if (symbolTable.lookupSymbol(varName)) {
                    addError(SemanticError::ErrorType::RedeclaredVariable,
                        "Variable '" + varName + "' is already declared");
                    continue;
                }

                TypeInfo varType;

                if (varDecl->_type == VarDeclType::TypeKnown ||
                    varDecl->_type == VarDeclType::ValueAndTypeKnown) {
                    varType = getTypeFromNode(varDecl->_typeNode);
                }
                else if (varDecl->_type == VarDeclType::ValueKnown) {
                    varType = inferType(varDecl->_valueNode);
                    varType.isInferred = true;
                }

                if (varDecl->_type == VarDeclType::ValueAndTypeKnown) {
                    TypeInfo exprType = analyzeExpr(varDecl->_valueNode);
                    if (!checkTypeCompatibility(varType, exprType)) {
                        addError(SemanticError::ErrorType::TypeMismatch,
                            "Cannot assign value of type '" + exprType.toString() +
                            "' to type '" + varType.toString() + "'");
                    }
                }

                auto varSymbol = std::make_shared<VariableSymbol>(varName, varType, isConstant);
                varSymbol->isInitialized = (varDecl->_valueNode != nullptr);
                symbolTable.declareSymbol(varSymbol);
            }
        }
        break;

    case StmtType::Return:
        if (!symbolTable.isInFunction()) {
            addError(SemanticError::ErrorType::Other,
                "return statement outside of function");
        }
        break;

    case StmtType::Loop:
        if (stmt->_loop) {
            symbolTable.enterScope(Scope::ScopeType::Loop, "loop");

            if (stmt->_loop->_body) {
                analyzeStmtList(stmt->_loop->_body);
            }

            symbolTable.exitScope();
        }
        break;

    case StmtType::IfElse:
        if (stmt->_ifElse) {
            if (stmt->_ifElse->_conditions) {
                for (auto cond : stmt->_ifElse->_conditions->_vec) {
                    TypeInfo condType = analyzeExpr(cond);
                    if (condType.typeName != "Bool") {
                        addWarning("Condition should be Bool type, got " + condType.toString());
                    }
                }
            }

            if (stmt->_ifElse->_ifTrue) {
                symbolTable.enterScope(Scope::ScopeType::Block, "if-then");
                analyzeStmtList(stmt->_ifElse->_ifTrue);
                symbolTable.exitScope();
            }

            if (stmt->_ifElse->_hasElse) {
                if (stmt->_ifElse->_type == IfElseNodeType::ifElseSimple &&
                    stmt->_ifElse->_elseSimple) {
                    symbolTable.enterScope(Scope::ScopeType::Block, "if-else");
                    analyzeStmtList(stmt->_ifElse->_elseSimple);
                    symbolTable.exitScope();
                }
                else if (stmt->_ifElse->_type == IfElseNodeType::ifElseComplex &&
                    stmt->_ifElse->_elseComplex) {
                    // Recursive for else-if chains
                    StmtNode tempStmt;
                    tempStmt._type = StmtType::IfElse;
                    tempStmt._ifElse = stmt->_ifElse->_elseComplex;
                    analyzeStmt(&tempStmt);
                }
            }
        }
        break;

    case StmtType::FuncDecl:
        if (stmt->_funcDecl) {
            std::string funcName = stmt->_funcDecl->_idName;

            TypeInfo returnType("Void");
            if (stmt->_funcDecl->_hasNonVoidReturn && stmt->_funcDecl->_returnType) {
                returnType = getTypeFromNode(stmt->_funcDecl->_returnType);
            }

            auto funcSymbol = std::make_shared<FunctionSymbol>(funcName, returnType);

            if (stmt->_funcDecl->_hasArgs && stmt->_funcDecl->_argList) {
                for (auto arg : stmt->_funcDecl->_argList->_vec) {
                   if (arg && arg->_argType) {
   			TypeInfo paramType = getTypeFromNode(arg->_argType);
                        auto paramSymbol = std::make_shared<Symbol>(
                            arg->_argName, SymbolKind::Parameter, paramType);
                        funcSymbol->parameters.push_back(paramSymbol);
                    }
                }
            }

            symbolTable.declareSymbol(funcSymbol);

            symbolTable.enterScope(Scope::ScopeType::Function, funcName);
            currentFunction = funcSymbol;

            for (const auto& param : funcSymbol->parameters) {
                symbolTable.declareSymbol(param);
            }

            if (stmt->_funcDecl->_hasBody && stmt->_funcDecl->_body) {
                analyzeStmtList(stmt->_funcDecl->_body);
            }

            currentFunction = nullptr;
            symbolTable.exitScope();
        }
        break;

    default:
        break;
    }
}

TypeInfo SemanticAnalyzer::analyzeExpr(ExprNode* expr) {
    if (!expr) {
        return TypeInfo("Void");
    }

    switch (expr->_type) {
    case ExprType::Int:
        return TypeInfo("Int");

    case ExprType::Float:
        return TypeInfo("Float");

    case ExprType::Bool:
        return TypeInfo("Bool");

    case ExprType::String:
        return TypeInfo("String");

    case ExprType::Id: {
        std::string idName = expr->_stringValue;
        auto symbol = symbolTable.lookupSymbol(idName);

        if (!symbol) {
            addError(SemanticError::ErrorType::UndeclaredVariable,
                "Use of undeclared identifier '" + idName + "'");
            return TypeInfo("Void");
        }

        if (!symbol->isInitialized) {
            addWarning("Variable '" + idName + "' may not be initialized");
        }

        return symbol->type;
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
        return analyzeBinaryOp(expr);

    case ExprType::UnaryMinus:
    case ExprType::LogNot:
    case ExprType::BitNot:
        return analyzeUnaryOp(expr);

    case ExprType::Array:
        if (expr->_arrayExprList && !expr->_arrayExprList->_vec.empty()) {
            TypeInfo elementType = analyzeExpr(expr->_arrayExprList->_vec[0]);
            elementType.arrayDimension++;
            return elementType;
        }
        return TypeInfo("Array");

    case ExprType::FuncCall:
        if (expr->_funcCall) {
            std::string funcName = expr->_funcCall->_funcName;
            auto func = symbolTable.lookupFunction(funcName);

            if (!func) {
                addError(SemanticError::ErrorType::UndeclaredFunction,
                    "Use of undeclared function '" + funcName + "'");
                return TypeInfo("Void");
            }

            return func->returnType;
        }
        return TypeInfo("Void");

    default:
        return TypeInfo("Void");
    }
}

TypeInfo SemanticAnalyzer::analyzeBinaryOp(ExprNode* expr) {
    if (!expr || !expr->_left || !expr->_right) {
        return TypeInfo("Void");
    }

    TypeInfo leftType = analyzeExpr(expr->_left);
    TypeInfo rightType = analyzeExpr(expr->_right);

    switch (expr->_type) {
    case ExprType::Sum:
    case ExprType::Sub:
    case ExprType::Mul:
    case ExprType::Div:
    case ExprType::Modulus:
        if (!leftType.isNumeric() || !rightType.isNumeric()) {
            addError(SemanticError::ErrorType::InvalidOperation,
                "Arithmetic operations require numeric operands");
            return TypeInfo("Int");
        }
        return getCommonType(leftType, rightType);

    case ExprType::LT:
    case ExprType::GT:
    case ExprType::GTE:
    case ExprType::LTE:
        if (!leftType.isNumeric() || !rightType.isNumeric()) {
            addError(SemanticError::ErrorType::InvalidOperation,
                "Comparison operations require numeric operands");
        }
        return TypeInfo("Bool");

    case ExprType::EQ:
    case ExprType::NEQ:
        return TypeInfo("Bool");

    case ExprType::LogAnd:
    case ExprType::LogOr:
        if (leftType.typeName != "Bool" || rightType.typeName != "Bool") {
            addError(SemanticError::ErrorType::InvalidOperation,
                "Logical operations require Bool operands");
        }
        return TypeInfo("Bool");

    default:
        return TypeInfo("Void");
    }
}

TypeInfo SemanticAnalyzer::analyzeUnaryOp(ExprNode* expr) {
    if (!expr || !expr->_unary) {
        return TypeInfo("Void");
    }

    TypeInfo operandType = analyzeExpr(expr->_unary);

    switch (expr->_type) {
    case ExprType::UnaryMinus:
        if (!operandType.isNumeric()) {
            addError(SemanticError::ErrorType::InvalidOperation,
                "Unary minus requires numeric operand");
        }
        return operandType;

    case ExprType::LogNot:
        if (operandType.typeName != "Bool") {
            addError(SemanticError::ErrorType::InvalidOperation,
                "Logical NOT requires Bool operand");
        }
        return TypeInfo("Bool");

    default:
        return operandType;
    }
}

TypeInfo SemanticAnalyzer::inferType(ExprNode* expr) {
    TypeInfo type = analyzeExpr(expr);
    type.isInferred = true;
    return type;
}

TypeInfo SemanticAnalyzer::getTypeFromNode(TypeNode* typeNode) {
    if (!typeNode) {
        return TypeInfo("Void");
    }

    std::string typeName = typeNode->getName();
    TypeInfo type(typeName);

    return type;
}

bool SemanticAnalyzer::checkTypeCompatibility(const TypeInfo& expected, const TypeInfo& actual) {
    return actual.isCompatibleWith(expected);
}

TypeInfo SemanticAnalyzer::getCommonType(const TypeInfo& type1, const TypeInfo& type2) {
    if (type1.typeName == type2.typeName) {
        return type1;
    }

    if (type1.isNumeric() && type2.isNumeric()) {
        if (type1.typeName == "Float" || type2.typeName == "Float") {
            return TypeInfo("Float");
        }
        return TypeInfo("Int");
    }

    return type1;
}

void SemanticAnalyzer::addError(SemanticError::ErrorType type, const std::string& message) {
    errors.emplace_back(type, message);
}

void SemanticAnalyzer::addWarning(const std::string& message) {
    warnings.push_back(message);
}

void SemanticAnalyzer::printErrors() const {
    if (errors.empty()) {
        return;
    }

    std::cout << "\n=== Semantic Errors ===" << std::endl;
    for (const auto& error : errors) {
        std::cout << error.toString() << std::endl;
    }
}

void SemanticAnalyzer::printWarnings() const {
    if (warnings.empty()) {
        return;
    }

    std::cout << "\n=== Warnings ===" << std::endl;
    for (const auto& warning : warnings) {
        std::cout << "Warning: " << warning << std::endl;
    }
}

void SemanticAnalyzer::printSymbolTable() const {
    std::cout << "\n=== Symbol Table ===" << std::endl;
    symbolTable.printAllScopes();
}
