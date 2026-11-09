#include "CodeEditor.h"
#include <QPainter>
#include <QTextBlock>

#include "CodeEditor.h"
#include <QPainter>
#include <QTextBlock>

//----------------------------------------------------------------------------------------------------------
// Реализация подсветчика синтаксиса
Highlighter::Highlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;

    // Настраиваем формат для ключевых слов (синий цвет, жирный шрифт)
    keywordFormat.setForeground(Qt::blue);

    // Список ваших ключевых слов
    QStringList keywordPatterns;
    keywordPatterns << "\\bpackage8\\b" << "\\bvar8\\b" << "\\bdo8\\b"
                    << "\\bif8\\b" << "\\bexit8\\b" << "\\benddo8\\b"
                    << "\\btypedef8\\b" << "\\bstruct8\\b"
                    << "\\binteger\\b" << "\\bfloat\\b" << "\\blogical\\b";

    // Превращаем каждое слово в регулярное выражение
    for (const QString &pattern : keywordPatterns)
    {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }
}

void Highlighter::highlightBlock(const QString &text)
{
    // Проходим по всем правилам (словам) и применяем формат, если нашли совпадение
    for (const HighlightingRule &rule : std::as_const(highlightingRules))
    {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext())
        {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}

//----------------------------------------------------------------------------------------------------------
CodeEditor::CodeEditor(QWidget *parent) : QPlainTextEdit(parent)
{
    lineNumberArea = new LineNumberArea(this);

    // ПОДКЛЮЧАЕМ ПОДСВЕТКУ СИНТАКСИСА К ДОКУМЕНТУ
    highlighter = new Highlighter(this->document());

    connect(this, &CodeEditor::blockCountChanged, this, &CodeEditor::UpdateLineNumberAreaWidth);
    connect(this, &CodeEditor::updateRequest, this, &CodeEditor::UpdateLineNumberArea);
    connect(this, &CodeEditor::cursorPositionChanged, this, &CodeEditor::HighlightCurrentLine);

    UpdateLineNumberAreaWidth(0);
    HighlightCurrentLine();
}

//----------------------------------------------------------------------------------------------------------
int CodeEditor::LineNumberAreaWidth()
{
    int digits = 1;
    int max = qMax(1, blockCount());
    while (max >= 10)
    {
        max /= 10;
        ++digits;
    }
    int space = 3 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
    return space;
}

//----------------------------------------------------------------------------------------------------------
void CodeEditor::UpdateLineNumberAreaWidth(int /* newBlockCount */)
{
    setViewportMargins(LineNumberAreaWidth(), 0, 0, 0);
}

//----------------------------------------------------------------------------------------------------------
void CodeEditor::UpdateLineNumberArea(const QRect &rect, int dy)
{
    if (dy)
        lineNumberArea->scroll(0, dy);
    else
        lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

    if (rect.contains(viewport()->rect()))
        UpdateLineNumberAreaWidth(0);
}

//----------------------------------------------------------------------------------------------------------
void CodeEditor::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);
    QRect cr = contentsRect();
    lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), LineNumberAreaWidth(), cr.height()));
}

//----------------------------------------------------------------------------------------------------------
void CodeEditor::HighlightCurrentLine()
{
    QList<QTextEdit::ExtraSelection> extraSelections;
    if (!isReadOnly())
    {
        QTextEdit::ExtraSelection selection;
        QColor lineColor = QColor(Qt::yellow).lighter(160);
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        extraSelections.append(selection);
    }
    setExtraSelections(extraSelections);
}

//----------------------------------------------------------------------------------------------------------
void CodeEditor::LineNumberAreaPaintEvent(QPaintEvent *event)
{
    QPainter painter(lineNumberArea);
    painter.fillRect(event->rect(), Qt::lightGray);

    QTextBlock block = firstVisibleBlock();
    int blockNumber = block.blockNumber();
    int top = qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + qRound(blockBoundingRect(block).height());

    while (block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width() - 2, fontMetrics().height(),
                             Qt::AlignRight | Qt::AlignVCenter, number);
        }

        block = block.next();
        top = bottom;
        bottom = top + qRound(blockBoundingRect(block).height());
        ++blockNumber;
    }
}

//----------------------------------------------------------------------------------------------------------
QSize LineNumberArea::sizeHint() const
{
    return QSize(codeEditor->LineNumberAreaWidth(), 0);
}

//----------------------------------------------------------------------------------------------------------
void LineNumberArea::paintEvent(QPaintEvent *event)
{
    codeEditor->LineNumberAreaPaintEvent(event);
}

//----------------------------------------------------------------------------------------------------------