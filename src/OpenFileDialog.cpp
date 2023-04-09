#include "OpenFileDialog.h"
#include <QFile>

#include <QDoubleValidator>

OpenFileDialog::OpenFileDialog(QString fileName, QWidget *parent)
	: myFileName(fileName), QDialog(parent)
{
	ui.setupUi(this);
    setWindowTitle(fileName);

    // �޶�����Ϊ������
    QDoubleValidator* doubleValidator = new QDoubleValidator();//QDoubleValidator �޶����븡���� ��QIntValidator �C ֻ���û���������
    ui.XAxis_left_Edit->setValidator(doubleValidator);
    ui.XAxis_right_Edit->setValidator(doubleValidator);
    ui.YAxis_left_Edit->setValidator(doubleValidator);
    ui.YAxis_right_Edit->setValidator(doubleValidator); 

    // ���ؼ���װ�¼�������
    ui.XAxis_left_Edit->installEventFilter(this);  
    ui.XAxis_right_Edit->installEventFilter(this); 
    ui.YAxis_left_Edit->installEventFilter(this);  
    ui.YAxis_right_Edit->installEventFilter(this);
    ui.customPlot->installEventFilter(this);
    
    pPlot = ui.customPlot; // ��customPlot��ͼ�ؼ������ø�������������д

    // ���߹����ر���
    mTracer = TracerFlag::NoTracer;
    tracerCross = Q_NULLPTR;
    lineTracer = Q_NULLPTR;
    for (int i = 0; i < 4; i++) {
        tracerX[i] = Q_NULLPTR;
    }
    lastPos = 0;

    // ��ȡ�ļ�����
	ReadFile();
	// ��ʼ��ͼ��
	QPlot_init(pPlot);
    // ����ͼƬ
    ShowPlot();
}

OpenFileDialog::~OpenFileDialog()
{
	delete this; // �ô���Ϊ��ģ̬��
}

// ��ȡ����
void OpenFileDialog::ReadFile() {
	QFile file(myFileName);
	file.open(QIODevice::ReadOnly | QIODevice::Text);
	QByteArray array;
	int line = 0;
	QTextStream in(&file);
	while (!in.atEnd()){
		QString str = in.readLine();
		QStringList column = str.split(" ", QString::SkipEmptyParts);
		line++;
		if (line > 2) {
			int count1 = column.at(1).toInt();
			int count2 = column.at(2).toInt();
			int count3 = column.at(3).toInt();
			int count4 = column.at(4).toInt();
			counter1.push_back(count1*1.0);
			counter2.push_back(count2*1.0);
			counter3.push_back(count3*1.0);
			counter4.push_back(count4*1.0);
		}
	}
}

// ��ͼͼ���ʼ��
void OpenFileDialog::QPlot_init(QCustomPlot* customPlot)
{
    // ͼ�������������
    pGraph1 = customPlot->addGraph();
    pGraph2 = customPlot->addGraph();
    pGraph3 = customPlot->addGraph();
    pGraph4 = customPlot->addGraph();

    //ui->checkBox1->setCheckState(Qt::Checked);  //���ø�ѡ���ʼ״̬ Unchecked
    //ui->checkBox2->setCheckState(Qt::Checked);
    //ui->checkBox3->setCheckState(Qt::Checked);
    //ui->checkBox4->setCheckState(Qt::Checked);
    //ui->rescaleAxesCheckBox->setCheckState(Qt::Checked); // ����������Ӧ
    //ui->refreshPlotCheckBox->setCheckState(Qt::Checked); // ͼ��ˢ��

    // ����������ɫ
    pGraph1->setPen(QPen(Qt::red));
    pGraph2->setPen(QPen(Qt::darkRed));
    pGraph3->setPen(QPen(Qt::green));
    pGraph4->setPen(QPen(Qt::blue));

    // ��������������
    customPlot->xAxis->setLabel("ʱ��/s");
    customPlot->yAxis->setLabel("������/cps");

    // ����y��������ʾ��Χ
    customPlot->xAxis->setRange(0, 120);

    // ��ʾͼ���ͼ��
    customPlot->legend->setBrush(QColor(255, 255, 255, 0));//legend����ɫ��Ϊ��ɫ������͸��������ͼ����legend����ɼ�
    customPlot->legend->setVisible(true);

    // �����������
    pGraph1->setName("̽����1");
    pGraph2->setName("̽����2");
    pGraph3->setName("̽����3");
    pGraph4->setName("̽����4");

    // ���ò������ߵĸ�ѡ��������ɫ
    //ui->checkBox1->setStyleSheet("QCheckBox{color:red}");//�趨ǰ����ɫ,����������ɫ
    //ui->checkBox2->setStyleSheet("QCheckBox{color:darkRed}");
    //ui->checkBox3->setStyleSheet("QCheckBox{color:green}");
    //ui->checkBox4->setStyleSheet("QCheckBox{color:blue}");

    // �����û�������϶��᷶Χ�������������ţ����ѡ��ͼ��:
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    //iRangeDrag ���������϶�; iRangeZoom ��Χ��ͨ������������; iSelectPlottables ������ѡ��

    // ���˫����ԭ
    connect(ui.customPlot, SIGNAL(mouseDoubleClick(QMouseEvent*)), this, SLOT(DoubleClick())); //��ȡ��Ϣ������

    // ����϶�����ȡ�����᷶Χ
    connect(ui.customPlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(myMoveMouse()));
    /*connect(ui.customPlot, &QCustomPlot::mouseDoubleClick, this, [=] {
        //ui.customPlot->xAxis->setRange(-100, 1000);
        //ui.customPlot->yAxis->setRange(-100, 100);
        pGraph1->rescaleAxes(); // �÷�Χ�������ţ�ʹͼ0��ȫ�ʺ��ڿɼ�����.���ﲻ�ܴ�����true
        pGraph2->rescaleAxes(true); // ͼ1Ҳ��һ���Զ�������Χ����ֻ�ǷŴ�򲻱䷶Χ
        pGraph3->rescaleAxes(true);
        pGraph4->rescaleAxes(true);
        ui.customPlot->replot();
        });
    */
}

