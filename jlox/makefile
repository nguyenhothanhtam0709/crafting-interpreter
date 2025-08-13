dev:
	mvn compile exec:java

gen_ast:
	mvn compile exec:java \
		-Dexec.mainClass="dev.tamnguyen.tool.GenerateAst" \
		-Dexec.args="$(CURDIR)/src/main/java/dev/tamnguyen/lox"

test:
	mvn test

build:
	mvn clean compile

.PHONY: dev test build gen_ast