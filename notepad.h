#ifndef NOTEPAD_H
#define NOTEPAD_H

#include <QMainWindow>
#include <QMessageBox>
#include <QLabel>

#include "findwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class Notepad;
}
QT_END_NAMESPACE

class Notepad : public QMainWindow
{
    Q_OBJECT

public:
    explicit Notepad(QWidget *parent = nullptr);
    ~Notepad() override;


private slots:
    void on_actionNew_triggered();

    void on_actionOpen_triggered();

    void on_actionSelect_Font_triggered();

    void on_actionZoom_In_triggered();

    void on_actionZoom_Out_triggered();

    void on_actionReset_Zoom_triggered();

    void on_actionStatus_Bar_triggered();

    void on_actionPrint_file_triggered();

    void on_actionFindReplace_triggered();

    void on_actionSave_and_Quit_triggered();

    void on_actionDiscard_changes_triggered();

private:
    Ui::Notepad *ui;
    QString currentFile;
    QString defaultTitle = QString("New File");

    FindWindow* findWindow{};

    QLabel* fontSizeLabel{};
    QLabel* charCountLabel{};
    QLabel* textCursorPosLabel{};

    bool isDirtyTitle = false;

    // Document
    bool save();
    bool saveAs();
    bool saveOutputToFile(const QString& filename);

    void closeEvent(QCloseEvent *event) override;

    void textChanged();
    void textSelected();
    void updateCharCount();
    void resetFontLabel();
    void textCursorPosChanged();
    void updateWindowTitle();

    QMessageBox::StandardButton askSave();
};
#endif // NOTEPAD_H
