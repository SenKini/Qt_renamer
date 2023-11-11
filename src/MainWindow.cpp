#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent), _ui(new Ui_MainWindow) {
	_ui->setupUi(this);

	/* 界面切换 */
	connect(_ui->numReplaceAction, &QAction::triggered, this, [this] {
		_ui->mainStackedWidget->setCurrentIndex(0);
	});	 // 切换到序号替换界面
	connect(_ui->partReplaceAction, &QAction::triggered, this, [this] {
		_ui->mainStackedWidget->setCurrentIndex(1);
	});	 // 切换到部分替换界面
	connect(_ui->diyAppendAction, &QAction::triggered, this, [this] {
		_ui->mainStackedWidget->setCurrentIndex(2);
	});	 // 切换到自定义追加界面
	connect(_ui->numAppendAction, &QAction::triggered, this, [this] {
		_ui->mainStackedWidget->setCurrentIndex(3);
	});	 // 切换到序号追加界面

	/* 使向前追加和向后追加不能同时选中 */
	connect(_ui->prefixCheckBox, &QCheckBox::clicked, this, [this] {
		_ui->suffixCheckBox->setChecked(false);
	});
	connect(_ui->suffixCheckBox, &QCheckBox::clicked, this, [this] {
		_ui->prefixCheckBox->setChecked(false);
	});

	/* 其他小功能 */
	_ui->filesListWidget->setSelectionMode(QAbstractItemView::MultiSelection);	// 将 filesListWidget 设置为可多选
	connect(_ui->seleteFileAction, &QAction::triggered, this, [this] {
		QStringList filesPath = QFileDialog::getOpenFileNames(this, "选择文件");

		for (auto path : filesPath)
			_filesInfo.append(QFileInfo(path));
		updateFilesList();
	});	 // 选择多个新文件，并显示到列表中
	connect(_ui->deletePushButton, &QPushButton::clicked,
			this, &MainWindow::removeFiles);  // 将文件移除文件列表
	connect(_ui->okPushButton, &QPushButton::clicked,
			this, &MainWindow::start);	// 连接开始按钮
}

MainWindow::~MainWindow() {
	delete _ui;
}

void MainWindow::start() {
	if (_filesInfo.empty()) {
		QMessageBox::critical(this, "错误", "没有选择文件");
		return;
	}

	int page = _ui->mainStackedWidget->currentIndex();

	/* 判断当前页面 */
	if (page == 0) {  // 序号替换界面
		int zeroNum = _ui->zeroNumSpinBox->value();
		int beginNum = _ui->beginNumSpinBox->value();
		int interval = _ui->numIntervalSpinBox->value();

		if (!numReplace(zeroNum, beginNum, interval))
			printErrorMess();
	}
	else if (page == 1) {  // 部分替换界面
		QString find = _ui->findLineEdit->text();
		QString replace = _ui->replaceLineEdit->text();
		bool isIgnore = _ui->IgnoreExCheckBox->isChecked();

		if (!partReplace(find, replace, isIgnore))
			printErrorMess();
	}
	else if (page == 2) {  // 自定义追加界面
		QString append = _ui->appendLineEdit->text();
		AppendType type = _ui->prefixCheckBox->isChecked() ? PREFIX : SUFFIX;

		if (!diyAppend(append, type))
			printErrorMess();
	}
	else if (page == 3) {  // 序号追加界面
		int index = _ui->numTypeComboBox->currentIndex();
		NumType numType;

		switch (index) {
			case 0:
				numType = GENERAL;
				break;
			case 1:
				numType = SHORT_LINE;
				break;
			case 2:
				numType = UNDER_LINE;
				break;
		}

		if (!numAppend(numType))
			printErrorMess();
	}
}

void MainWindow::removeFiles() {
	auto selectedFiles = _ui->filesListWidget->selectedItems();	 // 被选中的项目
	int tmpPos;													 // 临时变量，存储被选中的文件名在 _filesName 中的下标
	QStringList filesName;										 // 文件名

	/* 提取文件名 */
	for (auto info : _filesInfo)
		filesName.append(info.fileName());

	/* 删除选中的文件名 */
	for (auto item : selectedFiles) {
		tmpPos = filesName.indexOf(item->text());		// 获取对应文件名下标
		_filesInfo.erase(_filesInfo.begin() + tmpPos);	// 在 _filesName 中删除
	}

	updateFilesList();	// 更新文件列表
}

