#include "Symbol.h"
#include <sstream>

bool TypeInfo::isCompatibleWith(const TypeInfo& other) const {
    if (typeName == other.typeName && arrayDimension == other.arrayDimension) {
        return true;
    }
    
    if (arrayDimension != other.arrayDimension) {
        return false;
    }
    
    if (isNumeric() && other.isNumeric()) {
        return true;
    }
    
    return false;
}

bool TypeInfo::isNumeric() const {
    return typeName == "Int" || typeName == "Float" || 
           typeName == "Double" || typeName == "UInt";
}

std::string TypeInfo::toString() const {
    std::stringstream ss;
    
    for (int i = 0; i < arrayDimension; i++) {
        ss << "[";
    }
    
    ss << typeName;
    
    for (int i = 0; i < arrayDimension; i++) {
        ss << "]";
    }
    
    if (isOptional) ss << "?";
    if (isInferred) ss << " (inferred)";
    
    return ss.str();
}

std::string Symbol::toString() const {
    std::stringstream ss;
    
    switch (kind) {
        case SymbolKind::Variable:
            ss << (isConstant ? "let " : "var ") << name << ": " << type.toString();
            break;
        case SymbolKind::Function:
            ss << "func " << name;
            break;
        case SymbolKind::Class:
            ss << "class " << name;
            break;
        case SymbolKind::Struct:
            ss << "struct " << name;
            break;
        default:
            ss << name;
    }
    
    return ss.str();
}

std::string FunctionSymbol::getSignature() const {
    std::stringstream ss;
    ss << name << "(";
    
    for (size_t i = 0; i < parameters.size(); i++) {
        if (i > 0) ss << ", ";
        ss << parameters[i]->name << ": " << parameters[i]->type.toString();
    }
    
    ss << ")";
    
    if (returnType.typeName != "Void" && !returnType.typeName.empty()) {
        ss << " -> " << returnType.toString();
    }
    
    return ss.str();
}

std::shared_ptr<Symbol> ClassSymbol::findMember(const std::string& memberName) const {
    for (const auto& member : members) {
        if (member->name == memberName) {
            return member;
        }
    }
    return nullptr;
}

std::shared_ptr<FunctionSymbol> ClassSymbol::findMethod(const std::string& methodName) const {
    for (const auto& method : methods) {
        if (method->name == methodName) {
            return method;
        }
    }
    return nullptr;
}