package dev.tamnguyen.tool;

import java.io.IOException;
import java.io.PrintWriter;
import java.time.Instant;
import java.util.Arrays;
import java.util.List;

public class GenerateAst {

    public static void main(String[] args) throws IOException {
        if (args.length != 1) {
            System.err.println("Usage: generate_ast <output directory>");
            System.exit(64);
        }

        String outputDir = args[0];
        defineAst(outputDir, "Expr", Arrays.asList(
                "Assign         : Token name, Expr value",
                "Binary         : Expr left, Token operator, Expr right",
                "Grouping       : Expr expression",
                "Call           : Expr callee, Token paren, List<Expr> arguments",
                "Literal        : Object value",
                "Logical        : Expr left, Token operator, Expr right",
                "Unary          : Token operator, Expr right",
                "Conditional    : Expr condExpr, Expr thenExpr, Expr elseExpr",
                "Variable       : Token name"));
        defineAst(outputDir, "Stmt", Arrays.asList(
                "Block          : List<Stmt> statements",
                "Expression     : Expr expression",
                "If             : Expr condition, Stmt thenBranch, Stmt elseBranch",
                "Print          : Expr expression",
                "Var            : Token name, Expr initializer",
                "While          : Expr condition, Stmt body",
                "Break          : Token keyword",
                "Continue       : Token keyword",
                "Function       : Token name, List<Token> params, List<Stmt> body",
                "Return         : Token keyword, Expr value"));
    }

    private static void defineAst(String outputDir, String baseName, List<String> types) throws IOException {
        String path = outputDir + "/" + baseName + ".java";
        try (PrintWriter writer = new PrintWriter(path, "UTF-8")) {
            String generatedTime = Instant.now().toString();
            String generatedBy = GenerateAst.class.getName();

            writeHeaderComment(writer, generatedTime, generatedBy);

            writer.println("package dev.tamnguyen.lox;");
            writer.println();

            writer.println("import java.util.List;");
            writer.println("import javax.annotation.processing.Generated;");
            writer.println("import lombok.Getter;");
            writer.println("import lombok.AllArgsConstructor;");
            writer.println();

            writer.println("/**");
            writer.println(" * Abstract syntax tree node");
            writer.println(" *");
            writer.println(" */");

            writeBasicAnnotation(writer, generatedTime, generatedBy);
            writer.println("public abstract class " + baseName + " extends AstNode {");

            defineVisitor(writer, baseName, types);

            writer.println();

            // > Define subclasses
            for (String type : types) {
                String[] splitted = type.split(":");

                String className = splitted[0].trim();
                String fields = splitted.length > 1 ? splitted[1].trim() : "";
                defineType(writer, baseName, className, fields);
                writer.println();
            }
            // < Define subclasses

            // The base accept() method
            writer.println();
            writer.println("    public abstract <R> R accept(Visitor<R> visitor);");

            writer.println("}");
        }
    }

    private static void defineType(PrintWriter writer, String baseName, String className, String fieldList)
            throws IOException {
        writer.println("    @AllArgsConstructor");
        writer.println("    @Getter");
        writer.println("    public static class " + className + " extends " + baseName + " {");

        String[] fields = fieldList.split(", ");

        // fields
        for (String field : fields) {
            if (field.trim().length() > 0) {
                writer.println("        private final " + field + ";");
            }
        }

        // method
        writer.println();
        writer.println("        @Override");
        writer.println("        public <R> R accept(Visitor<R> visitor) {");
        writer.println("            return visitor.visit"
                + className + baseName + "(this);");
        writer.println("        }");

        writer.println("    }");
    }

    private static void defineVisitor(PrintWriter writer, String baseName, List<String> types) throws IOException {
        writer.println("    public interface Visitor<R> {");

        for (String type : types) {
            String typeName = type.split(":")[0].trim();
            writer.println("        R visit" + typeName + baseName + "("
                    + typeName + " " + baseName.toLowerCase() + ");");
        }

        writer.println("    }");
    }

    public static String capitalizeFirst(String str) {
        if (str == null || str.isEmpty()) {
            return str;
        }
        return str.substring(0, 1).toUpperCase() + str.substring(1);
    }

    private static void writeHeaderComment(PrintWriter writer, String generatedTime, String generatedBy) {
        writer.println("/*");
        writer.println(" * AUTO-GENERATED FILE. DO NOT MODIFY.");
        writer.println(" *");
        writer.println(" * This class was automatically generated by " + generatedBy);
        writer.println(" * Generated on " + generatedTime);
        writer.println("*/");
        writer.println();
    }

    private static void writeBasicAnnotation(PrintWriter writer, String generatedTime, String generatedBy) {
        writer.println("@Generated(");
        writer.println("    value=\"" + generatedBy + "\",");
        writer.println("    date=\"" + generatedTime + "\",");
        writer.println("    comments = \"Do not modify this class.\"");
        writer.println(")");
    }
}
