.PHONY: testsuite test-clean

build-testsuite:
	@echo "Building test containers..."
	@docker compose build tests

testsuite:
	@echo "Running..."
	@docker compose up --abort-on-container-exit tests 2>&1 || (echo "Some error" && exit 1)
	@echo "Tests Result:"
	@docker compose logs -f tests
	@make test-clean

test-clean:
	@echo "Clearing..."
	@docker compose down -v 2>&1


