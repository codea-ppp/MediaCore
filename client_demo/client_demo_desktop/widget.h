#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QListWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    static Widget* get_instance();

    int video_update_callback(std::shared_ptr<std::vector<std::string>>);
    void _play(QListWidgetItem* item);

    Widget(QWidget *parent = nullptr);
    ~Widget();

private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
