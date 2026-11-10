# Сборка проекта без CMake.
#
# Fedora (Qt 6 из репозитория):
#   sudo dnf install qt6-qtbase-devel
#   make -j$(nproc)
#   ./build/scanner
#
# Если Qt 6 установлен в нестандартный каталог:
#   make QT_PREFIX=/путь/к/Qt/6.x/gcc_64
#
# Переменные можно переопределять: CXX, CXXFLAGS, BUILD, TARGET, MOC.

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -fPIC
BUILD    ?= build
TARGET   ?= $(BUILD)/scanner

QT_PREFIX  ?= /usr
QT_INC     ?= $(QT_PREFIX)/include/qt6
QT_LIB     ?= $(QT_PREFIX)/lib64
QT_LIBEXEC ?= $(QT_LIB)/qt6/libexec
MOC        ?= $(QT_LIBEXEC)/moc

INCLUDES := -Iui -Iscanner -Isut -Ithird_party/pugixml \
            -I$(QT_INC) -I$(QT_INC)/QtCore -I$(QT_INC)/QtGui -I$(QT_INC)/QtWidgets
LIBS     := -L$(QT_LIB) -Wl,-rpath,$(QT_LIB) -lQt6Widgets -lQt6Gui -lQt6Core

SOURCES := \
    main.cpp \
    ui/MainWindow.cpp \
    ui/CodeEditor.cpp \
    scanner/Scanner.cpp \
    scanner/Token.cpp \
    sut/LRTable.cpp \
    sut/SymbolTable.cpp \
    sut/Tac.cpp \
    sut/SemanticActions.cpp \
    sut/SutParser.cpp \
    third_party/pugixml/pugixml.cpp

MOC_HEADERS := ui/MainWindow.h ui/CodeEditor.h
MOC_SOURCES := $(patsubst ui/%.h,$(BUILD)/moc/moc_%.cpp,$(MOC_HEADERS))

OBJECTS     := $(patsubst %.cpp,$(BUILD)/obj/%.o,$(SOURCES))
MOC_OBJECTS := $(patsubst $(BUILD)/moc/%.cpp,$(BUILD)/obj/moc/%.o,$(MOC_SOURCES))
ALL_OBJECTS := $(OBJECTS) $(MOC_OBJECTS)

.PHONY: all check-qt run clean

all: check-qt $(TARGET)

check-qt:
	@if [ ! -x "$(MOC)" ]; then \
		echo "Ошибка: не найден moc: $(MOC)"; \
		echo "Fedora: sudo dnf install qt6-qtbase-devel"; \
		echo "Другой Qt: make QT_PREFIX=/путь/к/Qt/6.x/gcc_64"; \
		exit 1; \
	fi

$(TARGET): $(ALL_OBJECTS)
	$(CXX) $(ALL_OBJECTS) -o $@ $(LIBS)

$(BUILD)/obj/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

$(BUILD)/moc/moc_%.cpp: ui/%.h | $(BUILD)/moc
	$(MOC) $(INCLUDES) $< -o $@

$(BUILD)/obj/moc/%.o: $(BUILD)/moc/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

$(BUILD)/moc:
	mkdir -p $@

run: all
	./$(TARGET)

clean:
	rm -rf $(BUILD)

-include $(ALL_OBJECTS:.o=.d)
