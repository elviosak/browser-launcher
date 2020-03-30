#include "mainwindow.h"
MainWindow::~MainWindow()
{
}

MainWindow::MainWindow(QString const arg, QWidget *parent)
    : QMainWindow(parent),
      url(arg),
      appIcon(QIcon(":/appicon"))

{
    settings = new QSettings("browser-launcher", "browserlist");
    initSettings();
    editMode = (url == "");
    //editMode = false;
    lineUrl = new QLineEdit(url);
    lineUrl->setPlaceholderText("Enter Url:");
    connect(lineUrl, &QLineEdit::textChanged,this, [=](QString lineUrlText){
        auto newTitle = lineUrlText == "" ? "Browser Launcher" : "Browser Launcher - " + lineUrlText;
        setWindowTitle(newTitle);
    });
    auto defaultIcon = settings->value("defaultIcon").toString();
    btnDefault = new QPushButton(QIcon::fromTheme(defaultIcon, appIcon), QString::number(counter));
    btnDefault->setMinimumHeight(30);

    //&MainWindow::launchDefault);

    auto title = url == "" ? "Browser Launcher" : "Browser Launcher - " + url;
    setWindowIcon(appIcon);
    setWindowTitle(title);

    central = new QWidget(this);
    vbox = new QVBoxLayout(central);

    gridWidget = new QWidget();
    gridWidget->setObjectName("loaded");


    setCentralWidget(central);


    if(editMode){
        QGroupBox * groupAdd = new QGroupBox();
         addBox = new QHBoxLayout(groupAdd);
        groupAdd->setTitle("Add new Browser");
        //groupAdd->setLayout(addBox);
        QWidget * defaultWidget = new QWidget(central);
        defaultBox = new QHBoxLayout(defaultWidget);
        vbox->setSpacing(10);
        vbox->addWidget(defaultWidget);
        vbox->addWidget(gridWidget);
        vbox->addWidget(groupAdd);

        createGrid();
        createAddBox();
        createDefaultBox();
        resize(640,200);
    }
    else {
        vbox->addWidget(gridWidget);
        createLauncher();
        resize(400,100);
    }

    if(!editMode){
        timer->start(1000);
        btnDefault->setFocus();
    }
    show();   
}

void MainWindow::createLauncher(){
    QGridLayout * grid;
    if(gridWidget->layout()){
        grid = qobject_cast<QGridLayout*>(gridWidget->layout());
    }else {
        grid = new QGridLayout(gridWidget);
    }
    QStringList groups = settings->childGroups();
    if(groups.size()==0){
        grid->addWidget(new QLabel("No Browser Detected"));
        return;
    }
    for (int i = 0; i < groups.size(); ++i) {
        QString browser = groups.at(i);
        QString icon = settings->value(browser+"/Icon").toString();
        QString path = settings->value(browser+"/Exec").toString();
        QString extra = settings->value(browser+"/Action").toString();
        QString tooltip = (path + " " + extra).simplified();
        auto radio = new QRadioButton("default",gridWidget);
        QString defaultBrowser = settings->value("defaultBrowser").toString();
        qDebug() << "default:" << defaultBrowser;
        if(browser == defaultBrowser)
            radio->setChecked(true);
        connect(radio, &QRadioButton::toggled,this, [=](bool toggled){
            if(toggled){
                settings->setValue("defaultIcon", icon);
                settings->setValue("defaultBrowser", browser);
                btnDefault->setIcon(QIcon::fromTheme(icon, appIcon));
                qDebug()<< "toggled" << browser << "default";
            }
        });
        QIcon browserIcon = QIcon::fromTheme(icon);
        if(browserIcon.isNull())
            browserIcon = appIcon;
        auto btn = new QPushButton(browserIcon, browser, gridWidget);
        btn->setToolTip(tooltip);
        connect(btn, &QPushButton::clicked, this, [=]{
            QString cmd = (path + " " + extra + " " + url).simplified();
            launch(cmd);
        });
        grid->addWidget(radio, i, 0, 1, 2, Qt::AlignHCenter);
        grid->addWidget(btn, i, 2, 1, 3);
    }
    int row = grid->rowCount();
    grid->addWidget(lineUrl, row, 0, 1, 4);
    grid->addWidget(btnDefault, row, 4, 1, 1);
    connect(btnDefault, &QPushButton::clicked, this, [=](){
        launchDefault();
    });
}

