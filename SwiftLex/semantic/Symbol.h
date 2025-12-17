#pragma once
#include <string>
#include <vector>
#include <memory>

enum class SymbolKind {
    Variable,
    Function,
    Parameter,
    Class,
    Struct,
    Enum
};

struct TypeInfo {
    std::string typeName;
    int arrayDimension;
    bool isInferred;
    bool isOptional;
    
    TypeInfo() : arrayDimension(0), isInferred(false), isOptional(false) {}
    TypeInfo(const std::string& name) : typeName(name), arrayDimension(0), 
                                         isInferred(false), isOptional(false) {}
    
    bool isCompatibleWith(const TypeInfo& other) const;
    bool isNumeric() const;
    std::string toString() const;
};

class Symbol {
public:
    std::string name;
    SymbolKind kind;
    TypeInfo type;
    int scopeLevel;
    bool isConstant;
    bool isInitialized;
    
    Symbol(const std::string& n, SymbolKind k, const TypeInfo& t)
        : name(n), kind(k), type(t), scopeLevel(0), 
          isConstant(false), isInitialized(false) {}
    
    virtual ~Symbol() = default;
    std::string toString() const;
};

class VariableSymbol : public Symbol {
public:
    VariableSymbol(const std::string& name, const TypeInfo& type, bool isConst)
        : Symbol(name, SymbolKind::Variable, type) {
        isConstant = isConst;
    }
};

class FunctionSymbol : public Symbol {
public:
    std::vector<std::shared_ptr<Symbol>> parameters;
    TypeInfo returnType;
    
    FunctionSymbol(const std::string& name, const TypeInfo& retType)
        : Symbol(name, SymbolKind::Function, retType), returnType(retType) {}
    
    std::string getSignature() const;
};

class ClassSymbol : public Symbol {
public:
    std::string parentClass;
    std::vector<std::shared_ptr<Symbol>> members;
    std::vector<std::shared_ptr<FunctionSymbol>> methods;
    
    ClassSymbol(const std::string& name)
        : Symbol(name, SymbolKind::Class, TypeInfo(name)) {}
    
    std::shared_ptr<Symbol> findMember(const std::string& memberName) const;
    std::shared_ptr<FunctionSymbol> findMethod(const std::string& methodName) const;
};