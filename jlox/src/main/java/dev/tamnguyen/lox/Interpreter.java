package dev.tamnguyen.lox;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import lombok.Getter;

/**
 * A tree-walking interpreter
 */
public class Interpreter implements Expr.Visitor<Object>, Stmt.Visitor<Void> {

    @Getter
    private final Environment globals = new Environment();
    private Environment environment = globals;
    private final Map<Expr, Integer> locals = new HashMap<>();

    public Interpreter() {
        globals.define("clock", new LoxCallable() {
            @Override
            public int arity() {
                return 0;
            }

            @Override
            public Object call(Interpreter interpreter, List<Object> arguments) {
                return ((double) System.currentTimeMillis()) / 1000.0;
            }

            @Override
            public String toString() {
                return "<native fn>";
            }
        });
    }

    public void interpret(List<Stmt> statements) {
        try {
            for (Stmt statement : statements) {
                execute(statement);
            }
        } catch (RuntimeError error) {
            Lox.runtimeError(error);
        }
    }

    /**
     * Store resolution information: - How many scopes there are between the
     * current scope and the scope where the variable is defined. At runtime,
     * this corresponds exactly to the number of environments between the
     * current one and the enclosing one where the interpreter can find the
     * variable’s value.
     *
     */
    public void resolve(Expr expr, int depth) {
        locals.put(expr, depth);
    }

    @Override
    public Void visitClassStmt(Stmt.Class stmt) {
        environment.define(stmt.getName().getLexeme(), null);

        Map<String, LoxFunction> methods = new HashMap<>();
        for (Stmt.Function method : stmt.getMethods()) {
            methods.put(method.getName().getLexeme(),
                    new LoxFunction(method, environment, method.getName().getLexeme().equals("init")));
        }

        LoxClass klass = new LoxClass(stmt.getName().getLexeme(), methods);
        environment.assign(stmt.getName(), klass);

        return null;
    }

    @Override
    public Void visitBreakStmt(Stmt.Break stmt) {
        throw new Interpreter.BreakContinuation();
    }

    @Override
    public Void visitContinueStmt(Stmt.Continue stmt) {
        throw new Interpreter.ContinueContinuation();
    }

    @Override
    public Void visitIfStmt(Stmt.If stmt) {
        if (isTruthy(evaluate(stmt.getCondition()))) {
            execute(stmt.getThenBranch());
        } else if (stmt.getElseBranch() != null) {
            execute(stmt.getElseBranch());
        }

        return null;
    }

    @Override
    public Void visitWhileStmt(Stmt.While stmt) {
        while (isTruthy(evaluate(stmt.getCondition()))) {
            try {
                execute(stmt.getBody());
            } catch (Interpreter.BreakContinuation e) {
                break;
            } catch (Interpreter.ContinueContinuation e) {
            }
        }

        return null;
    }

    @Override
    public Void visitReturnStmt(Stmt.Return stmt) {
        Object value = null;
        if (stmt.getValue() != null) {
            value = evaluate(stmt.getValue());
        }

        throw new Interpreter.ReturnContinuation(value);
    }

    @Override
    public Void visitVarStmt(Stmt.Var stmt) {
        Object value = null;
        if (stmt.getInitializer() != null) {
            value = evaluate(stmt.getInitializer());
        }

        environment.define(stmt.getName().getLexeme(), value);
        return null;
    }

    @Override
    public Object visitAssignExpr(Expr.Assign expr) {
        Object value = evaluate(expr.getValue());

        Integer distance = locals.get(expr);
        if (distance != null) {
            environment.assignAt(distance, expr.getName(), value);
        } else {
            globals.assign(expr.getName(), value);
        }

        return value;
    }

    @Override
    public Void visitExpressionStmt(Stmt.Expression stmt) {
        evaluate(stmt.getExpression());
        return null;
    }

    @Override
    public Void visitFunctionStmt(Stmt.Function stmt) {
        LoxFunction function = new LoxFunction(stmt, environment);
        environment.define(stmt.getName().getLexeme(), function);
        return null;
    }

    @Override
    public Void visitPrintStmt(Stmt.Print stmt) {
        Object value = evaluate(stmt.getExpression());
        System.out.print(stringify(value));
        return null;
    }

    @Override
    public Void visitBlockStmt(Stmt.Block stmt) {
        executeBlock(stmt.getStatements(), new Environment(environment));
        return null;
    }

    @Override
    public Object visitBinaryExpr(Expr.Binary expr) {
        Object left = evaluate(expr.getLeft());
        Object right = evaluate(expr.getRight());

        switch (expr.getOperator().getType()) {
            case TokenType.GREATER -> {
                checkNumberOperands(expr.getOperator(), left, right);

                return ((double) left) > ((double) right);
            }
            case TokenType.GREATER_EQUAL -> {
                checkNumberOperands(expr.getOperator(), left, right);

                return ((double) left) >= ((double) right);
            }
            case TokenType.LESS -> {
                checkNumberOperands(expr.getOperator(), left, right);

                return ((double) left) < ((double) right);
            }
            case TokenType.LESS_EQUAL -> {
                checkNumberOperands(expr.getOperator(), left, right);

                return ((double) left) <= ((double) right);
            }
            case TokenType.PLUS -> {
                if (left instanceof Double && right instanceof Double) {
                    return ((double) left) + ((double) right);
                }

                if (left instanceof String && right instanceof String) {
                    return ((String) left) + ((String) right);
                }

                throw new RuntimeError(expr.getOperator(), "Operands must be two numbers or two strings");
            }
            case TokenType.MINUS -> {
                checkNumberOperands(expr.getOperator(), left, right);

                return ((double) left) - ((double) right);
            }
            case TokenType.SLASH -> {
                checkNumberOperands(expr.getOperator(), left, right);

                if ((double) right == 0) {
                    throw new RuntimeError(expr.getOperator(), "Divide by zero.");
                }

                return ((double) left) / ((double) right);
            }
            case TokenType.STAR -> {
                checkNumberOperands(expr.getOperator(), left, right);

                return ((double) left) * ((double) right);
            }
            case TokenType.BANG_EQUAL -> {
                return !isEqual(left, right);
            }
            case TokenType.EQUAL_EQUAL -> {
                return isEqual(left, right);
            }
            case TokenType.COMMA -> {
                return right;
            }
        }

        return null;
    }

