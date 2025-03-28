.PHONY: test test-clean test-debug setup-docker

# Запуск тестов с минимальным выводом
test:
	@echo "Запуск тестов..."
	@docker-compose up --build --abort-on-container-exit tests > /dev/null 2>&1 || (echo "Ошибка запуска тестов" && exit 1)
	@echo "Результаты тестов:"
	@docker logs $$(docker ps -aq --filter name=fork-blob_tests_run_1 --filter status=exited) 2>&1 | grep -E "PASS|FAIL|RUNNING|test cases|Total Test time"
	@make test-clean

# Вывод подробных логов для отладки
test-debug:
	@echo "Запуск тестов в режиме отладки..."
	@docker-compose up --build --abort-on-container-exit
	@make test-clean

# Очистка контейнеров и томов
test-clean:
	@echo "Очистка Docker ресурсов..."
	@docker-compose down -v > /dev/null 2>&1

# Подготовка окружения Docker
setup-docker:
	@echo "Проверка настройки Docker..."
	@which docker > /dev/null 2>&1 || (echo "Docker не установлен" && exit 1)
	@which docker-compose > /dev/null 2>&1 || (echo "docker-compose не установлен" && exit 1)
	@echo "Docker настроен"

