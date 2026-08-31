#include <QFileDialog>
#include <QCloseEvent>
#include <QFontDialog>
#include <QShortcut>
#include <QFrame>

#include "findwindow.h"

#include "notepad.h"
#include "./ui_notepad.h"

using Button = QMessageBox::StandardButton;

Notepad::Notepad(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::Notepad)
{
    ui->setupUi(this);

    fontSizeLabel = new QLabel(
                            tr("Size: %1 pt")
                                .arg(QString::number(ui->textEdit->fontInfo().pointSize())),
                            this);

    charCountLabel = new QLabel(
                            tr("%1 characters")
                                .arg(QString::number(ui->textEdit->toPlainText().size())),
                            this);

    textCursorPosLabel = new QLabel(
                                tr("Line %1 Col %2")
                                    .arg(QString::number(ui->textEdit->textCursor().blockNumber()),
                                         QString::number(ui->textEdit->textCursor().columnNumber())
                                    ),
                                this);

    QFrame* separator = new QFrame(this);
    separator->setFrameShape(QFrame::VLine);

    ui->statusbar->addWidget(textCursorPosLabel);
    ui->statusbar->addWidget(separator);
    ui->statusbar->addWidget(charCountLabel);
    ui->statusbar->addPermanentWidget(fontSizeLabel);

    // File
    connect(ui->actionSave, &QAction::triggered, this, &Notepad::save);
    connect(ui->actionSave_as, &QAction::triggered, this, &Notepad::saveAs);

    connect(ui->textEdit, &QTextEdit::textChanged, this, &Notepad::textChanged);
    connect(ui->textEdit, &QTextEdit::cursorPositionChanged, this, &Notepad::textCursorPosChanged);
    connect(ui->textEdit, &QTextEdit::selectionChanged, this, &Notepad::textSelected);

    updateWindowTitle();

    resetFontLabel();
}

Notepad::~Notepad()
{
    delete ui;
}

QMessageBox::StandardButton Notepad::askSave() {
    return QMessageBox::question(this,
                                 tr("Save?"),
                                 tr("Do you want to save the current file?"),
                                 Button::Save | Button::Discard | Button::Cancel,
                                 Button::Save);
}

void Notepad::on_actionNew_triggered() {
    if (ui->textEdit->document()->isModified()) {
        Button clicked = askSave();

        if (clicked == Button::Save && !save())
            return;
    }

    ui->textEdit->clear();
    ui->textEdit->document()->setModified(false);
    updateWindowTitle();
}

void Notepad::on_actionOpen_triggered() {
    Button clicked = Button::Save;
    if (ui->textEdit->document()->isModified()) {
        clicked = askSave();
    }

    if(clicked == Button::Cancel) {
        return;
    }

    QString filename = QFileDialog::getOpenFileName(this, tr("Open the file"));

    if (filename.isEmpty()) {
        return;
    }

    QFile file(filename);
    currentFile = filename;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this,
                             tr("Warning"),
                             tr("Cannot open file: %1").arg(file.errorString()));
        return;
    }

    QTextStream in(&file);
    QString text = in.readAll();
    ui->textEdit->setText(text);
    ui->textEdit->document()->setModified(false);
    updateWindowTitle();
}

bool Notepad::save() {
    QString filename;
    if (currentFile.isEmpty()) {
        filename = QFileDialog::getSaveFileName(this, tr("Save"));
        if (filename.isEmpty()) {
            return false;
        }
        currentFile = filename;
    } else {
        filename = currentFile;
    }

    return saveOutputToFile(filename);
}

bool Notepad::saveAs() {
    QString filename = QFileDialog::getSaveFileName(this, tr("Save as"));
    if (filename.isEmpty()) {
        return false;
    }

    if (!saveOutputToFile(filename))
        return false;

    currentFile = filename;
    return true;
}

void Notepad::on_actionSelect_Font_triggered() {
    bool fontSelected;
    QFont font = QFontDialog::getFont(&fontSelected, this);
    if (fontSelected)
        ui->textEdit->setFont(font);
}

