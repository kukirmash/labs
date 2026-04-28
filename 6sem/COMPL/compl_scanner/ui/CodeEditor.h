#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>
#include <QWidget>

//----------------------------------------------------------------------------------------------------------
class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

private:
    QWidget *lineNumberArea;

public:
    CodeEditor(QWidget *parent = nullptr);
    virtual ~CodeEditor() override {}

    void LineNumberAreaPaintEvent(QPaintEvent *event);
    int LineNumberAreaWidth();

protected:
    virtual void resizeEvent(QResizeEvent *event) override;

private slots:
    void UpdateLineNumberAreaWidth(int newBlockCount);
    void HighlightCurrentLine();
    void UpdateLineNumberArea(const QRect &rect, int dy);
};

//----------------------------------------------------------------------------------------------------------
class LineNumberArea : public QWidget
{
private:
    CodeEditor *codeEditor;

public:
    LineNumberArea(CodeEditor *editor) : QWidget(editor), codeEditor(editor) {}
    virtual ~LineNumberArea() override {}

    virtual QSize sizeHint() const override;

protected:
    virtual void paintEvent(QPaintEvent *event) override;
};

//----------------------------------------------------------------------------------------------------------
#endif // CODEEDITOR_H