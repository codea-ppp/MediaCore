#include "widget.h"
#include "client.h"
#include "ui_widget.h"
#include "threadpool_instance.h"

Widget* Widget::get_instance()
{
    static Widget instance;
    return &instance;
}

int Widget::video_update_callback(std::shared_ptr<std::vector<std::string>> update_videos)
{
    for (auto i = update_videos->begin(); i != update_videos->end(); ++i)
    {
        ui->listWidget->addItem(QString(i->c_str()));
    }

    return 0;
}

void Widget::_play(QListWidgetItem *item)
{
    threadpool_instance::get_instance()->schedule(std::bind(&client::play, client::get_instance(), item->text().toStdString()));
}

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    QObject::connect(ui->listWidget, &QListWidget::itemDoubleClicked, this, &Widget::_play);
}

Widget::~Widget()
{
    delete ui;
}