void MainWindow::initSettings(){
    if(settings->childGroups().size()==0){
        QDir * dir = new QDir("/usr/share/applications/", "*.desktop");
        QStringList list = dir->entryList();
        foreach(QString file, list){
            if(file != "browser-launcher.desktop"){
                QSettings * desktop = new QSettings("/usr/share/applications/" + file, QSettings::Format::IniFormat);
                QString string = desktop->value("Desktop Entry/MimeType").toString();
                if(string.split(";", QString::SplitBehavior::SkipEmptyParts).contains("text/html")){
                    QString name = desktop->value("Desktop Entry/Name").toString();
                    QString icon = desktop->value("Desktop Entry/Icon").toString();
                    QString path = desktop->value("Desktop Entry/Exec").toString();
                    path = path.replace("%u", "").replace("%U", "").trimmed();
                    settings->setValue(name+"/Icon", icon);
                    settings->setValue(name+"/Exec", path);
                    settings->setValue(name+"/Action", "");
                }
            }
        }
    }
    if(settings->value("defaultBrowser").toString() == "" && (settings->childGroups()).size() > 0){

        auto defaultBrowser = settings->childGroups().first();
        settings->setValue("defaultBrowser", defaultBrowser);
        auto defaultIcon = settings->value(defaultBrowser + "/Icon").toString();
        settings->setValue("defaultIcon", defaultIcon);
    }

    if(settings->value("defaultTimer").toInt() == 0){
        settings->setValue("defaultTimer", 5);
    }

    counter = settings->value("defaultTimer").toInt();
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::minusOne);
}

void MainWindow::createGrid(){
    auto groups = settings->childGroups();
    createHeader();
    foreach(QString browser, groups){
        createRow(browser);
    }
}

void MainWindow::reloadGrid(){
    vbox->takeAt(1);
    gridWidget->~QWidget();
    gridWidget = new QWidget();
    gridWidget->setObjectName("reloaded");
    vbox->insertWidget(1, gridWidget);
    createGrid();
    resize(640,200);
}
void MainWindow::createHeader(){
    QGridLayout * grid;
    if(gridWidget->layout()){
        grid = qobject_cast<QGridLayout*>(gridWidget->layout());
    }else {
        grid = new QGridLayout(gridWidget);
    }

    grid->addWidget(new QLabel("Command"), 0, 1, 1, 4, Qt::AlignHCenter);
    grid->addWidget(new QLabel("Arg"), 0, 5, 1, 2, Qt::AlignHCenter);
    grid->addWidget(new QLabel("Set default"), 0, 7, 1, 1, Qt::AlignHCenter);
    grid->addWidget(new QLabel("Launch"), 0, 8, 1, 2, Qt::AlignHCenter);
}

void MainWindow::createRow(QString browser){
    QGridLayout * grid;
    if(gridWidget->layout()){
        grid = qobject_cast<QGridLayout*>(gridWidget->layout());
    }else {
        grid = new QGridLayout(gridWidget);
    }
    QString icon = settings->value(browser+"/Icon").toString();
    QString path = settings->value(browser+"/Exec").toString();
    QString extra = settings->value(browser+"/Action").toString();
    auto row = grid->rowCount();

    auto btnDel = new QPushButton("Delete",gridWidget);
    btnDel->setIcon(QIcon(":/delete"));
    connect(btnDel, &QPushButton::clicked, this, [=]{
        settings->remove(browser);
        reloadGrid();
    });
    btnDel->setFocusPolicy(Qt::NoFocus);
    auto line = new QLineEdit(path, gridWidget);
    auto combo = new QComboBox(gridWidget);
    combo->addItems({"--new-window","--private-window", "--incognito"});
    combo->setEditable(true);
    combo->setCurrentText(extra);

    auto radio = new QRadioButton(gridWidget);

    if(browser == settings->value("defaultBrowser").toString())
        radio->setChecked(true);
    connect(radio, &QRadioButton::toggled,this, [=](bool toggled){
        if(toggled){
            settings->setValue("defaultIcon", icon);
            settings->setValue("defaultBrowser", browser);
            btnDefault->setIcon(QIcon::fromTheme(icon, appIcon));
            qDebug()<< "toggled" << browser << "default";
        }
    });
    QIcon browserIcon = QIcon::fromTheme(icon);
    if(browserIcon.isNull())
        browserIcon = appIcon;
    auto btn = new QPushButton(browserIcon,"Launch " + browser,gridWidget);
    grid->addWidget(btnDel, row, 0, 1, 1);
    grid->addWidget(line, row, 1, 1, 4);
    grid->addWidget(combo, row, 5, 1, 2);
    grid->addWidget(radio, row, 7, 1, 1, Qt::AlignHCenter);
    grid->addWidget(btn, row, 8, 1, 2);
    connect(line, &QLineEdit::textChanged,this, [=](const QString newPath){
        settings->setValue(browser+"/Exec", newPath.trimmed());
    });
    connect(combo, &QComboBox::currentTextChanged, this, [=](const QString newExtra){
        settings->setValue(browser+"/Action", newExtra.trimmed());
    });
    connect(btn, &QPushButton::clicked, this, [=]{
        auto newPath = line->text().trimmed();
        auto newExtra = combo->currentText().trimmed();
        auto newUrl = lineUrl->text().trimmed();
        QString cmd = (newPath + " " + newExtra + " " + newUrl).simplified();
        launch(cmd);
    });

}

