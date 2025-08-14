package dev.tamnguyen.lox;

import java.util.List;
import java.util.Map;

import lombok.Getter;

/**
 * Runtime representation of Lox class
 */
public class LoxClass implements LoxCallable {

    @Getter
    private final String name;
    @Getter
    private final LoxClass superclass;
    @Getter
    private final Map<String, LoxFunction> methods;

    public LoxClass(String name, LoxClass superclass, Map<String, LoxFunction> methods) {
        this.name = name;
        this.superclass = superclass;
        this.methods = methods;
    }

    @Override
    public String toString() {
        return name;
    }

    @Override
    public int arity() {
        LoxFunction initializer = findMethod("init");
        if (initializer != null) {
            return initializer.arity();
        }

        return 0;
    }

    /**
     * Lox class constructor
     */
    @Override
    public Object call(Interpreter interpreter, List<Object> arguments) {
        LoxInstance instance = new LoxInstance(this);

        // custom constructor method
        LoxFunction initializer = findMethod("init");
        if (initializer != null) {
            initializer.bind(instance).call(interpreter, arguments);
        }

        return instance;
    }

    public LoxFunction findMethod(String name) {
        if (methods.containsKey(name)) {
            return methods.get(name);
        }

        if (superclass != null) {
            return superclass.findMethod(name);
        }

        return null;
    }
}
