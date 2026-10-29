#include "clinfo.h"
#include "ui_clinfo.h"
#include <QFile>
#include <QDebug>
#include <QTextStream>
#include <QMessageBox>

clinfo::clinfo(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::clinfo)
{
    ui->setupUi(this);
    getUnits();

}

void clinfo::getUnits()
{
    system("./gatherinfo.sh &");

    int maxWait=5;
    QFile file("info.temp");
    while (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        sleep(1);
        maxWait--;
        // return if problems don't resolve
        if(maxWait<1)
            return;
    }
    QTextStream stream( &file );


        // clear internal list of compute units
        listComUnits.clear();

        // default compute unit
        ComputeUnit cmpUnit;


       // var used to store current line
       QString line;

       while (!stream.atEnd())
       {

           line =stream.readLine();

           if(QString::compare(line.trimmed(),"#UNIT")==0)
           {

               // get characteristics
               cmpUnit.vendor = stream.readLine();
               cmpUnit.model=stream.readLine();
               cmpUnit.cores=stream.readLine();
               cmpUnit.gflops=stream.readLine();
               cmpUnit.opencl=stream.readLine();

               // append unit object to list
               listComUnits.append(cmpUnit);
           }

       }

       //close file after reading complete
       file.close();

        // clear ui list of units & update
       ui->computeUnits->clear();
       for(int i=0;i<listComUnits.count();i++)
           ui->computeUnits->addItem(listComUnits.at(i).vendor);
}

clinfo::~clinfo()
{
    delete ui;
}

void clinfo::on_computeUnits_currentIndexChanged(QString )
{

}

void clinfo::on_computeUnits_currentIndexChanged(int index)
{
    if((index<listComUnits.count())&&(index>=0))
    {
        ComputeUnit cmpUnit=listComUnits.at(index);

        if(cmpUnit.vendor=="AMD")
            ui->image->setPixmap(QPixmap("images/amd.jpg"));
        else if(cmpUnit.vendor=="AMD/ATI")
            ui->image->setPixmap(QPixmap("images/radeon.jpg"));
        else if(cmpUnit.vendor=="INTEL")
            ui->image->setPixmap(QPixmap("images/intel.jpg"));
        else if(cmpUnit.vendor=="NVIDIA")
            ui->image->setPixmap(QPixmap("images/nvidia.jpg"));
        else
            ui->image->setPixmap(QPixmap("images/unknown.jpg"));

        // data insertion
        ui->modelTextbox->setText(cmpUnit.model);
        ui->coresTextbox->setText(cmpUnit.cores);
        ui->gflopsTextbox->setText(cmpUnit.gflops);
        ui->openclTextbox->setText(cmpUnit.opencl);
    }

}

void clinfo::on_actionCredits_triggered()
{
    QMessageBox msgBox;
    msgBox.information(0,"clinfo GUI","clinfo 2010-present\nclinfo GUI 2010-present\nLupescu Grigore");
}

void clinfo::on_actionNew_Analysis_triggered()
{
     getUnits();
}
