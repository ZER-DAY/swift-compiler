#include "SymbolTable.h"
#include <iostream>

// Scope methods
std::shared_ptr<Symbol> Scope::findLocal(const std::string& name) const {
    auto it = symbols.find(name);
    if (it != symbols.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<Symbol> Scope::findSymbol(const std::string& name) const {
    auto symbol = findLocal(name);
    if (symbol) {
        return symbol;
    }
    
    if (parent) {
        return parent->findSymbol(name);
    }
    
    return nullptr;
}

bool Scope::addSymbol(std::shared_ptr<Symbol> symbol) {
    if (!symbol) return false;
    
    if (isDeclaredLocally(symbol->name)) {
        return false;
    }
    
    symbol->scopeLevel = level;
    symbols[symbol->name] = symbol;
    return true;
}

bool Scope::isDeclaredLocally(const std::string& name) const {
    return symbols.find(name) != symbols.end();
}

// SymbolTable methods
SymbolTable::SymbolTable() : currentLevel(0) {
    auto scope = std::make_unique<Scope>(0, nullptr, Scope::ScopeType::Global, "global");
    globalScope = scope.get();
    currentScope = globalScope;
    allScopes.push_back(std::move(scope));
    
    initBuiltInFunctions();
}

void SymbolTable::initBuiltInFunctions() {
    // print function
    auto printFunc = std::make_shared<FunctionSymbol>("print", TypeInfo("Void"));
    auto param = std::make_shared<Symbol>("items", SymbolKind::Parameter, TypeInfo("String"));
    printFunc->parameters.push_back(param);
    globalScope->addSymbol(printFunc);
}

void SymbolTable::enterScope(Scope::ScopeType type, const std::string& name) {
    currentLevel++;
    auto scope = std::make_unique<Scope>(currentLevel, currentScope, type, name);
    currentScope = scope.get();
    allScopes.push_back(std::move(scope));
}

void SymbolTable::exitScope() {
    if (currentScope && currentScope->parent) {
        currentScope = currentScope->parent;
        currentLevel--;
    }
}

bool SymbolTable::declareSymbol(std::shared_ptr<Symbol> symbol) {
    if (!symbol || !currentScope) {
        return false;
    }
    
    return currentScope->addSymbol(symbol);
}

std::shared_ptr<Symbol> SymbolTable::lookupSymbol(const std::string& name) const {
    if (!currentScope) {
        return nullptr;
    }
    
    return currentScope->findSymbol(name);
}

bool SymbolTable::isSymbolDeclared(const std::string& name) const {
    return lookupSymbol(name) != nullptr;
}

std::shared_ptr<ClassSymbol> SymbolTable::lookupClass(const std::string& className) const {
    auto symbol = lookupSymbol(className);
    if (symbol && (symbol->kind == SymbolKind::Class || symbol->kind == SymbolKind::Struct)) {
        return std::dynamic_pointer_cast<ClassSymbol>(symbol);
    }
    return nullptr;
}

std::shared_ptr<FunctionSymbol> SymbolTable::lookupFunction(const std::string& funcName) const {
    auto symbol = lookupSymbol(funcName);
    if (symbol && symbol->kind == SymbolKind::Function) {
        return std::dynamic_pointer_cast<FunctionSymbol>(symbol);
    }
    return nullptr;
}

bool SymbolTable::isInLoop() const {
    Scope* scope = currentScope;
    while (scope) {
        if (scope->type == Scope::ScopeType::Loop) {
            return true;
        }
        scope = scope->parent;
    }
    return false;
}

bool SymbolTable::isInFunction() const {
    Scope* scope = currentScope;
    while (scope) {
        if (scope->type == Scope::ScopeType::Function) {
            return true;
        }
        scope = scope->parent;
    }
    return false;
}

void SymbolTable::printCurrentScope() const {
    if (!currentScope) {
        std::cout << "No current scope" << std::endl;
        return;
    }
    
    std::cout << "=== Current Scope (Level " << currentScope->level << "): " 
              << currentScope->scopeName << " ===" << std::endl;
    
    for (const auto& pair : currentScope->symbols) {
        std::cout << "  " << pair.second->toString() << std::endl;
    }
}

void SymbolTable::printAllScopes() const {
    std::cout << "=== All Scopes ===" << std::endl;
    for (const auto& scope : allScopes) {
        std::cout << "Scope (Level " << scope->level << "): " << scope->scopeName << std::endl;
        for (const auto& pair : scope->symbols) {
            std::cout << "  " << pair.second->toString() << std::endl;
        }
    }
}