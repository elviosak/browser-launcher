#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <QDir>
#include <QPushButton>
#include <QFileDialog>
#include <QFile>
#include <QFormLayout>
#include <QComboBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QWidget>
#include <QSettings>
#include <QString>
#include <QDebug>
#include <QProcess>
#include <QRadioButton>
#include <QTimer>
#include <QApplication>
#include <QSizePolicy>
#include <QList>
#include <QStyle>
#include <QMessageBox>
#include <QSpinBox>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QString const arg, QWidget *parent = nullptr);
    ~MainWindow();

private:
    const QString url;
    bool editMode;
    QWidget * central;
    QIcon appIcon;
    QSettings * settings;
    QVBoxLayout * vbox;
    QHBoxLayout * defaultBox;
    QHBoxLayout * addBox;
    QGridLayout * grid;
    QWidget * gridWidget;
    QTimer * timer;
    int counter;
    QPushButton * btnDefault;
    QLineEdit * lineUrl;
private slots:
    void initSettings();
    void createGrid();
    void reloadGrid();
    void createHeader();
    void createRow(QString browser);
    void createAddBox();
    void createDefaultBox();
    void minusOne();
    void launchDefault();
    void launch(QString cmd);
    void stopTimer();
    void createLauncher();
};
#endif // MAINWINDOW_H
