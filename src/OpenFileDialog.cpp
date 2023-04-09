#include "OpenFileDialog.h"
#include <QFile>

#include <QDoubleValidator>

OpenFileDialog::OpenFileDialog(QString fileName, QWidget *parent)
	: myFileName(fileName), QDialog(parent)
{
	ui.setupUi(this);
    setWindowTitle(fileName);

    // 限定输入为浮点数
    QDoubleValidator* doubleValidator = new QDoubleValidator();//QDoubleValidator 限定输入浮点数 ，QIntValidator – 只让用户输入整数
    ui.XAxis_left_Edit->setValidator(doubleValidator);
    ui.XAxis_right_Edit->setValidator(doubleValidator);
    ui.YAxis_left_Edit->setValidator(doubleValidator);
    ui.YAxis_right_Edit->setValidator(doubleValidator); 

    // 给控件安装事件过滤器
    ui.XAxis_left_Edit->installEventFilter(this);  
    ui.XAxis_right_Edit->installEventFilter(this); 
    ui.YAxis_left_Edit->installEventFilter(this);  
    ui.YAxis_right_Edit->installEventFilter(this);
    ui.customPlot->installEventFilter(this);
    
    pPlot = ui.customPlot; // 给customPlot绘图控件，设置个别名，方便书写

    // 曲线光标相关变量
    mTracer = TracerFlag::NoTracer;
    tracerCross = Q_NULLPTR;
    lineTracer = Q_NULLPTR;
    for (int i = 0; i < 4; i++) {
        tracerX[i] = Q_NULLPTR;
    }
    lastPos = 0;

    // 读取文件数据
	ReadFile();
	// 初始化图表
	QPlot_init(pPlot);
    // 绘制图片
    ShowPlot();
}

OpenFileDialog::~OpenFileDialog()
{
	delete this; // 该窗口为非模态框
}

// 读取数据
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

