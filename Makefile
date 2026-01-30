# Makefile для Chemical Lab System
BUILD_DIR = build
SOURCE_DIR = .

all: init_db build_qt

init_db:
	mkdir -p data
	sqlite3 data/laboratory.db < sql/schema.sql
	sqlite3 data/laboratory.db < sql/init_data.sql
	echo "✓ База данных инициализирована!"

configure:
	mkdir -p $(BUILD_DIR)
	cd $(BUILD_DIR) && cmake ..

build_qt: configure
	cd $(BUILD_DIR) && make ChemicalLabQt -j4

run_qt: build_qt
	./$(BUILD_DIR)/ChemicalLabQt

# Быстрая консольная версия
quick_console:
	g++ -std=c++17 -o chem_console ../Core_lab.cpp -lsqlite3 -lncurses -lpthread
	cp data/laboratory.db ./ || true
	./chem_console

clean:
	rm -rf $(BUILD_DIR)
	rm -f chem_console ChemicalLabQt

install_deps:
	sudo apt update
	sudo apt install -y qt6-base-dev qt6-tools-dev libsqlite3-dev cmake make g++ libncurses-dev

help:
	@echo "Команды:"
	@echo "  make              - Инициализировать БД и собрать Qt версию"
	@echo "  make run_qt       - Собрать и запустить Qt версию"
	@echo "  make quick_console- Быстрая сборка консольной версии"
	@echo "  make clean        - Очистить сборку"
	@echo "  make install_deps - Установить зависимости"

.PHONY: all configure build_qt run_qt clean init_db help