void Notepad::on_actionZoom_In_triggered() {
    ui->textEdit->zoomIn(1);
    resetFontLabel();
}

void Notepad::on_actionZoom_Out_triggered(){
    ui->textEdit->zoomOut(1);
    resetFontLabel();
}

void Notepad::on_actionReset_Zoom_triggered() {
    QFont font = ui->textEdit->font();
    font.setPointSize(10);
    ui->textEdit->setFont(font);
    resetFontLabel();
}

void Notepad::resetFontLabel() {
    fontSizeLabel->setText(tr("Size: %1 pt")
                               .arg(QString::number(ui->textEdit->fontInfo().pointSize()))
    );
}

void Notepad::on_actionStatus_Bar_triggered() {
    ui->statusbar->setVisible(ui->actionStatus_Bar->isChecked());
}

void Notepad::on_actionPrint_file_triggered()
{
    #if defined(QT_PRINTSUPPORT_LIB) && QT_CONFIG(printer)
        QPrinter printDev;
        #if QT_CONFIG(printdialog)
            QPrintDialog dialog(&printDev, this);
            if (dialog.exec() == QDialog::Rejected)
                return;
        #endif // QT_CONFIG(printdialog
        ui->textEdit->print(&printDev);
    #endif // QT_CONFIG(printer)
}

void Notepad::on_actionFindReplace_triggered() {
    if (!findWindow) {
        findWindow = new FindWindow(ui->textEdit, this);

        findWindow->setAttribute(Qt::WA_DeleteOnClose);

        connect(findWindow, &QObject::destroyed, this, [this]() {
            findWindow = nullptr;
        });

        findWindow->show();
    } else {
        findWindow->raise();
        findWindow->activateWindow();
    }
}

void Notepad::on_actionSave_and_Quit_triggered() {
    if (save())
        QApplication::quit();
}

void Notepad::on_actionDiscard_changes_triggered() {
    ui->textEdit->document()->setModified(false);
    QApplication::quit();
}

void Notepad::closeEvent(QCloseEvent *event) {
    if (ui->textEdit->document()->isModified()) {
        Button clicked = askSave();

        if (clicked == Button::Save) {
            if (save())
                event->accept();
            else
                event->ignore();
        }
        else if (clicked == Button::Cancel) {
            event->ignore();
        }
        else {
            event->accept();
        }
    } else {
        event->accept();
    }
}

bool Notepad::saveOutputToFile(const QString& filename) {
    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this,
                             tr("Warning"),
                             tr("Cannot save file: ") + file.errorString());
        return false;
    }
    ui->textEdit->document()->setModified(false);
    updateWindowTitle();
    QTextStream out(&file);
    QString text = ui->textEdit->toPlainText();
    out << text;
    return true;
}

void Notepad::updateWindowTitle() {
    const QString title = currentFile.isEmpty() ? defaultTitle : currentFile;

    setWindowTitle(ui->textEdit->document()->isModified() ? "*" + title : title);
}

void Notepad::textChanged() {
    updateWindowTitle();
    updateCharCount();
}

void Notepad::textSelected() {
    updateCharCount();
}

void Notepad::updateCharCount() {
    const QTextCursor cursor = ui->textEdit->textCursor();
    const int total = ui->textEdit->toPlainText().size();

    if (cursor.hasSelection()) {
        charCountLabel->setText(
            tr("%1 of %2 characters")
                .arg(cursor.selectedText().size())
                .arg(total)
            );
    } else {
        charCountLabel->setText(
            tr("%1 characters").arg(total)
            );
    }
}

void Notepad::textCursorPosChanged() {
    const QTextCursor cursor = ui->textEdit->textCursor();
    textCursorPosLabel->setText(
                                tr("Line %1 Col %2")
                                    .arg(QString::number(cursor.blockNumber()), QString::number(cursor.columnNumber()))
    );
}



