void OpenFileDialog::ShowPlot()
{
    int times = counter1.size() * 1.0;
    QVector<double> time;
    for (int i=1; i<=times; i++) {
        time.push_back(i*1.0);
    }
    pGraph1->addData(time, counter1);
    pGraph2->addData(time, counter2);
    pGraph3->addData(time, counter3);
    pGraph4->addData(time, counter4);

    // �Զ�����������
    //if (RescaleAxesFlag) {
    pGraph1->rescaleAxes(); // �÷�Χ�������ţ�ʹͼ0��ȫ�ʺ��ڿɼ�����.���ﲻ�ܴ�����true
    pGraph2->rescaleAxes(true); // ͼ1Ҳ��һ���Զ�������Χ����ֻ�ǷŴ�򲻱䷶Χ
    pGraph3->rescaleAxes(true);
    pGraph4->rescaleAxes(true);
    //}
    pPlot->replot(QCustomPlot::rpQueuedReplot);
    
    UpdateAxisRange();
}

// ��Ӧ�༭������������뿪�༭��ʱ��ͼ�������Ӧ�����»���
// �¼�������
// Qt::Key_Left = 0x01000012,
// Qt::Key_Up = 0x01000013,
// Qt::Key_Right = 0x01000014,
// Qt::Key_Down = 0x01000015,
bool OpenFileDialog::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == ui.XAxis_left_Edit){
        if (event->type() == QEvent::KeyPress && 
            ((QKeyEvent*)event)->key() == Qt::Key_Enter){ //����Enter����
            setAxisRange();
        }
    }
    if (watched == ui.XAxis_right_Edit) {
        if ((event->type() == QEvent::FocusOut) || (event->type() == QEvent::KeyPress)){
            setAxisRange();
        }
    }
    if (watched == ui.YAxis_left_Edit) {
        if (event->type() == QEvent::KeyPress && 
            ((QKeyEvent*)event)->key() == Qt::Key_Enter) { //����Enter����
            setAxisRange();
        }
    }
    if (watched == ui.YAxis_right_Edit) {
        if ((event->type() == QEvent::FocusOut) || (event->type() == QEvent::KeyPress)){
            setAxisRange();
        }
    }
    if (mTracer == TracerFlag::CurveTracer) { //���������߹������Ӧ����¼�붯��
        if (watched == ui.customPlot && event->type() == QEvent::KeyPress) {
            if ( ((QKeyEvent*)event)->key() == Qt::Key_Left || 
                ((QKeyEvent*)event)->key() == Qt::Key_Down) { //�����������������
                DoCurveTracer(leftMove);
            }
            else if (((QKeyEvent*)event)->key() == Qt::Key_Right || 
                ((QKeyEvent*)event)->key() == Qt::Key_Up) { //���������ҷ��������
                DoCurveTracer(rightMove);
            }
        }
    }
    return QDialog::eventFilter(watched, event);
}

