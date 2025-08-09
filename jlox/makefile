dev:
	mvn compile exec:java

gen_ast:
	mvn compile exec:java \
		-Dexec.mainClass="dev.tamnguyen.tool.GenerateAst" \
		-Dexec.args="$(CURDIR)/src/main/java/dev/tamnguyen/lox"

test:
	mvn test

.PHONY: dev test gen_ast