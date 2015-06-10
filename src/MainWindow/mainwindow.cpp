#include "mainwindow.h"
#include <qdatetime.h>
#include <qsqlerror.h>
#include <qsqlquery.h>
#include <qtablewidgetitem>
#include <qsettings>
#include <qmessagebox.h>

#include <string>
using std::string;

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	mCurItem = "";
	mIndex = 0;

	ui.setupUi(this);

	if(openDB())
	{
		initWidget();
		connect(ui.tabWidget,SIGNAL(currentChanged(int)), this ,SLOT(slotTabChanged(int)));
		loadWidget();
	}
}

MainWindow::~MainWindow()
{
	QSettings setting("system.ini", QSettings::IniFormat);
	setting.beginGroup("widget");
	setting.setValue("index", mIndex);	
	setting.setValue("row", mRow);	
	setting.setValue("col", mCol);	
	setting.endGroup();

	mSql.close();
}

void MainWindow::loadWidget()
{
	int index=0;
	QSettings setting("system.ini", QSettings::IniFormat);
	setting.beginGroup("widget");
	index = setting.value("index").toInt();
	mRow = setting.value("row").toInt();
	mCol = setting.value("col").toInt();
	setting.endGroup();

	if(index == 0)
	{
		//initRecord();	
		slotTabChanged(0);
	}
	else
	{
		ui.tabWidget->setCurrentIndex(index);
	}


	if(!(mRow==0 && mCol==0))
	{
		QTableWidget *tableWidget = mVecWidget[mIndex];
		tableWidget->setCurrentCell(mRow, mCol);
	}
}

void MainWindow::slotTabChanged(int index)
{
	//关闭槽
	QTableWidget *oldWidget = mVecWidget[mIndex];
	disconnect(oldWidget, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), 
		this, SLOT(slotDouble(QTableWidgetItem*)));
	disconnect(oldWidget, SIGNAL(itemChanged(QTableWidgetItem*)), 
	this, SLOT(slotChanged(QTableWidgetItem*)));

	//初始化记录
	mIndex = index;
	initRecord();

	//打开槽
	QTableWidget *tableWidget = mVecWidget[index];
	connect(tableWidget, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), 
		this, SLOT(slotDouble(QTableWidgetItem*)));
	connect(tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), 
	this, SLOT(slotChanged(QTableWidgetItem*)));
}

bool MainWindow::openDB()
{
	QString database, hostName, databaseName, user,password;
	int port;
	QSettings setting("system.ini", QSettings::IniFormat);
	setting.beginGroup("sql");
	database = setting.value("database").toString();
	hostName = setting.value("hostname").toString();
	databaseName = setting.value("databasename").toString();
	user = setting.value("user").toString();
	password = setting.value("password").toString();
	port = setting.value("port").toInt();
	setting.endGroup();

	string sdata = database.toStdString();
	mSql = QSqlDatabase::addDatabase(database);
	mSql.setHostName(hostName);  //设置主机地址
	mSql.setPort(port);  //设置端口
	mSql.setDatabaseName(databaseName);  //设置数据库名称
	if(!mSql.open(user, password))
	{
		QSqlError err = mSql.lastError();
		QMessageBox::warning(this, QStringLiteral("数据库连接失败"), QStringLiteral("重启软件或联系管理员！"),QStringLiteral("确定"));
		return false;
	}
	return true;
}