// ���ݽ������������������ķ�Χ
void OpenFileDialog::setAxisRange() {
    double xMin = ui.XAxis_left_Edit->text().toDouble();
    double xMax = ui.XAxis_right_Edit->text().toDouble();
    double yMin = ui.YAxis_left_Edit->text().toDouble();
    double yMax = ui.YAxis_right_Edit->text().toDouble();

    if (xMin > xMax) {
        QMessageBox::information(this, "�����᷶Χ����ʧ��", "��ȷ���������߽�С���ұ߽�");
        return;
    }
    if (yMin > yMax) {
        QMessageBox::information(this, "�����᷶Χ����ʧ��", "��ȷ���������߽�С���ұ߽�");
        return;
    }

    pPlot->xAxis->setRange(ui.XAxis_left_Edit->text().toDouble(), ui.XAxis_right_Edit->text().toDouble());
    pPlot->yAxis->setRange(ui.YAxis_left_Edit->text().toDouble(), ui.YAxis_right_Edit->text().toDouble());
    pPlot->replot(QCustomPlot::rpQueuedReplot);
}

// ������˫��ʱ�ػ棬���µ��������᷶Χ
void OpenFileDialog::DoubleClick() {
    pGraph1->rescaleAxes(); // �÷�Χ�������ţ�ʹͼ0��ȫ�ʺ��ڿɼ�����.���ﲻ�ܴ�����true
    pGraph2->rescaleAxes(true); // ͼ1Ҳ��һ���Զ�������Χ����ֻ�ǷŴ�򲻱䷶Χ
    pGraph3->rescaleAxes(true);
    pGraph4->rescaleAxes(true);
    ui.customPlot->replot();

    UpdateAxisRange();
}

// ����϶�����ʱ������仯�����᷶Χ
void OpenFileDialog::myMoveMouse() {
    UpdateAxisRange();
} 

// ˢ�������᷶Χ����ʾֵ��ͼ���Ӧ
void OpenFileDialog::UpdateAxisRange() {
    // ��ȡ��ǰ�����᷶Χ
    double left = ui.customPlot->xAxis->range().lower;
    double right = ui.customPlot->xAxis->range().upper;
    double down = ui.customPlot->yAxis->range().lower;
    double up = ui.customPlot->yAxis->range().upper;
    // ����ǰ�����᷶Χ���õ��ı���
    ui.XAxis_left_Edit->setText(QString::number(left, 'f', 1));
    ui.XAxis_right_Edit->setText(QString::number(right, 'f', 1));
    ui.YAxis_left_Edit->setText(QString::number(down, 'f', 1));
    ui.YAxis_right_Edit->setText(QString::number(up, 'f', 1));
}

// ����ͼ�α��浼��
bool OpenFileDialog::on_pbn_save_clicked()
{
    QJsonObject jsonSetting = mainWindow::ReadSetting();
    QString saveDir = jsonSetting["SavePictureDir"].toString(); //"SaveDir": "/home"
    QString wholePath = saveDir + "//Picture.bmp";

    QString fileName = QFileDialog::getSaveFileName(this, "ͼ�����Ϊ", wholePath, "BMP(*.bmp);; PNG(*.png);; JPG(*.jpg);; PDF(*.pdf)");
    //QString fileName = QFileDialog::getSaveFileName(this, "ͼ�����Ϊ", "test.bmp", "PNG(*.png);; JPG(*.jpg);; BMP(*.bmp);; PDF(*.pdf)");
    if (fileName == "") {
        QMessageBox::information(this, "fail", "����ʧ��");
        return false;
    }
    else if (fileName.endsWith(".png")) {
        saveFilePath(fileName);
        QMessageBox::information(this, "success", "�ɹ�����Ϊpng�ļ�");
        return ui.customPlot->savePng(fileName, ui.customPlot->width(), ui.customPlot->height());
    }
    else if (fileName.endsWith(".jpg") || fileName.endsWith(".jpeg")) {
        saveFilePath(fileName);
        QMessageBox::information(this, "success", "�ɹ�����Ϊjpg�ļ�");
        return ui.customPlot->saveJpg(fileName, ui.customPlot->width(), ui.customPlot->height());
    }
    else if (fileName.endsWith(".bmp")) {
        saveFilePath(fileName);
        QMessageBox::information(this, "success", "�ɹ�����Ϊbmp�ļ�");
        return ui.customPlot->saveBmp(fileName, ui.customPlot->width(), ui.customPlot->height());
    }
    else if (fileName.endsWith(".pdf")) {
        saveFilePath(fileName);
        QMessageBox::information(this, "success", "�ɹ�����Ϊpdf�ļ�");
        return ui.customPlot->savePdf(fileName, ui.customPlot->width(), ui.customPlot->height());
    }
    else {
        //����׷�Ӻ�׺��Ϊ.png�����ļ�
        QMessageBox::information(this, "success", "����ɹ�,��Ĭ�ϱ���Ϊpng�ļ�");
        return ui.customPlot->savePng(fileName.append(".png"), ui.customPlot->width(), ui.customPlot->height());
    }
}

