#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <cmath>
#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QTimer>

#include "ArrayFuncs.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	connect(this, SIGNAL(parametersChanged()), this, SLOT(updateTree()));

	Scene = new QGraphicsScene(this);
	ui->graphicsView->setScene(Scene);

	n = ui->spinBox->value();
	R = ui->doubleSpinBox_2->value();
	designSNR = ui->doubleSpinBox->value();
	grayMode = ui->checkBox->isChecked();

	N = 1<<n;
	K = N*R;
	updateKBox();

	ellipseSize = QSizeF(15, 15);
	ellipseOffset = QPointF(ellipseSize.width()/2, ellipseSize.height()/2);

	emit parametersChanged();
	QTimer::singleShot(200, this, SLOT(updateTree()));
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::updateKBox()
{
	ui->spinBox_2->setMaximum(N);
	ui->spinBox_2->setValue(K);
}

void MainWindow::updateTree()
{
	ui->label_info->setText(QStringLiteral("N=")+QString::number(N));

	Z.resize(N);
	simplifiedTree.resize((N<<1) - 1);
	float designSNRlin = pow(10.0, designSNR/10.0);
	Z[0] = exp(-designSNRlin);

	float T; int B;

	for(int lev=n-1; lev >= 0; --lev)
	{
		B = 1<<lev;//pow(2, lev);
		for(int j = 0; j < N; j+=(B<<1))
		{
			T = Z[j];
			Z[j+B] = T*T;
			Z[j] = 2.0*T - Z[j+B];
		}
	}

	trackingSorter sorter;
	sorter.set(Z);
	sorter.stableSortDescending();

	for(int i=0; i<N-K; ++i)
	{
		simplifiedTree[N-1+sorter.permuted[i]] = RateZero;
	}

	for(int i=N-K; i<N; ++i)
	{
		simplifiedTree[N-1+sorter.permuted[i]] = RateOne;
	}

	for(int lev=n-1; lev>=0; --lev)
	{
		int st = (1<<lev)-1;
		int ed = (1<<(lev+1))-1;
		int idx, ctr;
		for(idx=st, ctr=ed; idx<ed; ++idx)
		{
			nodeInfo Left  = simplifiedTree[ctr++];
			nodeInfo Right = simplifiedTree[ctr++];
			if(Left == RateZero && Right == RateZero)
			{
				simplifiedTree[idx] = RateZero;
			}
			else if(Left == RateOne && Right == RateOne)
			{
				simplifiedTree[idx] = RateOne;
			}
			else if((Left == RateHalf || Left == SPCnode) && Right == RateOne/* && lev >= n-2*/)
			{
				simplifiedTree[idx] = SPCnode;
			}
			else if(Left == RateZero && (Right == RateHalf || Right == RepetitionNode))
			{
				simplifiedTree[idx] = RepetitionNode;
			}
			else if(Left == RateZero && Right == RateOne && lev == n-1)
			{
				simplifiedTree[idx] = RateHalf;
			}
			else if(Left == RepetitionNode && Right == SPCnode && lev == n-3)
			{
				simplifiedTree[idx] = RepSPCnode;
			}
			else
			{
				simplifiedTree[idx] = RateR;
			}
		}
	}
	updateScene();
}

