#ifndef FINDWINDOW_H
#define FINDWINDOW_H

#include <QWidget>
#include <QTextEdit>

namespace Ui {
class FindWindow;
}

class FindWindow : public QWidget
{
    Q_OBJECT

public:
    explicit FindWindow(QTextEdit* textEdit, QWidget *parent = nullptr);
    ~FindWindow();

private slots:
    void on_findText_textChanged(const QString &text);

    void on_replaceButton_clicked();

    void on_findNextButton_clicked();

    void on_replaceAllButton_clicked();

private:
    Ui::FindWindow *ui;
    QTextEdit* textEdit;
    QList<QTextEdit::ExtraSelection> selections;
    int curIndex;

    static constexpr int INVALID_INDEX = -1;

    bool isDirectionDown = true;

    bool replacing = false;

    void updateSelections();

    int findNextSelectionIndex();

    void replaceAll();

    void setButtonsEnabled(bool enabled);

    QTextDocument::FindFlags getFindFlags();

    void updateMatchesCountLabel();
};

#endif // FINDWINDOW_H
