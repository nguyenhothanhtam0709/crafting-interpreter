package dev.tamnguyen.lox;

import lombok.Getter;

public class RuntimeError extends RuntimeException {

    @Getter
    protected final Token token;

    public RuntimeError(Token token, String message) {
        super(message);
        this.token = token;
    }
}
