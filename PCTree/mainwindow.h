/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QPointF>
#include <QGraphicsScene>
#include <QBrush>


enum nodeInfo
{
	RateZero,
	RateOne,
	RateHalf,
	RepetitionNode,
	SPCnode,
	RepSPCnode,
	RateR
};


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

signals:
	void parametersChanged();

private slots:
	void on_spinBox_valueChanged(int arg1);
	void updateTree();
	void on_doubleSpinBox_2_valueChanged(double arg1);
	void on_doubleSpinBox_valueChanged(double arg1);
	void on_spinBox_2_valueChanged(int arg1);

	void on_checkBox_toggled(bool checked);

private:

	QPointF getCoord(int layer, int element);
	QBrush getBrush(int nodeID);
	QBrush getBrushByCode(nodeInfo code);
	QSizeF ellipseSize;
	QPointF ellipseOffset;

	void addLines(int nodeID, int layer, int element, bool gray);
	void addNodes(int nodeID, int layer, int element, bool gray);

	void updateKBox();

	void updateScene();

	Ui::MainWindow *ui;

	int n, N, K;
	float R, designSNR;

	QVector<float> Z;
	QVector<nodeInfo> simplifiedTree;

	QGraphicsScene *Scene;
	float sceneWidth, sceneHeight;
	bool grayMode;
};

#endif // MAINWINDOW_H