void MainWindow::updateScene()
{
	Scene->clear();

	{//Set scener64ect
	QRect graphicsRect = ui->graphicsView->rect();
	graphicsRect.setWidth(graphicsRect.width()-2);
	graphicsRect.setHeight(graphicsRect.height()-2);
	Scene->setSceneRect(graphicsRect);
	}

	sceneWidth = Scene->width();
	sceneHeight = Scene->height();

	//Draw subcodes recursively
	if(grayMode)
	{
		addLines(0, 0, 0, simplifiedTree[0] != RateR);
		addNodes(0, 0, 0, simplifiedTree[0] != RateR);
	}
	else if(simplifiedTree[0] == RateR)
	{
		addLines(0, 0, 0, false);
		addNodes(0, 0, 0, false);
	}

	//Draw root node
	QPointF coord = getCoord(0, 0) - ellipseOffset;
	QRectF rect(coord, ellipseSize);
	Scene->addEllipse(rect, QPen(), getBrush(0));

	//Draw SIMD hints
	QGraphicsSimpleTextItem *item;
	for(int layer=0; layer <= n; ++layer)
	{
		QPointF coord = getCoord(layer, 0);
		coord.setX(5);
		QString string = QString::number(1<<(n-layer));
		if(layer == n) string += QStringLiteral(" bit");
		else string += QStringLiteral(" bits");
		item = Scene->addSimpleText(string);
		item->moveBy(coord.x(), coord.y());
	}

	//Draw legend
	int margin = 5;
	coord = QPointF(margin, margin);

	//Rate zero
	rect = QRectF(coord, ellipseSize);
	Scene->addEllipse(rect, QPen(), getBrushByCode(RateZero));
	coord.rx() += ellipseSize.width() + margin;
	item = Scene->addSimpleText("Rate-0");
	item->moveBy(coord.x(), coord.y()-margin/2);
	coord.rx() += item->boundingRect().width() + 2*margin;

	//Rate one
	rect = QRectF(coord, ellipseSize);
	Scene->addEllipse(rect, QPen(), getBrushByCode(RateOne));
	coord.rx() += ellipseSize.width() + margin;
	item = Scene->addSimpleText("Rate-1");
	item->moveBy(coord.x(), coord.y()-margin/2);
	coord.rx() += item->boundingRect().width() + 2*margin;

	//Rate R
	rect = QRectF(coord, ellipseSize);
	Scene->addEllipse(rect, QPen(), getBrushByCode(RateR));
	coord.rx() += ellipseSize.width() + margin;
	item = Scene->addSimpleText("Unspecified");
	item->moveBy(coord.x(), coord.y()-margin/2);
	coord.rx() += item->boundingRect().width() + 2*margin;

	//Repetition
	rect = QRectF(coord, ellipseSize);
	Scene->addEllipse(rect, QPen(), getBrushByCode(RepetitionNode));
	coord.rx() += ellipseSize.width() + margin;
	item = Scene->addSimpleText("Repetition");
	item->moveBy(coord.x(), coord.y()-margin/2);
	coord.rx() += item->boundingRect().width() + 2*margin;

	//RepSPC
	rect = QRectF(coord, ellipseSize);
	Scene->addEllipse(rect, QPen(), getBrushByCode(RepSPCnode));
	coord.rx() += ellipseSize.width() + margin;
	item = Scene->addSimpleText("Rep/SPC");
	item->moveBy(coord.x(), coord.y()-margin/2);
	coord.rx() += item->boundingRect().width() + 2*margin;

	//SPC
	rect = QRectF(coord, ellipseSize);
	Scene->addEllipse(rect, QPen(), getBrushByCode(SPCnode));
	coord.rx() += ellipseSize.width() + margin;
	item = Scene->addSimpleText("Single Parity Check");
	item->moveBy(coord.x(), coord.y()-margin/2);
	coord.rx() += item->boundingRect().width() + 2*margin;

	ui->graphicsView->update();
}

void MainWindow::addLines(int nodeID, int layer, int element, bool gray)
{
	if(gray && !grayMode) return;

	int leftNode = (nodeID<<1)+1, rightNode = leftNode+1;
	int leftElem = element<<1, rightElem = leftElem+1;

	QLineF	leftLine( getCoord(layer, element), getCoord(layer+1, leftElem)),
			rightLine(getCoord(layer, element), getCoord(layer+1, rightElem));

	QPen pen = QPen();

	if(gray) pen.setColor(Qt::lightGray);
	else	 pen.setWidth(2);

	Scene->addLine(leftLine, pen);
	Scene->addLine(rightLine, pen);

	if(layer == n-1) return;

	if(gray)
	{
		addLines(leftNode, layer+1, leftElem, true);
		addLines(rightNode, layer+1, rightElem, true);
	}
	else
	{
		addLines(leftNode, layer+1, leftElem, simplifiedTree[leftNode] != RateR);
		addLines(rightNode, layer+1, rightElem, simplifiedTree[rightNode] != RateR);
	}
}

