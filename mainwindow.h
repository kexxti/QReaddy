#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlQueryModel>
#include "pdfviewer.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class PdfViewer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_AddBtn_clicked();
    void on_SearchBtn_clicked();
    void on_OpenBtn_clicked();
    void on_DeleteBtn_clicked();

    void on_SearchEdit_textEdited(const QString &arg1);

    void on_action_csv_triggered();

private:
    Ui::MainWindow *ui;
    QSqlDatabase db;
    QSqlQueryModel *model;
    PdfViewer *pdfViewer;


    void setupDB();
    void loadData();
};

#endif // MAINWINDOW_H