// 绘图图表初始化
void OpenFileDialog::QPlot_init(QCustomPlot* customPlot)
{
    // 图表添加两条曲线
    pGraph1 = customPlot->addGraph();
    pGraph2 = customPlot->addGraph();
    pGraph3 = customPlot->addGraph();
    pGraph4 = customPlot->addGraph();

    //ui->checkBox1->setCheckState(Qt::Checked);  //设置复选框初始状态 Unchecked
    //ui->checkBox2->setCheckState(Qt::Checked);
    //ui->checkBox3->setCheckState(Qt::Checked);
    //ui->checkBox4->setCheckState(Qt::Checked);
    //ui->rescaleAxesCheckBox->setCheckState(Qt::Checked); // 坐标轴自适应
    //ui->refreshPlotCheckBox->setCheckState(Qt::Checked); // 图像刷新

    // 设置曲线颜色
    pGraph1->setPen(QPen(Qt::red));
    pGraph2->setPen(QPen(Qt::darkRed));
    pGraph3->setPen(QPen(Qt::green));
    pGraph4->setPen(QPen(Qt::blue));

    // 设置坐标轴名称
    customPlot->xAxis->setLabel("时间/s");
    customPlot->yAxis->setLabel("计数率/cps");

    // 设置y坐标轴显示范围
    customPlot->xAxis->setRange(0, 120);

    // 显示图表的图例
    customPlot->legend->setBrush(QColor(255, 255, 255, 0));//legend背景色设为白色但背景透明，允许图像在legend区域可见
    customPlot->legend->setVisible(true);

    // 添加曲线名称
    pGraph1->setName("探测器1");
    pGraph2->setName("探测器2");
    pGraph3->setName("探测器3");
    pGraph4->setName("探测器4");

    // 设置波形曲线的复选框字体颜色
    //ui->checkBox1->setStyleSheet("QCheckBox{color:red}");//设定前景颜色,就是字体颜色
    //ui->checkBox2->setStyleSheet("QCheckBox{color:darkRed}");
    //ui->checkBox3->setStyleSheet("QCheckBox{color:green}");
    //ui->checkBox4->setStyleSheet("QCheckBox{color:blue}");

    // 允许用户用鼠标拖动轴范围，用鼠标滚轮缩放，点击选择图形:
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    //iRangeDrag 左键点击可拖动; iRangeZoom 范围可通过鼠标滚轮缩放; iSelectPlottables 线条可选中

    // 鼠标双击复原
    connect(ui.customPlot, SIGNAL(mouseDoubleClick(QMouseEvent*)), this, SLOT(DoubleClick())); //读取信息的连接

    // 鼠标拖动，获取坐标轴范围
    connect(ui.customPlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(myMoveMouse()));
    /*connect(ui.customPlot, &QCustomPlot::mouseDoubleClick, this, [=] {
        //ui.customPlot->xAxis->setRange(-100, 1000);
        //ui.customPlot->yAxis->setRange(-100, 100);
        pGraph1->rescaleAxes(); // 让范围自行缩放，使图0完全适合于可见区域.这里不能带参数true
        pGraph2->rescaleAxes(true); // 图1也是一样自动调整范围，但只是放大或不变范围
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

    // 自动调节坐标轴
    //if (RescaleAxesFlag) {
    pGraph1->rescaleAxes(); // 让范围自行缩放，使图0完全适合于可见区域.这里不能带参数true
    pGraph2->rescaleAxes(true); // 图1也是一样自动调整范围，但只是放大或不变范围
    pGraph3->rescaleAxes(true);
    pGraph4->rescaleAxes(true);
    //}
    pPlot->replot(QCustomPlot::rpQueuedReplot);
    
    UpdateAxisRange();
}

// 响应编辑框动作，当光标离开编辑框时，图像跟随响应，重新绘制
// 事件过滤器
// Qt::Key_Left = 0x01000012,
// Qt::Key_Up = 0x01000013,
// Qt::Key_Right = 0x01000014,
// Qt::Key_Down = 0x01000015,
bool OpenFileDialog::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == ui.XAxis_left_Edit){
        if (event->type() == QEvent::KeyPress && 
            ((QKeyEvent*)event)->key() == Qt::Key_Enter){ //按键Enter按下
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
            ((QKeyEvent*)event)->key() == Qt::Key_Enter) { //按键Enter按下
            setAxisRange();
        }
    }
    if (watched == ui.YAxis_right_Edit) {
        if ((event->type() == QEvent::FocusOut) || (event->type() == QEvent::KeyPress)){
            setAxisRange();
        }
    }
    if (mTracer == TracerFlag::CurveTracer) { //若存在曲线光标则响应键盘录入动作
        if (watched == ui.customPlot && event->type() == QEvent::KeyPress) {
            if ( ((QKeyEvent*)event)->key() == Qt::Key_Left || 
                ((QKeyEvent*)event)->key() == Qt::Key_Down) { //按键键盘左方向键按下
                DoCurveTracer(leftMove);
            }
            else if (((QKeyEvent*)event)->key() == Qt::Key_Right || 
                ((QKeyEvent*)event)->key() == Qt::Key_Up) { //按键键盘右方向键按下
                DoCurveTracer(rightMove);
            }
        }
    }
    return QDialog::eventFilter(watched, event);
}

// 根据界面输入框设置坐标轴的范围
void OpenFileDialog::setAxisRange() {
    double xMin = ui.XAxis_left_Edit->text().toDouble();
    double xMax = ui.XAxis_right_Edit->text().toDouble();
    double yMin = ui.YAxis_left_Edit->text().toDouble();
    double yMax = ui.YAxis_right_Edit->text().toDouble();

    if (xMin > xMax) {
        QMessageBox::information(this, "坐标轴范围设置失败", "请确保输入的左边界小于右边界");
        return;
    }
    if (yMin > yMax) {
        QMessageBox::information(this, "坐标轴范围设置失败", "请确保输入的左边界小于右边界");
        return;
    }

    pPlot->xAxis->setRange(ui.XAxis_left_Edit->text().toDouble(), ui.XAxis_right_Edit->text().toDouble());
    pPlot->yAxis->setRange(ui.YAxis_left_Edit->text().toDouble(), ui.YAxis_right_Edit->text().toDouble());
    pPlot->replot(QCustomPlot::rpQueuedReplot);
}

// 鼠标左键双击时重绘，重新调整坐标轴范围
void OpenFileDialog::DoubleClick() {
    pGraph1->rescaleAxes(); // 让范围自行缩放，使图0完全适合于可见区域.这里不能带参数true
    pGraph2->rescaleAxes(true); // 图1也是一样自动调整范围，但只是放大或不变范围
    pGraph3->rescaleAxes(true);
    pGraph4->rescaleAxes(true);
    ui.customPlot->replot();

    UpdateAxisRange();
}

// 鼠标拖动画面时，跟随变化坐标轴范围
void OpenFileDialog::myMoveMouse() {
    UpdateAxisRange();
} 

// 刷新坐标轴范围的显示值与图像对应
void OpenFileDialog::UpdateAxisRange() {
    // 获取当前坐标轴范围
    double left = ui.customPlot->xAxis->range().lower;
    double right = ui.customPlot->xAxis->range().upper;
    double down = ui.customPlot->yAxis->range().lower;
    double up = ui.customPlot->yAxis->range().upper;
    // 将当前坐标轴范围设置到文本框
    ui.XAxis_left_Edit->setText(QString::number(left, 'f', 1));
    ui.XAxis_right_Edit->setText(QString::number(right, 'f', 1));
    ui.YAxis_left_Edit->setText(QString::number(down, 'f', 1));
    ui.YAxis_right_Edit->setText(QString::number(up, 'f', 1));
}

// 绘制图形保存导出
bool OpenFileDialog::on_pbn_save_clicked()
{
    QJsonObject jsonSetting = mainWindow::ReadSetting();
    QString saveDir = jsonSetting["SavePictureDir"].toString(); //"SaveDir": "/home"
    QString wholePath = saveDir + "//Picture.bmp";

    QString fileName = QFileDialog::getSaveFileName(this, "图像另存为", wholePath, "BMP(*.bmp);; PNG(*.png);; JPG(*.jpg);; PDF(*.pdf)");
    //QString fileName = QFileDialog::getSaveFileName(this, "图像另存为", "test.bmp", "PNG(*.png);; JPG(*.jpg);; BMP(*.bmp);; PDF(*.pdf)");
    if (fileName == "") {
        QMessageBox::information(this, "fail", "保存失败");
        return false;
    }
    else if (fileName.endsWith(".png")) {
        saveFilePath(fileName);
        QMessageBox::information(this, "success", "成功保存为png文件");
        return ui.customPlot->savePng(fileName, ui.customPlot->width(), ui.customPlot->height());
    }
    else if (fileName.endsWith(".jpg") || fileName.endsWith(".jpeg")) {
        saveFilePath(fileName);
        QMessageBox::information(this, "success", "成功保存为jpg文件");
        return ui.customPlot->saveJpg(fileName, ui.customPlot->width(), ui.customPlot->height());
    }
    else if (fileName.endsWith(".bmp")) {
        saveFilePath(fileName);
        QMessageBox::information(this, "success", "成功保存为bmp文件");
        return ui.customPlot->saveBmp(fileName, ui.customPlot->width(), ui.customPlot->height());
    }
    else if (fileName.endsWith(".pdf")) {
        saveFilePath(fileName);
        QMessageBox::information(this, "success", "成功保存为pdf文件");
        return ui.customPlot->savePdf(fileName, ui.customPlot->width(), ui.customPlot->height());
    }
    else {
        //否则追加后缀名为.png保存文件
        QMessageBox::information(this, "success", "保存成功,已默认保存为png文件");
        return ui.customPlot->savePng(fileName.append(".png"), ui.customPlot->width(), ui.customPlot->height());
    }
}

// 保存本次保存图片的路径
void OpenFileDialog::saveFilePath(QString fileFullName) {
    QFileInfo fileinfo = QFileInfo(fileFullName);
    //QString file_name = fileinfo.fileName();//文件名称
    //QString file_suffix = fileinfo.suffix();//文件后缀格式
    QString file_path = fileinfo.absolutePath();//文件绝对路径

    QJsonObject jsonSetting = mainWindow::ReadSetting();
    jsonSetting["SavePictureDir"] = file_path;
    mainWindow::WriteSetting(jsonSetting);
}

// 选择曲线光标类型：无/任意点十字光标/曲线取值光标
// 若当前对象com_index_string值发生改变则触发此函数
void OpenFileDialog::on_GetData_comboBox_currentIndexChanged(const QString& arg1)
{
    //将当前选项名赋值给变量str，输出当前选项名
    QString str = ui.GetData_comboBox->currentText();
    qDebug() << "Text:" << str;
    if (str == "无") {
        mTracer = TracerFlag::NoTracer;
        // 删除浮标相关变量，指针置空
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
    if (str == "十字光标") {
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
        lineTracer = new myTracerLine(pPlot, myTracerLine::Both);//画十字交叉线
        connect(pPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(SLOT_mouseTracetoCoord(QMouseEvent*)));
    }
    if (str == "曲线取值") {
        mTracer = TracerFlag::CurveTracer;
        if (tracerCross != Q_NULLPTR) {
            delete tracerCross;
            tracerCross = Q_NULLPTR;
        }
        if (lineTracer != Q_NULLPTR) {
            delete lineTracer;
            lineTracer = Q_NULLPTR;
        }
        ////设置追踪曲线
        //for (int i = 0; i < 4; i++) {
        //    plottracer[i] = new QCPItemTracer(pPlot1);
        //    plottracer[i]->setGraph(pPlot1->graph(i));
        //    //设置十字浮标样式
        //    QPen pen = pPlot1->graph(i)->pen();
        //    pen.setStyle(Qt::SolidLine);//
        //    plottracer[i]->setPen(pen);
        //}
        for (int i = 0; i < 4; i++) {
            tracerX[i] = new myTracer(pPlot, pPlot->graph(i), DataTracer);
        }
        lineTracer = new myTracerLine(pPlot, myTracerLine::VerticalLine);//画垂直线
        connect(pPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(SLOT_mouseTracetoCoord(QMouseEvent*)));
    }

    pPlot->replot();
}

// 曲线取值,鼠标点击取值
void OpenFileDialog::DoCurveTracer(QMouseEvent* event)
{
    //直线范围限制
    double xLow = pPlot->xAxis->range().lower;
    double yLow = pPlot->yAxis->range().lower;
    double xUp = pPlot->xAxis->range().upper;
    double yUp = pPlot->yAxis->range().upper;

    //获取坐标,窗体鼠标的位置，不是曲线x轴的值
    int x_pos = event->pos().x();

    //将鼠标坐标值换成曲线x轴的值
    int x_value = round(pPlot->xAxis->pixelToCoord(x_pos));

    for (int i = pPlot->graphCount() - 1; i >= 0; --i)
    {
        // 获取x轴值对应的曲线中的y轴值
        float y_value = pPlot->graph(i)->data()->at(x_value)->value;
        //定义标签格式
        QString tip;
        if (x_value > xLow && x_value<xUp && y_value>yLow && y_value < yUp) {   // 直线、游标范围限制
            lastPos = x_value; //记录本次的游标取值位置
            lineTracer->updatePosition(x_value, y_value); //只需要绘制一次直线
            tracerX[i]->updatePosition(x_value, y_value);
            lineTracer->setVisible(true);
            tracerX[i]->setVisible(true);
            //定义标签格式
            QString tip;
            tip = QString::number(x_value) + "," + QString::number(y_value);
            tracerX[i]->setText(tip);
        }
        else {
            lineTracer->setVisible(false);
            tracerX[i]->setVisible(false);
        }

        //更新曲线
        pPlot->replot(QCustomPlot::rpQueuedReplot);
    }
}

// 曲线取值响应键盘左右键移动
void OpenFileDialog::DoCurveTracer(KeyboardType type){
    // 确定偏移量
    int offset = 0;
    if (type == leftMove) offset = -1;
    if (type == rightMove) offset = 1;

    // 直线范围限制
    double xLow = pPlot->xAxis->range().lower;
    double yLow = pPlot->yAxis->range().lower;
    double xUp = pPlot->xAxis->range().upper;
    double yUp = pPlot->yAxis->range().upper;

    // 记录上次的游标位置，并增加一个偏移量
    int x_value = lastPos + offset;
    for (int i = pPlot->graphCount() - 1; i >= 0; --i)
    {
        // 获取x轴值对应的曲线中的y轴值
        float y_value = pPlot->graph(i)->data()->at(x_value)->value;
        //定义标签格式
        QString tip;
        if (x_value > xLow && x_value<xUp && y_value>yLow && y_value < yUp) {   // 直线、游标范围限制
            lastPos = x_value; // 记录新的光标位置
            lineTracer->updatePosition(x_value, y_value); //只需要绘制一次直线
            tracerX[i]->updatePosition(x_value, y_value);
            lineTracer->setVisible(true);
            tracerX[i]->setVisible(true);
            //定义标签格式
            QString tip;
            tip = QString::number(x_value) + "," + QString::number(y_value);
            tracerX[i]->setText(tip);
        }
        else {
            lineTracer->setVisible(false);
            tracerX[i]->setVisible(false);
        }

        //更新曲线
        pPlot->replot(QCustomPlot::rpQueuedReplot);
    }
}

// 十字架取值
void OpenFileDialog::DoCrossTracer(QMouseEvent* event){
    //直线范围限制
    double xLow = pPlot->xAxis->range().lower;
    double yLow = pPlot->yAxis->range().lower;
    double xUp = pPlot->xAxis->range().upper;
    double yUp = pPlot->yAxis->range().upper;

    double x = pPlot->xAxis->pixelToCoord(event->pos().x());
    double y2 = pPlot->yAxis->pixelToCoord(event->pos().y());

    if (x > xLow && x<xUp && y2>yLow && y2 < yUp) {   //直线、游标范围限制
        lineTracer->updatePosition(x, y2);
        tracerCross->updatePosition(x, y2);
        lineTracer->setVisible(true);
        tracerCross->setVisible(true);
        //定义标签格式
        QString tip;
        tip = QString::number(x, 'f', 2) + "," + QString::number(y2, 'f', 2);
        tracerCross->setText(tip);
    }
    else {
        lineTracer->setVisible(false);
        tracerCross->setVisible(false);
    }

    //更新曲线
    pPlot->replot(QCustomPlot::rpQueuedReplot);
}

// 鼠标左键点击图像取值
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