void MainWindow::createAddBox(){
    auto btnFind = new QPushButton("Find");
    btnFind->setIcon(QIcon(":/find"));
    auto lineName = new QLineEdit();
    lineName->setPlaceholderText("Name");
    auto lineIcon = new QLineEdit();
    lineIcon->setPlaceholderText("Icon name");
    auto linePath = new QLineEdit();
    linePath->setPlaceholderText("Command to execute");


    auto btnSave = new QPushButton(appIcon, "Add: ");
    connect(btnFind, &QPushButton::clicked, this, [=](){
        QString file = QFileDialog::getOpenFileUrl(this, tr("Open File"),
                                                    QUrl("file:///usr/share/applications"),
                                                    tr("Desktop files (*.desktop)")).toLocalFile();
        QString filePath = file;
        if(filePath != ""){
            QSettings * desktop = new QSettings(file, QSettings::Format::IniFormat);
            QString name = desktop->value("Desktop Entry/Name").toString();
            QString icon = desktop->value("Desktop Entry/Icon").toString();
            QString path = desktop->value("Desktop Entry/Exec").toString();
            path = path.replace("%u", "").replace("%U", "").trimmed();
            lineName->setText(name);
            lineIcon->setText(icon);
            linePath->setText(path);
            btnSave->setIcon(QIcon::fromTheme(icon, appIcon));
            btnSave->setText("Add: " + name);
        }
    });
    connect(lineName, &QLineEdit::editingFinished , this, [=](){
        if(lineName->text() != ""){
            btnSave->setText("Add: " + lineName->text());
        }
    });
    connect(lineIcon, &QLineEdit::textChanged, this, [=](QString newIcon){
        if(newIcon != ""){
            btnSave->setIcon(QIcon::fromTheme(newIcon, appIcon));
        }
    });
    connect(btnSave, &QPushButton::clicked, this, [=]{
        QString text = lineName->text();
        if(text == ""){
            QMessageBox * msg = new QMessageBox(QMessageBox::Icon::Warning,
                                                "Error saving data",
                                                "Please type a Name eg:\"MyBrowser\"",
                                                QMessageBox::Ok);
            msg->show();
        }
        else if(settings->childGroups().contains(text)){
            QMessageBox * msg = new QMessageBox(QMessageBox::Icon::Warning,
                                                "Error saving data",
                                                "Name: \"" + text + "\" already exists, please choose another name",
                                                QMessageBox::Ok);
            msg->show();
        }
        else if(lineIcon->text() == ""){
            QMessageBox * msg = new QMessageBox(QMessageBox::Icon::Warning,
                                                "Error saving data",
                                                "Please type icon name eg:\"firefox\"",
                                                QMessageBox::Ok);
            msg->show();
        }
        else if(linePath->text() == ""){
            QMessageBox * msg = new QMessageBox(QMessageBox::Icon::Warning,
                                                "Error saving data",
                                                "Please type command to execute eg:\"/usr/bin/firefox\" or \"firefox\"",
                                                QMessageBox::Ok);
            msg->show();
        }
        else {
            auto name = lineName->text().trimmed();
            auto icon = lineIcon->text().trimmed();
            auto path = linePath->text().trimmed();
            settings->setValue(name+"/Icon", icon);
            settings->setValue(name+"/Exec", path);
            settings->setValue(name+"/Action", "");
            createRow(name);
            lineName->setText("");
            lineIcon->setText("");
            linePath->setText("");
            btnSave->setIcon(appIcon);
            btnSave->setText("Save: ");
        }
    });    
    addBox->addWidget(btnFind);
    addBox->addWidget(lineName);
    addBox->addWidget(lineIcon);
    addBox->addWidget(linePath,2);
    addBox->addWidget(btnSave);
}

void MainWindow::createDefaultBox(){


    auto defaultTimer = settings->value("defaultTimer").toInt();
    QSpinBox * spinTimer = new QSpinBox();
    spinTimer->setValue(defaultTimer);
    spinTimer->setRange(1, 20);
    spinTimer->setSuffix(" s");
    connect(spinTimer, QOverload<int>::of(&QSpinBox::valueChanged), this, [=](int newTimer){
        settings->setValue("defaultTimer", newTimer);
        counter = newTimer;
    });

    QHBoxLayout * timerBox = new QHBoxLayout();
    timerBox->addWidget(new QLabel("Default Timer:"));
    timerBox->addWidget(spinTimer);
    defaultBox->addWidget(lineUrl, 4);
    defaultBox->addLayout(timerBox, 1);
//    defaultBox->addWidget(btnDefault, 1);

//    connect(btnDefault, &QPushButton::clicked, this, [=](){
//        launchDefault();
//    });
}

void MainWindow::stopTimer(){
    if(timer->isActive()){
        timer->stop();
        btnDefault->setText("Timer stopped");
    }
}

void MainWindow::minusOne(){
    --counter;
    btnDefault->setText(QString::number(counter));
    if(counter <= 0)
        launchDefault();
}

void MainWindow::launchDefault(){
    auto defaultBrowser = settings->value("defaultBrowser").toString();
    QString path = settings->value(defaultBrowser+"/Exec").toString();
    QString extra = settings->value(defaultBrowser+"/Action").toString();
    QString newUrl = lineUrl->text().trimmed();
    QString cmd = (path + " " + extra + " " + newUrl).simplified();
    launch(cmd);
}

void MainWindow::launch(QString cmd){
    qDebug() << "launch:" << cmd;
    timer->stop();
    QProcess::startDetached(cmd);
    if(!editMode)
        QApplication::quit();
}

