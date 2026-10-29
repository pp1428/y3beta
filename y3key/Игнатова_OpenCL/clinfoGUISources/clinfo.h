#ifndef CLINFO_H
#define CLINFO_H

#include <QMainWindow>
#include <QList>
#include <computeunit.h>

namespace Ui {
    class clinfo;
}

class clinfo : public QMainWindow
{
    Q_OBJECT

public:
    QList<ComputeUnit> listComUnits;
    explicit clinfo(QWidget *parent = 0);
    ~clinfo();
    void getUnits();

private:
    Ui::clinfo *ui;

private slots:
    void on_actionNew_Analysis_triggered();
    void on_actionCredits_triggered();
    void on_computeUnits_currentIndexChanged(int index);
    void on_computeUnits_currentIndexChanged(QString );
};

#endif // CLINFO_H
