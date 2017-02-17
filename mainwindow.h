#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QImage>
#include <QDebug>
#include <QFileDialog>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>

#include "slic.h"
#include "merw.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    cv::Mat image;
    cv::Mat contour;
    cv::Mat map;
    cv::Mat averagedMap;
    cv::Mat stationaryDist;

    void setRawImage();
    void setSlicOutline();
    void setAveragedSlic();
    void setStationaryDist();

    void addImage(const cv::Mat& mat, QLabel* view);

private slots:
    void onOpenFile();

    void onCalculateButtonClick();
};

#endif // MAINWINDOW_H