void MainWindow::printErrorMess() {
	QMessageBox::critical(this, "错误", _errorMess);
}

void MainWindow::updateFilesList() {
	QStringList filesName;	// 文件名

	/* 提取文件名 */
	for (auto info : _filesInfo)
		filesName.append(info.fileName());

	_ui->filesListWidget->clear();
	_ui->filesListWidget->addItems(filesName);
}

bool MainWindow::partReplace(QString find, QString replace, bool isIgnore) {
	QString tmpPath;	// 临时变量，存储文件路径
	QString tmpName;	// 临时变量，存储文件名
	QString tmpSuffix;	// 临时变量，存储后缀名
	QFile tmpFile;		// 临时变量，存储文件

	/* 对每个文件信息逐一更改 */
	for (auto &info : _filesInfo) {
		tmpFile.setFileName(info.absoluteFilePath());
		tmpPath = info.path();

		if (isIgnore) {	 // 根据 isIgnore 决定是否忽略后缀名
			tmpName = info.baseName();
			tmpSuffix = info.completeSuffix();

			tmpName.replace(find, replace);
			tmpPath = tmpPath + "/" + tmpName + "." + tmpSuffix;
			info.setFile(tmpPath);
		}
		else {
			tmpName = info.fileName();

			tmpName.replace(find, replace);
			tmpPath = tmpPath + "/" + tmpName;
			info.setFile(tmpPath);
		}

		if (!tmpFile.rename(tmpPath)) {
			_errorMess = "重命名失败：" + tmpFile.fileName();
			return false;
		}
	}

	updateFilesList();

	return true;
}

bool MainWindow::numReplace(int zeroNum, int beginNum, int interval) {
	QString tmpPath;	 // 临时变量，存储文件路径
	QString tmpSuffix;	 // 临时变量，存储后缀名
	QString tmpNum;		 // 临时变量，存储字符串形式的序号
	QFile tmpFile;		 // 临时变量，存储文件
	int num = beginNum;	 // 序号

	/* 对每个文件信息逐一更改 */
	for (auto &info : _filesInfo) {
		tmpNum = QString::number(num);
		tmpPath = info.path();
		tmpSuffix = info.completeSuffix();
		tmpFile.setFileName(info.absoluteFilePath());

		while (tmpNum.size() < zeroNum)
			tmpNum.push_front('0');

		tmpPath = tmpPath + "/" + tmpNum + "." + tmpSuffix;
		info.setFile(tmpPath);
		if (!tmpFile.rename(tmpPath)) {
			_errorMess = "重命名失败：" + tmpFile.fileName();
			return false;
		}

		num += interval;
	}

	updateFilesList();

	return true;
}

bool MainWindow::diyAppend(QString append, AppendType type) {
	QString tmpPath;  // 临时变量，存储文件路径
	QFile tmpFile;	  // 临时变量，存储文件
	int pos;		  // 追加的位置

	for (auto &info : _filesInfo) {
		tmpFile.setFileName(info.absoluteFilePath());
		tmpPath = info.absoluteFilePath();
		pos = (type == PREFIX) ? 0 : tmpPath.indexOf('.');	// 根据 type 决定追加类型是前缀还是后缀

		tmpPath.insert(pos, append);
		info.setFile(tmpPath);
		if (!tmpFile.rename(tmpPath)) {
			_errorMess = "重命名失败";
			return false;
		}
	}

	return true;
}

bool MainWindow::numAppend(NumType numType) {
	QString tmpPath;  // 临时变量，存储文件路径
	QFile tmpFile;	  // 临时变量，存储文件
	int pos;		  // 追加的位置
	QString prefix;	  // 序号前缀

	/* 根据 numType 决定序号前缀 */
	switch (numType) {
		case GENERAL:
			prefix = "";
			break;
		case SHORT_LINE:
			prefix = "-";
			break;
		case UNDER_LINE:
			prefix = "_";
			break;
	}

	for (int i = 0; i < _filesInfo.size(); i++) {
		tmpFile.setFileName(_filesInfo[i].absoluteFilePath());
		tmpPath = _filesInfo[i].absoluteFilePath();
		pos = tmpPath.indexOf('.');

		tmpPath.insert(pos, prefix + QString::number(i + 1));
		_filesInfo[i].setFile(tmpPath);
		if (!tmpFile.rename(tmpPath)) {
			_errorMess = "重命名失败" + tmpFile.fileName();
			return false;
		}
	}

	updateFilesList();

	return true;
}
