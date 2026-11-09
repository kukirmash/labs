#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>

class CodeEditor; // Предварительное объявление

//----------------------------------------------------------------------------------------------------------
class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    CodeEditor *codeEditor;
    QTableWidget *resultTable;

    QPushButton *loadButton;
    QPushButton *saveButton;
    QPushButton *scanButton;

public:
    MainWindow(QWidget *parent = nullptr);
    virtual ~MainWindow() override {}

private slots:
    void RunScanner();
    void LoadFile();
    void SaveFile();
};

//----------------------------------------------------------------------------------------------------------
#endif // MAINWINDOW_H