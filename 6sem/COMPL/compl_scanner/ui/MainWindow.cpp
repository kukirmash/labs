#include "MainWindow.h"
#include "CodeEditor.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QSplitter>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QApplication>
#include <QFont>

#include "../scanner/Scanner.h"

//----------------------------------------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    QWidget *centralWidget = new QWidget(this);

    QWidget *leftWidget = new QWidget(this);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *topLeftLayout = new QHBoxLayout();
    loadButton = new QPushButton("Загрузить", this);
    saveButton = new QPushButton("Сохранить", this);
    topLeftLayout->addWidget(loadButton);
    topLeftLayout->addWidget(saveButton);
    topLeftLayout->addStretch();

    codeEditor = new CodeEditor(this);

    // 1) Устанавливаем шрифт ТОЛЬКО для редактора кода
    QFont codeFont("Cascadia Code", 12);
    codeFont.setStyleHint(QFont::Monospace); // Подсказка системе использовать моноширинный шрифт, если Cascadia нет
    codeEditor->setFont(codeFont);

    // 2) Красивый стиль для границ и скроллбаров (вертикального и горизонтального)
    QString modernStyle =
        "QPlainTextEdit, QTableWidget { border: 2px solid #555555; border-radius: 4px; }"

        /* Вертикальный скроллбар */
        "QScrollBar:vertical { border: none; background: #f0f0f0; width: 14px; margin: 0px; border-radius: 4px; }"
        "QScrollBar::handle:vertical { background: #c0c0c0; min-height: 20px; border-radius: 7px; margin: 2px; }"
        "QScrollBar::handle:vertical:hover { background: #a0a0a0; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }"

        /* Горизонтальный скроллбар */
        "QScrollBar:horizontal { border: none; background: #f0f0f0; height: 14px; margin: 0px; border-radius: 4px; }"
        "QScrollBar::handle:horizontal { background: #c0c0c0; min-width: 20px; border-radius: 7px; margin: 2px; }"
        "QScrollBar::handle:horizontal:hover { background: #a0a0a0; }"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0px; }"
        "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal { background: none; }";

    codeEditor->setStyleSheet(modernStyle);

    leftLayout->addLayout(topLeftLayout);
    leftLayout->addWidget(codeEditor);

    QWidget *rightWidget = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightWidget);
    rightLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *topRightLayout = new QHBoxLayout();
    scanButton = new QPushButton("Сканер", this);
    topRightLayout->addWidget(scanButton);
    topRightLayout->addStretch();

    resultTable = new QTableWidget(this);
    // Применяем тот же красивый стиль и к таблице результатов
    resultTable->setStyleSheet(modernStyle);

    resultTable->setColumnCount(3);
    resultTable->setHorizontalHeaderLabels({"Токен", "Аттрибут", "Лексема"});
    resultTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    rightLayout->addLayout(topRightLayout);
    rightLayout->addWidget(resultTable);

    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(leftWidget);
    splitter->addWidget(rightWidget);
    splitter->setSizes({550, 250});

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->addWidget(splitter);

    setCentralWidget(centralWidget);
    resize(900, 600);
    setWindowTitle("Лексический анализатор");

    connect(loadButton, &QPushButton::clicked, this, &MainWindow::LoadFile);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::SaveFile);
    connect(scanButton, &QPushButton::clicked, this, &MainWindow::RunScanner);
}

//----------------------------------------------------------------------------------------------------------
void MainWindow::LoadFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Открыть файл", "", "Текстовые файлы (*.txt);;Все файлы (*)");
    if (fileName.isEmpty())
    {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл для чтения");
        return;
    }

    QTextStream in(&file);
    codeEditor->setPlainText(in.readAll());
    file.close();
}

//----------------------------------------------------------------------------------------------------------
void MainWindow::SaveFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить файл", "", "Текстовые файлы (*.txt);;Все файлы (*)");
    if (fileName.isEmpty())
    {
        return;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл для записи");
        return;
    }

    QTextStream out(&file);
    out << codeEditor->toPlainText();
    file.close();
}

//----------------------------------------------------------------------------------------------------------
void MainWindow::RunScanner()
{
    // Очищаем таблицу перед новым запуском
    resultTable->setRowCount(0);

    // 1. Получаем текст из редактора
    QString qText = codeEditor->toPlainText();
    if (qText.isEmpty())
    {
        return; // Если кода нет, ничего не делаем
    }

    // Безопасное получение C-строки
    QByteArray textData = qText.toUtf8();
    const char *code = textData.constData();

    // 2. Создаем сканер и получаем список токенов
    Scanner scanner;
    std::vector<Token> tokens = scanner.RunScanner(code);

    // 3. Заполняем таблицу результатов
    resultTable->setRowCount(tokens.size()); // Создаем нужное количество строк

    for (size_t i = 0; i < tokens.size(); ++i)
    {
        // Конвертируем данные токена в строки для Qt
        QString tokenName = QString::fromStdString(tokens[i].GetTokenName_ByCode(tokens[i].GetCode()));
        QString attrValue = QString::number(tokens[i].GetAttr());
        QString lexeme = QString::fromStdString(tokens[i].GetLexeme());

        // Заполняем ячейки ("Токен", "Аттрибут", "Лексема")
        resultTable->setItem(i, 0, new QTableWidgetItem(tokenName));
        resultTable->setItem(i, 1, new QTableWidgetItem(attrValue));
        resultTable->setItem(i, 2, new QTableWidgetItem(lexeme));
    }
}

//----------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------------------------