void MainWindow::initWidget()
{
	QString sSQL = "select emp_name from employee order by emp_id asc";
	QSqlQuery query;
	if(!query.exec(sSQL))
	{
		QSqlError err = query.lastError();
	}

	while(query.next())
	{
		QWidget *tab = new QWidget();
		QGridLayout *gridLayout = new QGridLayout(tab);
		QTableWidget *tableWidget = new QTableWidget(tab);
		mVecWidget.push_back(tableWidget);
		gridLayout->addWidget(tableWidget,0,0,1,1);
		ui.tabWidget->addTab(tab, query.value(0).toString());		
	}
}
void MainWindow::initRecord()
{
	QStringList headers; 

	int total = 1;
	QDate date(2015, 1, 1);
	headers<<"01-01,1W"<<QStringLiteral("加班");
	while(date.year() == 2015)
	{
		int day = date.dayOfWeek();
		if(day == 1)	
		{
			total++;
			headers<<date.toString("MM-dd,%1W").arg(total)<<QStringLiteral("加班");
			date = date.addDays(7);	
		}
		else
		{
			date = date.addDays(1);	
		}
		int year = date.year();
	}

	QTableWidget *tableWidget = mVecWidget[mIndex];
	tableWidget->setColumnCount(total*2); 
	tableWidget->setHorizontalHeaderLabels(headers);

	QSqlQuery query;
	QString sSQL = "select pro_name from project order by pro_id asc";
	if(!query.exec(sSQL))
	{
		QMessageBox::warning(this, QStringLiteral("加载列头失败"), QStringLiteral("重启软件或联系管理员！"),QStringLiteral("确定"));
	}
	int row = query.numRowsAffected();
	tableWidget->setRowCount(row+1);

	//加载第列头
	int record = 0;
	while(query.next())
	{
		QString sValue = query.value(0).toString();	

		QTableWidgetItem *item = new QTableWidgetItem(sValue);
		//item->setFlags(item->flags() & (~Qt::ItemIsEditable));
		//tableWidget->setItem(record, 0, item); 	
		tableWidget->setVerticalHeaderItem(record++,  item);
	}
	tableWidget->setVerticalHeaderItem(record,  new QTableWidgetItem(QStringLiteral("统计")));


	//加载值
	sSQL = QString("select pt_first,pt_pro_id,pt_time,pt_overtime from projecttime where pt_emp_id=%1 and pt_year=%2").arg(mIndex+1).arg(2015);
	if(!query.exec(sSQL))
	{
		QMessageBox::warning(this, QStringLiteral("初始值失败"), QStringLiteral("重启软件或联系管理员！"),QStringLiteral("确定"));
		//QSqlError err = query.lastError();
	}

	int pt_first, pt_pro_id;
	QString pt_time, pt_overtime;
	while(query.next())
	{
		pt_first = query.value(0).toInt();	
		pt_pro_id = query.value(1).toInt();
		pt_time = query.value(2).toString();
		pt_overtime = query.value(3).toString();

		tableWidget->setItem(pt_pro_id-1, (pt_first-1)*2, new QTableWidgetItem(pt_time)); 	
		tableWidget->setItem(pt_pro_id-1, (pt_first-1)*2+1, new QTableWidgetItem(pt_overtime)); 	

	}

	//统计
	sSQL = QString("select min(pt_first),sum(pt_time),sum(pt_overtime) from projecttime where pt_emp_id=%1 and pt_year=%2 group by pt_date")
		.arg(mIndex+1).arg(2015);
	if(!query.exec(sSQL))
	{
		QSqlError err = query.lastError();
	}

	while(query.next())
	{
		pt_first = query.value(0).toInt();	
		pt_time = query.value(1).toString();
		pt_overtime = query.value(2).toString();

		tableWidget->setItem(record, (pt_first-1)*2, new QTableWidgetItem(pt_time)); 	
		tableWidget->setItem(record, (pt_first-1)*2+1, new QTableWidgetItem(pt_overtime)); 	
	}


}

void MainWindow::slotDouble(QTableWidgetItem* item)
{
	mCurItem = item->text();
}

void MainWindow::slotChanged(QTableWidgetItem* item)
{
	mRow = item->row();
	mCol = item->column();

	QTableWidget *tableWidget = mVecWidget[mIndex];

	QString sVal = item->text();

		if(mCurItem != sVal)	
		{
			QString rowItem, pt_date, field;
			int pt_first = 0;
			//日期列
			if(mCol % 2 == 0)
			{
				rowItem = tableWidget->horizontalHeaderItem(mCol)->text();
				field = "pt_time";
			}//加班列
			else
			{
				rowItem = tableWidget->horizontalHeaderItem(mCol-1)->text();
				field = "pt_overtime";
			}
			pt_date = rowItem.mid(0,5);

			if(rowItem.length() > 8)
			{
			pt_first = rowItem.mid(6,2).toInt();
			}
			else
			{
			pt_first = rowItem.mid(6,1).toInt();
			}

			//select update or insert
			QString sSQL = QString("select 1 from projecttime where pt_pro_id=%1 and pt_emp_id=%2 and pt_year=%3 and pt_date='%4'")
				.arg(mRow+1).arg(mIndex+1).arg(2015).arg(pt_date);
			string s = sSQL.toStdString();

			QSqlQuery query;
			if(query.exec(sSQL))
			{
				int affRow = query.numRowsAffected();
				//无记录则插入
				if(affRow == 0)
				{//start

					sSQL = QString("insert into projecttime(pt_year, pt_date, pt_first, %1, pt_pro_id, pt_emp_id) "
						"values(%2,'%3',%4,%5,%6,%7)").arg(field).arg(2015).arg(pt_date).arg(pt_first).arg(sVal).arg(mRow+1).arg(mIndex+1);	
					string ss = sSQL.toStdString();

					if(!query.exec(sSQL))
					{
						//QSqlError err = query.lastError();	
						item->setText("0");
						QMessageBox::warning(this, QStringLiteral("保存记录失败"), QStringLiteral("请填写数字！"),QStringLiteral("确定"));
						return;
					}


				}//end,否则更新记录
				else if(affRow == 1)
				{
					if(mCol % 2 == 0)
					{
						field = "pt_time";
					}
					else
					{
						field = "pt_overtime";	
					}
					sSQL = QString("update projecttime set %1=%2 where pt_pro_id=%3 and pt_emp_id=%4 and pt_date='%5' and pt_year=%6")
						.arg(field).arg(sVal).arg(mRow+1).arg(mIndex+1).arg(pt_date).arg(2015);	
					string s = sSQL.toStdString();

					if(!query.exec(sSQL))
					{
						item->setText("0");
						//QSqlError err = query.lastError();	
						QMessageBox::warning(this, QStringLiteral("更新记录失败"), QStringLiteral("请填写数字!"),QStringLiteral("确定"));
						return;
					}

				}
				else
				{
					//记录不唯一	
					QMessageBox::warning(this, QStringLiteral("加载值失败"), QStringLiteral("重启软件或联系管理员！"),QStringLiteral("确定"));

				}

			}
		}
}

