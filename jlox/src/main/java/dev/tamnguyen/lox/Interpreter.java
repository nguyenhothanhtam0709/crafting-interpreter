package dev.tamnguyen.lox;

import java.util.List;

/**
 * A tree-walking interpreter
 */
public class Interpreter implements Expr.Visitor<Object>, Stmt.Visitor<Void> {
    private Environment environment = new Environment();

    public void interpret(List<Stmt> statements) {
        try {
            for (Stmt statement : statements) {
                execute(statement);
            }
        } catch (RuntimeError error) {
            Lox.runtimeError(error);
        }
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
        environment.assign(expr.getName(), value);
        return value;
    }

    @Override
    public Void visitExpressionStmt(Stmt.Expression stmt) {
        evaluate(stmt.getExpression());
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
    public Object visitLogicalExpr(Expr.Logical expr) {
        Object left = evaluate(expr.getLeft());
        Object right = evaluate(expr.getRight());

        switch (expr.getOperator().getType()) {
            case TokenType.OR -> {
                return isTruthy(left) || isTruthy(right);
            }
            case TokenType.AND -> {
                return isTruthy(left) && isTruthy(right);
            }
        }

        return null;
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
        return environment.get(expr.getName());
    }

    /**
     * Execute a statement.
     */
    private void execute(Stmt stmt) {
        stmt.accept(this);
    }

    private void executeBlock(List<Stmt> statements, Environment environment) {
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
}
