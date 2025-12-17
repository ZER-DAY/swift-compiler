#pragma once
#include <string>
#include <vector>
#include <map>
#include <memory>
#include "Symbol.h"

class Scope {
public:
    int level;
    Scope* parent;
    std::map<std::string, std::shared_ptr<Symbol>> symbols;
    std::string scopeName;
    
    enum class ScopeType {
        Global,
        Function,
        Class,
        Block,
        Loop
    };
    
    ScopeType type;
    
    Scope(int level, Scope* parent, ScopeType type, const std::string& name = "")
        : level(level), parent(parent), type(type), scopeName(name) {}
    
    std::shared_ptr<Symbol> findLocal(const std::string& name) const;
    std::shared_ptr<Symbol> findSymbol(const std::string& name) const;
    bool addSymbol(std::shared_ptr<Symbol> symbol);
    bool isDeclaredLocally(const std::string& name) const;
};

class SymbolTable {
private:
    std::vector<std::unique_ptr<Scope>> allScopes;
    Scope* currentScope;
    Scope* globalScope;
    int currentLevel;
    
    void initBuiltInFunctions();
    
public:
    SymbolTable();
    ~SymbolTable() = default;
    
    void enterScope(Scope::ScopeType type, const std::string& name = "");
    void exitScope();
    Scope* getCurrentScope() const { return currentScope; }
    Scope* getGlobalScope() const { return globalScope; }
    
    bool declareSymbol(std::shared_ptr<Symbol> symbol);
    std::shared_ptr<Symbol> lookupSymbol(const std::string& name) const;
    bool isSymbolDeclared(const std::string& name) const;
    
    std::shared_ptr<ClassSymbol> lookupClass(const std::string& className) const;
    std::shared_ptr<FunctionSymbol> lookupFunction(const std::string& funcName) const;
    
    bool isInLoop() const;
    bool isInFunction() const;
    
    void printCurrentScope() const;
    void printAllScopes() const;
};