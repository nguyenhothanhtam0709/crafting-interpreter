package dev.tamnguyen.lox;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Stack;

import dev.tamnguyen.lox.Expr.Conditional;
import dev.tamnguyen.lox.Stmt.Break;
import dev.tamnguyen.lox.Stmt.Continue;

/**
 * Semantic analyzer
 */
public class Resolver implements Expr.Visitor<Void>, Stmt.Visitor<Void> {

    private final Interpreter interpreter;
    /**
     * Tracking nested scopes
     */
    private final Stack<Map<String, Boolean>> scopes = new Stack<>();
    private FunctionType currentFunction = FunctionType.NONE;
    private BlockScopeType currentScope = BlockScopeType.GLOBAL;

    public Resolver(Interpreter interpreter) {
        this.interpreter = interpreter;
    }

    @Override
    public Void visitBlockStmt(Stmt.Block stmt) {
        beginScope();
        resolve(stmt.getStatements());
        endScope();
        return null;
    }

    @Override
    public Void visitClassStmt(Stmt.Class stmt) {
        declare(stmt.getName());

        beginScope();
        scopes.peek().put("this", true);

        for (Stmt.Function method : stmt.getMethods()) {
            FunctionType declaration = method.getName().getLexeme().equals("init")
                    ? FunctionType.INITIALIZER
                    : FunctionType.METHOD;
            resolveFunction(method, declaration);
        }

        endScope();

        define(stmt.getName());
        return null;
    }

    @Override
    public Void visitExpressionStmt(Stmt.Expression stmt) {
        resolve(stmt.getExpression());
        return null;
    }

    @Override
    public Void visitFunctionStmt(Stmt.Function stmt) {
        declare(stmt.getName());
        define(stmt.getName());

        resolveFunction(stmt, FunctionType.FUNCTION);
        return null;
    }

    @Override
    public Void visitIfStmt(Stmt.If stmt) {
        resolve(stmt.getCondition());
        resolve(stmt.getThenBranch());
        if (stmt.getElseBranch() != null) {
            resolve(stmt.getElseBranch());
        }
        return null;
    }

    @Override
    public Void visitPrintStmt(Stmt.Print stmt) {
        resolve(stmt.getExpression());
        return null;
    }

    @Override
    public Void visitReturnStmt(Stmt.Return stmt) {
        if (currentFunction == FunctionType.NONE) {
            Lox.error(stmt.getKeyword(), "Can't return from top-level code.");
        }

        if (stmt.getValue() != null) {
            if (currentFunction == FunctionType.INITIALIZER) {
                Lox.error(stmt.getKeyword(),
                        "Can't return a value from an initializer.");
            }

            resolve(stmt.getValue());
        }

        return null;
    }

    @Override
    public Void visitVarStmt(Stmt.Var stmt) {
        declare(stmt.getName());
        if (stmt.getInitializer() != null) {
            resolve(stmt.getInitializer());
        }
        define(stmt.getName());
        return null;
    }

    @Override
    public Void visitBreakStmt(Break stmt) {
        if (currentScope != BlockScopeType.LOOP) {
            Lox.error(stmt.getKeyword(), "Can't use break statement outside of loop.");
        }

        return null;
    }

    @Override
    public Void visitContinueStmt(Continue stmt) {
        Lox.error(stmt.getKeyword(), "Can't use continue statement outside of loop.");
        return null;
    }

    @Override
    public Void visitConditionalExpr(Conditional expr) {
        resolve(expr.getCondExpr());
        resolve(expr.getThenExpr());
        resolve(expr.getElseExpr());
        return null;
    }

    @Override
    public Void visitBinaryExpr(Expr.Binary expr) {
        resolve(expr.getLeft());
        resolve(expr.getRight());
        return null;
    }

    @Override
    public Void visitCallExpr(Expr.Call expr) {
        resolve(expr.getCallee());

        for (Expr argument : expr.getArguments()) {
            resolve(argument);
        }

        return null;
    }

