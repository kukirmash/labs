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
#include <QDialog>
#include <QTabWidget>
#include <QPlainTextEdit>
#include <QDir>
#include <QCoreApplication>
#include <QAbstractItemView>

#include "../scanner/Scanner.h"
#include "../sut/SutParser.h"

//----------------------------------------------------------------------------------------------------------
namespace
{
QString CatName(CatId cat)
{
    switch (cat)
    {
    case catProgName: return "программа";
    case catTypeName: return "имя типа";
    case catVarName: return "переменная";
    case catConst: return "константа";
    default: return "—";
    }
}

QString TypeName(TypeCode code)
{
    switch (code)
    {
    case typeInt: return "integer";
    case typeFloat: return "float";
    case typeLogical: return "logical";
    case typeRecord: return "record";
    default: return "void";
    }
}

QString SymText(const SutResult &r, int idx)
{
    if (idx < 0)
        return "—";
    if (idx < (int)r.symbols.size())
        return QString::fromStdString(r.symbols[idx].GetLex());
    return QString("?%1").arg(idx);
}
} // namespace

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
    sutButton = new QPushButton("СУТ", this);
    topRightLayout->addWidget(scanButton);
    topRightLayout->addWidget(sutButton);
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
    setWindowTitle("Лексический и синтаксический анализатор");

    connect(loadButton, &QPushButton::clicked, this, &MainWindow::LoadFile);
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::SaveFile);
    connect(scanButton, &QPushButton::clicked, this, &MainWindow::RunScanner);
    connect(sutButton, &QPushButton::clicked, this, &MainWindow::RunSut);
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
//----------------------------------------------------------------------------------------------------------
void MainWindow::RunSut()
{
    QString qText = codeEditor->toPlainText();
    if (qText.isEmpty())
        return;

    // Путь по умолчанию: результаты генератора в каталоге проекта
    QString defaultPath = QDir::currentPath() + "/Результаты генератора таблицы разбора/LR_Table.xml";
    if (!QFile::exists(defaultPath))
        defaultPath = QCoreApplication::applicationDirPath() + "/LR_Table.xml";

    QString tableFile = QFileDialog::getOpenFileName(this, "Таблица разбора", defaultPath,
                                                     "XML (*.xml);;Все файлы (*)");
    if (tableFile.isEmpty())
        return;

    SutParser parser;
    std::string loadErr;
    if (!parser.LoadTable(tableFile.toStdString(), loadErr))
    {
        QMessageBox::critical(this, "СУТ", QString("Не удалось загрузить таблицу разбора:\n%1")
                                                .arg(QString::fromStdString(loadErr)));
        return;
    }

    SutResult res;
    parser.Parse(qText.toStdString(), res);

    QDialog dlg(this);
    dlg.setWindowTitle("СУТ — результаты");
    dlg.resize(950, 620);
    QVBoxLayout *dlgLayout = new QVBoxLayout(&dlg);
    QTabWidget *tabs = new QTabWidget(&dlg);
    tabs->setStyleSheet("QTableWidget { border: 2px solid #555555; border-radius: 4px; }");
    dlgLayout->addWidget(tabs);

    // ---- Трехадресный код ----
    QTableWidget *quads = new QTableWidget(&dlg);
    quads->setColumnCount(5);
    quads->setHorizontalHeaderLabels({"№", "Операция", "Аргумент 1", "Аргумент 2", "Результат"});
    quads->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    quads->setEditTriggers(QAbstractItemView::NoEditTriggers);
    quads->setRowCount((int)res.quads.size());
    for (int i = 0; i < (int)res.quads.size(); ++i)
    {
        const Quad &q = res.quads[i];
        bool jump = (q.op == opIfGoto || q.op == opGoto);
        quads->setItem(i, 0, new QTableWidgetItem(QString::number(i)));
        quads->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(Tac::OpName(q.op))));
        quads->setItem(i, 2, new QTableWidgetItem(SymText(res, q.a1)));
        quads->setItem(i, 3, new QTableWidgetItem(jump ? QString("—") : SymText(res, q.a2)));
        quads->setItem(i, 4, new QTableWidgetItem(jump ? QString::number(q.res) : SymText(res, q.res)));
    }
    tabs->addTab(quads, "Трехадресный код");

    // ---- Таблица символов ----
    int fieldCount = 0;
    for (const auto &t : res.fieldTables)
        fieldCount += (int)t.size();

    QTableWidget *syms = new QTableWidget(&dlg);
    syms->setColumnCount(5);
    syms->setHorizontalHeaderLabels({"№", "Лексема", "Категория", "Тип", "Адрес"});
    syms->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    syms->setEditTriggers(QAbstractItemView::NoEditTriggers);
    syms->setRowCount((int)res.symbols.size() + fieldCount);

    int row = 0;
    for (int i = 0; i < (int)res.symbols.size(); ++i)
    {
        const TableId_Elem &e = res.symbols[i];
        syms->setItem(row, 0, new QTableWidgetItem(QString::number(i)));
        syms->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(e.GetLex())));
        syms->setItem(row, 2, new QTableWidgetItem(CatName(e.GetCat())));
        syms->setItem(row, 3, new QTableWidgetItem(TypeName(e.GetTypeCode())));
        syms->setItem(row, 4, new QTableWidgetItem(e.GetAddr() >= 0 ? QString::number(e.GetAddr()) : QString("—")));
        ++row;
    }
    for (int t = 0; t < (int)res.fieldTables.size(); ++t)
    {
        for (const TableId_Elem &e : res.fieldTables[t])
        {
            syms->setItem(row, 0, new QTableWidgetItem(QString("поле %1").arg(t)));
            syms->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(e.GetLex())));
            syms->setItem(row, 2, new QTableWidgetItem("поле записи"));
            syms->setItem(row, 3, new QTableWidgetItem(TypeName(e.GetTypeCode())));
            syms->setItem(row, 4, new QTableWidgetItem(QString::number(e.GetAddr())));
            ++row;
        }
    }
    tabs->addTab(syms, "Таблица символов");

    // ---- Ошибки ----
    QPlainTextEdit *errView = new QPlainTextEdit(&dlg);
    errView->setReadOnly(true);
    if (res.errors.empty())
    {
        errView->setPlainText("Ошибок нет.\nТрансляция завершена успешно.");
    }
    else
    {
        QString text;
        for (const std::string &e : res.errors)
            text += QString::fromStdString(e) + "\n";
        errView->setPlainText(text);
    }
    tabs->addTab(errView, "Ошибки");

    dlg.exec();
}

//----------------------------------------------------------------------------------------------------------
