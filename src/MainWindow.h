#pragma once
#include "ui_MainWindow.h"
#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>

class MainWindow : public QMainWindow {
	Q_OBJECT

public:
	MainWindow(QWidget* parent = nullptr);
	~MainWindow();

private:
	Ui_MainWindow* _ui;
	QFileInfoList _filesInfo;  // 文件信息
	QString _errorMess;		   // 错误信息
	enum NumType { GENERAL,
				   SHORT_LINE,
				   UNDER_LINE };  // 追加序号的类型
	enum AppendType { PREFIX,
					  SUFFIX };	 // 追加类型

	void start();													 // 确定按钮的执行函数
	void removeFiles();												 // 移除选中的文件
	void printErrorMess();											 // 打印错误信息
	void updateFilesList();											 // 更新 UI 界面中的文件列表
	bool partReplace(QString find, QString replace, bool isIgnore);	 // 部分替换的实现
	bool numReplace(int zeroNum, int beginNum, int interval);		 // 序号替换的实现
	bool diyAppend(QString append, AppendType type);				 // 自定义追加的实现
	bool numAppend(NumType numType);								 // 序号追加的实现
};