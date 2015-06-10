#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets/QMainWindow>
#include "ui_mainwindow.h"
#include <qsqldatabase.h>

#include <vector>
using std::vector;

class QTableWidget;
class QTableWidgetItem;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

public slots:
	void slotDouble(QTableWidgetItem* item);
	void slotChanged(QTableWidgetItem* item);
	void slotTabChanged(int index);


private:
	bool openDB();
	//��ʼ������
	void initWidget();
	//���ش���ֵ
	void loadWidget();
	void initRecord();

private:
	Ui::MainWindowClass ui;
	QSqlDatabase mSql;

	QString mCurItem;
	vector<QTableWidget*> mVecWidget;
	//��ǰ����
	int mIndex;
	int mRow;
	int mCol;
};

#endif // MAINWINDOW_H
