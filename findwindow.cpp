#include "findwindow.h"
#include "ui_findwindow.h"

FindWindow::FindWindow(QTextEdit* textEdit, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FindWindow)
{
    ui->setupUi(this);

    Qt::WindowFlags flags = windowFlags();

    flags &= ~Qt::WindowMaximizeButtonHint;
    flags &= ~Qt::WindowMinimizeButtonHint;
    flags |= Qt::WindowStaysOnTopHint | Qt::Tool;

    setWindowFlags(flags);

    this->textEdit = textEdit;

    connect(textEdit, &QTextEdit::textChanged, this, [this]() {
        if (!replacing)
            this->on_findText_textChanged(ui->findText->text());
    });

    setButtonsEnabled(false);

    updateMatchesCountLabel();
}

FindWindow::~FindWindow()
{
    delete ui;

    textEdit->setExtraSelections({});
}

void FindWindow::on_findText_textChanged(const QString &text) {
    curIndex = INVALID_INDEX;

    updateSelections();

    setButtonsEnabled(!selections.empty());
    if (!selections.empty()) {
        ui->replaceButton->setEnabled(false);
    }

    updateMatchesCountLabel();
}

void FindWindow::updateSelections() {
    QList<QTextEdit::ExtraSelection> newSelections;

    const QString text = ui->findText->text();

    if (text.isEmpty()) {
        selections.clear();
        return;
    }

    QTextCursor cursor = textEdit->textCursor();

    constexpr QColor foregroundColor = QColorConstants::Yellow;
    constexpr QColor backgroundColor = QColorConstants::Black;

    QTextEdit::ExtraSelection selection;
    selection.format.setBackground(foregroundColor);
    selection.format.setForeground(backgroundColor);

    cursor.movePosition(QTextCursor::Start);

    while(true) {
        cursor = textEdit->document()->find(text, cursor, getFindFlags());

        if (cursor.isNull()) {
            break;
        }

        selection.cursor = cursor;
        newSelections.append(selection);
    }

    selections = std::move(newSelections);

    textEdit->setExtraSelections(selections);
}

void FindWindow::on_findNextButton_clicked() {
    curIndex = findNextSelectionIndex();

    textEdit->setTextCursor(selections[curIndex].cursor);

    ui->replaceButton->setEnabled(true);

    updateMatchesCountLabel();
}

void FindWindow::on_replaceButton_clicked() {
    int index = curIndex;

    replacing = true;

    selections[curIndex].cursor.insertText(ui->replaceText->text());

    replacing = false;

    updateSelections();

    if (selections.empty()) {
        setButtonsEnabled(false);
        curIndex = INVALID_INDEX;
        return;
    }

    if (index >= selections.size()) {
        index = 0;
    }

    curIndex = index;

    textEdit->setTextCursor(selections[curIndex].cursor);

    updateMatchesCountLabel();
}

void FindWindow::on_replaceAllButton_clicked()
{
    replacing = true;
    for (auto& selection : selections) {
        selection.cursor.insertText(ui->replaceText->text());
    }

    selections.clear();
    curIndex = INVALID_INDEX;
    setButtonsEnabled(false);

    updateMatchesCountLabel();
}

int FindWindow::findNextSelectionIndex() {
    if (selections.empty())
        return INVALID_INDEX;

    const int delta = ui->directionDownButton->isChecked() ? 1 : -1;
    const int next = curIndex + delta;

    if (next >= 0 && next < selections.size())
        return next;

    if (ui->wrapAroundBox->isChecked())
        return (next + selections.size()) % selections.size();

    return curIndex;
}

void FindWindow::setButtonsEnabled(bool enabled) {
    ui->findNextButton->setEnabled(enabled);
    ui->replaceButton->setEnabled(enabled);
    ui->replaceAllButton->setEnabled(enabled);
}

QTextDocument::FindFlags FindWindow::getFindFlags() {
    QTextDocument::FindFlags flags{};
    if (ui->matchCaseBox->isChecked())
        flags | QTextDocument::FindFlag::FindCaseSensitively;

    return flags;
}

void FindWindow::updateMatchesCountLabel() {
    QString text{};
    if (!selections.empty()) {
        text = tr("%1 of %2 matches")
                   .arg(QString::number(curIndex + 1),
                        QString::number(selections.size()));
    } else {
        text = tr("0 matches");
    }

    ui->matchesCountLabel->setText(text);
}