    @Override
    public Object visitCallExpr(Expr.Call expr) {
        Object callee = evaluate(expr.getCallee());

        List<Object> arguments = new ArrayList<>();
        for (Expr argument : expr.getArguments()) {
            arguments.add(evaluate(argument));
        }

        if (!(callee instanceof LoxCallable)) {
            throw new RuntimeError(expr.getParen(), "Can only call function and classes.");
        }

        LoxCallable function = (LoxCallable) callee;
        if (arguments.size() != function.arity()) {
            throw new RuntimeError(expr.getParen(),
                    "Expected " + function.arity() + " arguments but got " + arguments.size() + ".");
        }

        return function.call(this, arguments);
    }

    @Override
    public Object visitGetExpr(Expr.Get expr) {
        Object object = evaluate(expr.getObject());
        if (object instanceof LoxInstance) {
            return ((LoxInstance) object).get(expr.getName());
        }

        throw new RuntimeError(expr.getName(),
                "Only instances have properties.");
    }

    @Override
    public Object visitLogicalExpr(Expr.Logical expr) {
        Object left = evaluate(expr.getLeft());

        switch (expr.getOperator().getType()) {
            case TokenType.OR -> {
                if (isTruthy(left)) {
                    return left;
                }
            }
            case TokenType.AND -> {
                if (!isTruthy(left)) {
                    return left;
                }
            }
        }

        return evaluate(expr.getRight());
    }

    @Override
    public Object visitSetExpr(Expr.Set expr) {
        Object object = evaluate(expr.getObject());

        if (!(object instanceof LoxInstance)) {
            throw new RuntimeError(expr.getName(),
                    "Only instances have fields.");
        }

        Object value = evaluate(expr.getValue());
        ((LoxInstance) object).set(expr.getName(), value);
        return value;
    }

    @Override
    public Object visitThisExpr(Expr.This expr) {
        return lookUpVariable(expr.getKeyword(), expr);
    }

    @Override
    public Object visitGroupingExpr(Expr.Grouping expr) {
        return evaluate(expr.getExpression());
    }

    @Override
    public Object visitLiteralExpr(Expr.Literal expr) {
        return expr.getValue();
    }

    @Override
    public Object visitUnaryExpr(Expr.Unary expr) {
        Object right = evaluate(expr.getRight());

        switch (expr.getOperator().getType()) {
            case TokenType.MINUS -> {
                return -((double) right);
            }
            case TokenType.BANG -> {
                return !isTruthy(expr);
            }
        }

        return null;
    }

    @Override
    public Object visitConditionalExpr(Expr.Conditional expr) {
        return isTruthy(evaluate(expr.getCondExpr())) ? evaluate(expr.getThenExpr()) : evaluate(expr.getElseExpr());
    }

    @Override
    public Object visitVariableExpr(Expr.Variable expr) {
        return lookUpVariable(expr.getName(), expr);
    }

    private Object lookUpVariable(Token name, Expr expr) {
        Integer distance = locals.get(expr);
        if (distance != null) {
            return environment.getAt(distance, name.getLexeme());
        } else {
            return globals.get(name);
        }
    }

    /**
     * Execute a statement.
     */
    private void execute(Stmt stmt) {
        stmt.accept(this);
    }

    public void executeBlock(List<Stmt> statements, Environment environment) {
        Environment previous = this.environment;

        try {
            this.environment = environment;

            for (Stmt statement : statements) {
                execute(statement);
            }
        } finally {
            this.environment = previous;
        }
    }

    /**
     * Evaluate an expression.
     */
    private Object evaluate(Expr expr) {
        return expr.accept(this);
    }

    private boolean isTruthy(Object object) {
        if (object == null) {
            return false;
        }

        if (object instanceof Boolean) {
            return ((boolean) object);
        }

        return true;
    }

    private boolean isEqual(Object a, Object b) {
        if (a == null && b == null) {
            return true;
        }

        if (a == null) {
            return false;
        }

        return a.equals(b);
    }

    private void checkNumberOperand(Token operator, Object operand) {
        if (operand instanceof Double) {
            return;
        }

        throw new RuntimeError(operator, "Operand must be a number.");
    }

    private void checkNumberOperands(Token operator, Object left, Object right) {
        if (left instanceof Double && right instanceof Double) {
            return;
        }

        throw new RuntimeError(operator, "Operands must be numbers.");
    }

    private String stringify(Object object) {
        if (object == null) {
            return "nil";
        }

        if (object instanceof Double) {
            String text = object.toString();
            if (text.endsWith(".0")) {
                text = text.substring(0, text.length() - 2);
            }

            return text;
        }

        return object.toString();
    }

    private static class Continuation extends RuntimeException {

        public Continuation() {
            super(null, null, false, false);
        }
    }

    private static class BreakContinuation extends Continuation {
    }

    private static class ContinueContinuation extends Continuation {
    }

    public static class ReturnContinuation extends Continuation {

        @Getter
        private final Object value;

        public ReturnContinuation(Object value) {
            super();
            this.value = value;
        }
    }
}
