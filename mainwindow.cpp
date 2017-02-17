#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->calculateButton, SIGNAL(clicked()), this, SLOT(onCalculateButtonClick()));
    connect(ui->actionLoad, SIGNAL(triggered()), this, SLOT(onOpenFile()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setRawImage()
{
    addImage(image, ui->image0View);
}

void MainWindow::setSlicOutline()
{
    addImage(contour, ui->image1View);
}

void MainWindow::setAveragedSlic()
{
    addImage(averagedMap, ui->image2View);
}

void MainWindow::setStationaryDist()
{
    addImage(stationaryDist, ui->image3View);
}

void MainWindow::addImage(const cv::Mat& mat, QLabel* view)
{
    // 8-bits unsigned, NO. OF CHANNELS=1
    if(mat.type()==CV_8UC1)
    {
        // Set the color table (used to translate colour indexes to qRgb values)
        QVector<QRgb> colorTable;
        for (int i=0; i<256; i++)
            colorTable.push_back(qRgb(i,i,i));
        // Copy input Mat
        const uchar *qImageBuffer = (const uchar*)mat.data;
        // Create QImage with same dimensions as input Mat
        QImage img(qImageBuffer, mat.cols, mat.rows, mat.step, QImage::Format_Indexed8);
        img.setColorTable(colorTable);
        view->setPixmap(QPixmap::fromImage(img));
    }
    // 8-bits unsigned, NO. OF CHANNELS=3
    else if(mat.type()==CV_8UC3)
    {
        cv::Mat temp;
        cv:cvtColor(mat, temp, CV_BGR2RGB);
        // Copy input Mat
        const uchar *qImageBuffer = (const uchar*)temp.data;
        // Create QImage with same dimensions as input Mat
        QImage img(qImageBuffer, temp.cols, temp.rows, temp.step, QImage::Format_RGB888);
        view->setPixmap(QPixmap::fromImage(img));
    }
    else
    {
        qDebug() << "ERROR: Mat could not be converted to QImage.";
    }
}

void MainWindow::onCalculateButtonClick()
{
    ui->calculateButton->setEnabled(false);

    int iterations = ui->slicIterationsSlider->value();
    float ruler = (float)ui->slicRulerSlider->value() / 100.f;
    int size = ui->slicSizeSlider->value();
    bool enforceConnectivity = ui->slicEnforceConnectivityCheckBox->isChecked();

    Slic slic(size, ruler, iterations, enforceConnectivity);
    qDebug() << "INFO: Creating new SLIC with params: iter: " << iterations << " ruler: " << ruler << " size: " << size << " enforceConnectivity: " << enforceConnectivity << ".";

    int regions = 0;

    slic.process(image, &map, &contour, &regions);
    qDebug() << "INFO: SLIC returned: " << regions << " regions.";

    setSlicOutline();
    ui->numberOfNodesLabel->setText(QString::number(regions));
    QCoreApplication::processEvents();

    double colorValue = (double)ui->merwColorDistanceSlider->value() / 100.;
    bool localizeGraph = ui->merwLocalizeGraphCheckbox->isChecked();
    double epsilon = std::pow<double>(10, -ui->merwEpsilonSlider->value());

    Merw merw(image, map, regions);
    merw.calculateGraph(localizeGraph);
    merw.createAveragedImage(&averagedMap);
    setAveragedSlic();
    QCoreApplication::processEvents();

    qDebug() << "INFO: performing merw with colorDistance param: " << colorValue << ", localizeGraph: " << localizeGraph << " and epsilon: " << epsilon << ".";
    merw.calculateStationaryDistribution(&stationaryDist, colorValue, epsilon);
    setStationaryDist();
    QCoreApplication::processEvents();

    ui->calculateButton->setEnabled(true);

    qDebug() << "INFO: Done.";
}

void MainWindow::onOpenFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open file", "", "Files (*.*)");

    qDebug() << "INFO: Opening file: " << fileName << ".";

    cv::Mat loadedImage = cv::imread(fileName.toStdString());

    double maxWidth = ui->image0View->geometry().width();
    double maxHeight = ui->image0View->geometry().height();

    double scale = std::min(maxWidth / loadedImage.cols, maxHeight / loadedImage.rows);

    cv::resize(loadedImage, image, cv::Size(), scale, scale);

    if(fileName.length() > 0)
        ui->calculateButton->setEnabled(true);

    setRawImage();
}