void MainWindow::addNodes(int nodeID, int layer, int element, bool gray)
{
	if(gray && !grayMode) return;

	int leftNode = (nodeID<<1)+1, rightNode = leftNode+1;
	int leftElem = element<<1, rightElem = leftElem+1;

	QPointF leftCoord  = getCoord(layer+1, leftElem) - ellipseOffset,
			rightCoord = getCoord(layer+1, rightElem) - ellipseOffset;

	QRectF	leftEllipse(leftCoord, ellipseSize),
			rightEllipse(rightCoord, ellipseSize);

	QPen pen = QPen();

	if(gray) pen.setColor(Qt::lightGray);

	Scene->addEllipse(leftEllipse, pen, getBrush(leftNode));
	Scene->addEllipse(rightEllipse, pen, getBrush(rightNode));

	if(layer == n-1) return;

	if(gray)
	{
		addNodes(leftNode, layer+1, leftElem, true);
		addNodes(rightNode, layer+1, rightElem, true);
	}
	else
	{
		addNodes(leftNode, layer+1, leftElem, simplifiedTree[leftNode] != RateR);
		addNodes(rightNode, layer+1, rightElem, simplifiedTree[rightNode] != RateR);
	}
}

QBrush MainWindow::getBrush(int nodeID)
{
	return getBrushByCode(simplifiedTree[nodeID]);
}

QBrush MainWindow::getBrushByCode(nodeInfo code)
{
	QBrush brush = QBrush();
	brush.setStyle(Qt::SolidPattern);

	switch(code)
	{
	case RateZero:
		brush.setColor(Qt::white);
		break;
	case RateOne:
		brush.setColor(Qt::black);
		break;
	case RateHalf:
	case RepetitionNode:
		brush.setColor(Qt::green);
		brush.setStyle(Qt::Dense5Pattern);
		break;
	case SPCnode:
		brush.setColor(Qt::darkRed);
		brush.setStyle(Qt::Dense1Pattern);
		break;
	case RepSPCnode:
		brush.setColor(Qt::darkCyan);
		brush.setStyle(Qt::Dense3Pattern);
		break;
	case RateR:
		brush.setColor(Qt::gray);
		break;
	default:
		brush.setColor(Qt::red);
	}

	return brush;
}

QPointF MainWindow::getCoord(int layer, int element)
{
	float nElements = (1<<layer)+1;
	float columnWidth = sceneWidth/nElements, columnOffset = columnWidth;
	float rowHeight = sceneHeight/(n+2), rowOffset = rowHeight;

	return QPointF(columnOffset + element*columnWidth, rowOffset + layer*rowHeight);
}

void MainWindow::on_spinBox_valueChanged(int arg1)
{
	n = arg1;
	N=1<<n;
	K=N*R;
	updateKBox();
	emit parametersChanged();
}

void MainWindow::on_doubleSpinBox_2_valueChanged(double arg1)
{
	R = arg1;
	K=N*R;
	bool old = ui->spinBox_2->blockSignals(true);
	updateKBox();
	ui->spinBox_2->blockSignals(old);
	emit parametersChanged();
}

void MainWindow::on_doubleSpinBox_valueChanged(double arg1)
{
	designSNR = arg1;
	emit parametersChanged();
}

void MainWindow::on_spinBox_2_valueChanged(int arg1)
{
	R = K = arg1;
	R/=N;
	bool old = ui->doubleSpinBox_2->blockSignals(true);
	ui->doubleSpinBox_2->setValue(R);
	ui->doubleSpinBox_2->blockSignals(old);
	emit parametersChanged();
}

void MainWindow::on_checkBox_toggled(bool checked)
{
	grayMode = checked;
	updateScene();
}
