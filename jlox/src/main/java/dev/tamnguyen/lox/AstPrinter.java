package dev.tamnguyen.lox;

import dev.tamnguyen.lox.Expr.Get;
import dev.tamnguyen.lox.Expr.Logical;
import dev.tamnguyen.lox.Expr.Set;
import dev.tamnguyen.lox.Expr.This;

public class AstPrinter implements Expr.Visitor<String> {

    public String print(Expr expr) {
        return expr.accept(this);
    }

    @Override
    public String visitBinaryExpr(Expr.Binary expr) {
        return parenthesize(expr.getOperator().getLexeme(), expr.getLeft(), expr.getRight());
    }

    @Override
    public String visitGroupingExpr(Expr.Grouping expr) {
        return parenthesize("group", expr.getExpression());
    }

    @Override
    public String visitLiteralExpr(Expr.Literal expr) {
        if (expr.getValue() == null) {
            return "nil";
        }
        return expr.getValue().toString();
    }

    @Override
    public String visitUnaryExpr(Expr.Unary expr) {
        return parenthesize(expr.getOperator().getLexeme(), expr.getRight());
    }

    @Override
    public String visitConditionalExpr(Expr.Conditional expr) {
        return parenthesize("?:", expr.getCondExpr(), expr.getThenExpr(), expr.getElseExpr());
    }

    @Override
    public String visitLogicalExpr(Logical expr) {
        return parenthesize(expr.getOperator().getLexeme(), expr.getLeft(), expr.getRight());
    }

    @Override
    public String visitVariableExpr(Expr.Variable expr) {
        return expr.getName().getLexeme();
    }

    @Override
    public String visitAssignExpr(Expr.Assign expr) {
        return parenthesize("=" + expr.getName().getLexeme(), expr.getValue());
    }

    @Override
    public String visitCallExpr(Expr.Call expr) {
        return "";
    }

    private String parenthesize(String name, Expr... exprs) {
        StringBuilder builder = new StringBuilder();

        builder.append("(").append(name);
        for (Expr expr : exprs) {
            builder.append(" ");
            builder.append(expr.accept(this));
        }
        builder.append(")");

        return builder.toString();
    }

    @Override
    public String visitGetExpr(Get expr) {
        // TODO Auto-generated method stub
        throw new UnsupportedOperationException("Unimplemented method 'visitGetExpr'");
    }

    @Override
    public String visitSetExpr(Set expr) {
        // TODO Auto-generated method stub
        throw new UnsupportedOperationException("Unimplemented method 'visitSetExpr'");
    }

    @Override
    public String visitThisExpr(This expr) {
        // TODO Auto-generated method stub
        throw new UnsupportedOperationException("Unimplemented method 'visitThisExpr'");
    }

    @Override
    public String visitSuperExpr(Expr.Super expr) {
        throw new UnsupportedOperationException("Not supported yet.");
    }
}
