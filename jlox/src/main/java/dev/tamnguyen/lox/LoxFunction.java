package dev.tamnguyen.lox;

import java.util.List;

/**
 * Runtime representation of Lox function
 */
public class LoxFunction implements LoxCallable {

    private final Stmt.Function declaration;
    /**
     * The environment that is active when the function is declared, not when it's
     * called.
     */
    private final Environment closure;

    private final boolean isInitializer;

    public LoxFunction(Stmt.Function declaration, Environment closure) {
        this(declaration, closure, false);
    }

    public LoxFunction(Stmt.Function declaration, Environment closure, boolean isInitializer) {
        this.declaration = declaration;
        this.closure = closure;
        this.isInitializer = isInitializer;
    }

    @Override
    public int arity() {
        return declaration.getParams().size();
    }

    @Override
    public Object call(Interpreter interpreter, List<Object> arguments) {
        Environment environment = new Environment(closure);
        for (int i = 0; i < declaration.getParams().size(); i++) {
            environment.define(declaration.getParams().get(i).getLexeme(), arguments.get(i));
        }

        try {
            interpreter.executeBlock(declaration.getBody(), environment);
        } catch (Interpreter.ReturnContinuation returnValue) {
            if (isInitializer) {
                return closure.getAt(0, "this");
            }

            return returnValue.getValue();
        }

        if (isInitializer) {
            return closure.getAt(0, "this");
        }

        return null;
    }

    @Override
    public String toString() {
        return "<fn " + declaration.getName().getLexeme() + ">";
    }

    public LoxFunction bind(LoxInstance instance) {
        Environment environment = new Environment(closure);
        environment.define("this", instance);
        return new LoxFunction(declaration, environment, isInitializer);
    }
}