// ���汾�α���ͼƬ��·��
void OpenFileDialog::saveFilePath(QString fileFullName) {
    QFileInfo fileinfo = QFileInfo(fileFullName);
    //QString file_name = fileinfo.fileName();//�ļ�����
    //QString file_suffix = fileinfo.suffix();//�ļ���׺��ʽ
    QString file_path = fileinfo.absolutePath();//�ļ�����·��

    QJsonObject jsonSetting = mainWindow::ReadSetting();
    jsonSetting["SavePictureDir"] = file_path;
    mainWindow::WriteSetting(jsonSetting);
}

// ѡ�����߹�����ͣ���/�����ʮ�ֹ��/����ȡֵ���
// ����ǰ����com_index_stringֵ�����ı��򴥷��˺���
void OpenFileDialog::on_GetData_comboBox_currentIndexChanged(const QString& arg1)
{
    //����ǰѡ������ֵ������str�������ǰѡ����
    QString str = ui.GetData_comboBox->currentText();
    qDebug() << "Text:" << str;
    if (str == "��") {
        mTracer = TracerFlag::NoTracer;
        // ɾ��������ر�����ָ���ÿ�
        if (tracerCross != Q_NULLPTR) {
            delete tracerCross;
            tracerCross = Q_NULLPTR;
        }
        for (int i = 0; i < 4; i++) {
            if (tracerX[i] != Q_NULLPTR) {
                delete tracerX[i];
                tracerX[i] = Q_NULLPTR;
            }
        }
        if (lineTracer != Q_NULLPTR) {
            delete lineTracer;
            lineTracer = Q_NULLPTR;
        }
        disconnect(pPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(SLOT_mouseTracetoCoord(QMouseEvent*)));
    }
    if (str == "ʮ�ֹ��") {
        mTracer = TracerFlag::CrossTracer;
        tracerCross = new myTracer(pPlot, pGraph1, DataTracer);
        if (lineTracer != Q_NULLPTR) {
            delete lineTracer;
            lineTracer = Q_NULLPTR;
        }
        for (int i = 0; i < 4; i++) {
            if (tracerX[i] != Q_NULLPTR) {
                delete tracerX[i];
                tracerX[i] = Q_NULLPTR;
            }
        }
        lineTracer = new myTracerLine(pPlot, myTracerLine::Both);//��ʮ�ֽ�����
        connect(pPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(SLOT_mouseTracetoCoord(QMouseEvent*)));
    }
    if (str == "����ȡֵ") {
        mTracer = TracerFlag::CurveTracer;
        if (tracerCross != Q_NULLPTR) {
            delete tracerCross;
            tracerCross = Q_NULLPTR;
        }
        if (lineTracer != Q_NULLPTR) {
            delete lineTracer;
            lineTracer = Q_NULLPTR;
        }
        ////����׷������
        //for (int i = 0; i < 4; i++) {
        //    plottracer[i] = new QCPItemTracer(pPlot1);
        //    plottracer[i]->setGraph(pPlot1->graph(i));
        //    //����ʮ�ָ�����ʽ
        //    QPen pen = pPlot1->graph(i)->pen();
        //    pen.setStyle(Qt::SolidLine);//
        //    plottracer[i]->setPen(pen);
        //}
        for (int i = 0; i < 4; i++) {
            tracerX[i] = new myTracer(pPlot, pPlot->graph(i), DataTracer);
        }
        lineTracer = new myTracerLine(pPlot, myTracerLine::VerticalLine);//����ֱ��
        connect(pPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(SLOT_mouseTracetoCoord(QMouseEvent*)));
    }

    pPlot->replot();
}

