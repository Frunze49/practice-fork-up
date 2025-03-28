.PHONY: testsuite test-clean

testsuite:
	@echo "Running..."
	@docker compose up --build --abort-on-container-exit tests >/dev/null 2>&1 || (echo "Ошибка запуска тестов" && exit 1)
	@echo "Tests Result:"
	@docker compose logs -f tests
	@make test-clean

test-clean:
	@echo "Clearing..."
	@docker compose down -v >/dev/null 2>&1