    @Override
    public Void visitGetExpr(Expr.Get expr) {
        resolve(expr.getObject());
        return null;
    }

    @Override
    public Void visitGroupingExpr(Expr.Grouping expr) {
        resolve(expr.getExpression());
        return null;
    }

    @Override
    public Void visitLiteralExpr(Expr.Literal expr) {
        return null;
    }

    @Override
    public Void visitLogicalExpr(Expr.Logical expr) {
        resolve(expr.getLeft());
        resolve(expr.getRight());
        return null;
    }

    @Override
    public Void visitSetExpr(Expr.Set expr) {
        resolve(expr.getValue());
        resolve(expr.getObject());
        return null;
    }

    @Override
    public Void visitThisExpr(Expr.This expr) {
        resolveLocal(expr, expr.getKeyword());
        return null;
    }

    @Override
    public Void visitUnaryExpr(Expr.Unary expr) {
        resolve(expr.getRight());
        return null;
    }

    @Override
    public Void visitWhileStmt(Stmt.While stmt) {
        BlockScopeType enclosingScope = currentScope;
        currentScope = BlockScopeType.LOOP;

        resolve(stmt.getCondition());
        resolve(stmt.getBody());

        currentScope = enclosingScope;

        return null;
    }

    @Override
    public Void visitAssignExpr(Expr.Assign expr) {
        resolve(expr.getValue()); // resolve the expression for the assigned value.
        resolveLocal(expr, expr.getName()); // resolve the variable that’s being assigned to.
        return null;
    }

    @Override
    public Void visitVariableExpr(Expr.Variable expr) {
        if (!scopes.isEmpty() && scopes.peek().get(expr.getName().getLexeme()) == Boolean.FALSE) {
            Lox.error(expr.getName(), "Can't read local variable in its own initializer.");
        }

        resolveLocal(expr, expr.getName());
        return null;
    }

    public void resolve(List<Stmt> statements) {
        for (Stmt statement : statements) {
            resolve(statement);
        }
    }

    private void resolve(Stmt stmt) {
        stmt.accept(this);
    }

    private void resolve(Expr expr) {
        expr.accept(this);
    }

    private void resolveFunction(Stmt.Function function, FunctionType type) {
        FunctionType enclosingFunction = currentFunction;
        currentFunction = type;

        BlockScopeType enclosingScope = currentScope;
        currentScope = BlockScopeType.FUNCTION;

        beginScope();
        for (Token param : function.getParams()) {
            declare(param);
            define(param);
        }
        resolve(function.getBody());
        endScope();

        currentScope = enclosingScope;
        currentFunction = enclosingFunction;
    }

    private void beginScope() {
        scopes.push(new HashMap<String, Boolean>());
    }

    private void endScope() {
        scopes.pop();
    }

    private void declare(Token name) {
        if (scopes.isEmpty()) {
            return;
        }

        Map<String, Boolean> scope = scopes.peek();
        if (scope.containsKey(name.getLexeme())) {
            Lox.error(name, "Already a variable with this name in this scope.");
        }

        scope.put(name.getLexeme(), false); // Mark variable as uninitialized
    }

    private void define(Token name) {
        if (scopes.isEmpty()) {
            return;
        }

        scopes.peek().put(name.getLexeme(), true); // Mark variable as initialized
    }

    private void resolveLocal(Expr expr, Token name) {
        for (int i = scopes.size() - 1; i >= 0; i--) {
            if (scopes.get(i).containsKey(name.getLexeme())) {
                interpreter.resolve(expr, scopes.size() - 1 - i);
                return;
            }
        }
    }

    private enum FunctionType {
        NONE,
        FUNCTION,
        INITIALIZER,
        METHOD
    }

    private enum BlockScopeType {
        GLOBAL,
        FUNCTION,
        LOOP
    }
}