// ����ȡֵ,�����ȡֵ
void OpenFileDialog::DoCurveTracer(QMouseEvent* event)
{
    //ֱ�߷�Χ����
    double xLow = pPlot->xAxis->range().lower;
    double yLow = pPlot->yAxis->range().lower;
    double xUp = pPlot->xAxis->range().upper;
    double yUp = pPlot->yAxis->range().upper;

    //��ȡ����,��������λ�ã���������x���ֵ
    int x_pos = event->pos().x();

    //���������ֵ��������x���ֵ
    int x_value = round(pPlot->xAxis->pixelToCoord(x_pos));

    for (int i = pPlot->graphCount() - 1; i >= 0; --i)
    {
        // ��ȡx��ֵ��Ӧ�������е�y��ֵ
        float y_value = pPlot->graph(i)->data()->at(x_value)->value;
        //�����ǩ��ʽ
        QString tip;
        if (x_value > xLow && x_value<xUp && y_value>yLow && y_value < yUp) {   // ֱ�ߡ��α귶Χ����
            lastPos = x_value; //��¼���ε��α�ȡֵλ��
            lineTracer->updatePosition(x_value, y_value); //ֻ��Ҫ����һ��ֱ��
            tracerX[i]->updatePosition(x_value, y_value);
            lineTracer->setVisible(true);
            tracerX[i]->setVisible(true);
            //�����ǩ��ʽ
            QString tip;
            tip = QString::number(x_value) + "," + QString::number(y_value);
            tracerX[i]->setText(tip);
        }
        else {
            lineTracer->setVisible(false);
            tracerX[i]->setVisible(false);
        }

        //��������
        pPlot->replot(QCustomPlot::rpQueuedReplot);
    }
}

// ����ȡֵ��Ӧ�������Ҽ��ƶ�
void OpenFileDialog::DoCurveTracer(KeyboardType type){
    // ȷ��ƫ����
    int offset = 0;
    if (type == leftMove) offset = -1;
    if (type == rightMove) offset = 1;

    // ֱ�߷�Χ����
    double xLow = pPlot->xAxis->range().lower;
    double yLow = pPlot->yAxis->range().lower;
    double xUp = pPlot->xAxis->range().upper;
    double yUp = pPlot->yAxis->range().upper;

    // ��¼�ϴε��α�λ�ã�������һ��ƫ����
    int x_value = lastPos + offset;
    for (int i = pPlot->graphCount() - 1; i >= 0; --i)
    {
        // ��ȡx��ֵ��Ӧ�������е�y��ֵ
        float y_value = pPlot->graph(i)->data()->at(x_value)->value;
        //�����ǩ��ʽ
        QString tip;
        if (x_value > xLow && x_value<xUp && y_value>yLow && y_value < yUp) {   // ֱ�ߡ��α귶Χ����
            lastPos = x_value; // ��¼�µĹ��λ��
            lineTracer->updatePosition(x_value, y_value); //ֻ��Ҫ����һ��ֱ��
            tracerX[i]->updatePosition(x_value, y_value);
            lineTracer->setVisible(true);
            tracerX[i]->setVisible(true);
            //�����ǩ��ʽ
            QString tip;
            tip = QString::number(x_value) + "," + QString::number(y_value);
            tracerX[i]->setText(tip);
        }
        else {
            lineTracer->setVisible(false);
            tracerX[i]->setVisible(false);
        }

        //��������
        pPlot->replot(QCustomPlot::rpQueuedReplot);
    }
}

// ʮ�ּ�ȡֵ
void OpenFileDialog::DoCrossTracer(QMouseEvent* event){
    //ֱ�߷�Χ����
    double xLow = pPlot->xAxis->range().lower;
    double yLow = pPlot->yAxis->range().lower;
    double xUp = pPlot->xAxis->range().upper;
    double yUp = pPlot->yAxis->range().upper;

    double x = pPlot->xAxis->pixelToCoord(event->pos().x());
    double y2 = pPlot->yAxis->pixelToCoord(event->pos().y());

    if (x > xLow && x<xUp && y2>yLow && y2 < yUp) {   //ֱ�ߡ��α귶Χ����
        lineTracer->updatePosition(x, y2);
        tracerCross->updatePosition(x, y2);
        lineTracer->setVisible(true);
        tracerCross->setVisible(true);
        //�����ǩ��ʽ
        QString tip;
        tip = QString::number(x, 'f', 2) + "," + QString::number(y2, 'f', 2);
        tracerCross->setText(tip);
    }
    else {
        lineTracer->setVisible(false);
        tracerCross->setVisible(false);
    }

    //��������
    pPlot->replot(QCustomPlot::rpQueuedReplot);
}

// ���������ͼ��ȡֵ
void OpenFileDialog::SLOT_mouseTracetoCoord(QMouseEvent* event)
{
    switch (mTracer) {
    case TracerFlag::CrossTracer: {
        DoCrossTracer(event);
        break;
    }
    case TracerFlag::CurveTracer: {
        DoCurveTracer(event);
        break;
    }
    case TracerFlag::NoTracer:
        break;
    default:
        break;
    }
}