#include <QtGui>
#include "mainwindow.h"
#include <QMessageBox>
#include <QTimer>
#include <QtWidgets>

const QString imagesDir = "images/";
const int INTERVAL = 10; // Интервал движения шахмат (меньше - быстрее)
const QString tempfileName = ".temp";

const int squareWidth  = 60;
const int squareHeight = 60;

const int boardWidth = 8;
const int boardHeight = 8;

const int boardMarginX = 40;
const int boardMarginY = 100;

int mouseX = -1;
int mouseY = -1;

int boardX = -1;
int boardY = -1;

bool isPressed = false;
int chessPressedX = -1;
int chessPressedY = -1;

bool isMovedNow = false;
int movedX = -1;
int movedY = -1;
int movedEndX = -1;
int movedEndY = -1;
int aX = -1;
int aY = -1;
QString movedChess = "";
QTimer *timer;

QVector<QVector<int> > chessQueque;
int nextChessN = -1;

bool isRecordNow = false;
QFile tempfile(tempfileName);

QString board[boardHeight][boardWidth] = {
	{"b5", "b2", "b0", "b4", "b1", "b0", "b2", "b5"},
	{"b3", "b3", "b3", "b3", "b3", "b3", "b3", "b3"},
	{"", "", "", "", "", "", "", ""},
	{"", "", "", "", "", "", "", ""},
	{"", "", "", "", "", "", "", ""},
	{"", "", "", "", "", "", "", ""},
	{"w3", "w3", "w3", "w3", "w3", "w3", "w3", "w3"},
	{"w5", "w2", "w0", "w4", "w1", "w0", "w2", "w5"}
};


MainWindow::MainWindow() {
	setWindowTitle(tr("Шахматы"));
	resize(560, 630);

	position = new QLabel(this);
	position->setGeometry(boardWidth * squareWidth + boardMarginX, boardHeight * squareHeight + boardMarginY, 100, 20);

	setMouseTracking(true);

	startRecordButton = new QPushButton("Начать запись", this);
	stopRecordButton = new QPushButton("Завершить запись", this);

	startRecordButton->setGeometry(boardMarginX, 10, 150, 50);
	stopRecordButton->setGeometry(boardMarginX, 10, 150, 50);
	stopRecordButton->setVisible(false);

	saveButton = new QPushButton("Сохранить в файл", this);
	openButton = new QPushButton("Открыть файл", this);

	saveButton->setGeometry(boardMarginX + 170, 10, 145, 50);
	openButton->setGeometry(boardMarginX + 170 + 165, 10, 145, 50);

	saveButton->setEnabled(false);

	connect(openButton, SIGNAL(clicked()), this, SLOT(openFile()));
	connect(saveButton, SIGNAL(clicked()), this, SLOT(saveFile()));
	connect(startRecordButton, SIGNAL(clicked()), this, SLOT(startRecord()));
	connect(stopRecordButton, SIGNAL(clicked()), this, SLOT(stopRecord()));

}

void MainWindow::openFile() {
	QString filename = QFileDialog::getOpenFileName(this, "Выберите файл для открытия", NULL, "Chess-file (*.chess)");
	if (!filename.isNull()) {
		startGame(filename);
	}
}

void MainWindow::saveFile() {
	QString filename = QFileDialog::getSaveFileName(this, "Выберите место сохранения файла", NULL, "Chess-file (*.chess)");
	if (!filename.isNull()) {
		
		if (filename.indexOf(".chess", 0, Qt::CaseInsensitive) == -1) {
			filename += ".chess";
		}

		if (QFile::copy(tempfileName, filename)) {
			QMessageBox::information(this, "Сохранено", "Файл сохранен в " + filename);			
		} else {
			QMessageBox::warning(this, "Ошибка сохранения", "Файл не сохранен");
		}

	}
}

void MainWindow::startRecord() {
	resetBoard();
	isRecordNow = true;

	stopRecordButton->setVisible(true);
	startRecordButton->setVisible(false);
	saveButton->setDisabled(true);
	openButton->setDisabled(true);

	if (!tempfile.open(QIODevice::WriteOnly)) {
		QMessageBox::warning(this, "Ошибка записи", "Временный файл недоступен");
		stopRecord();	
	}
}

void MainWindow::stopRecord() {
	isRecordNow = false;

	stopRecordButton->setVisible(false);
	startRecordButton->setVisible(true);
	saveButton->setDisabled(false);
	openButton->setDisabled(false);

	tempfile.close();
}

void MainWindow::startGame(QString filename) {
	saveButton->setEnabled(false);
	openButton->setEnabled(false);
	startRecordButton->setEnabled(false);
	stopRecordButton->setEnabled(false);

	resetBoard();

	QFile file(filename);
	if (file.open(QIODevice::ReadOnly)) {

		QString gameRaw;
		gameRaw = file.readAll();

		if (gameRaw.trimmed() != "") {
			
			QStringList items = gameRaw.split("\n");
			for (int i = 0; i < items.length(); i++) {
				QString item = items[i];
				if (item == "") {
					break;
				}
				QStringList oneItem = item.split(":");
				if (oneItem.length() != 2) {
					endGame("Ошибка воспроизведения - неверный формат файла");
					return;
				}

				QString before = oneItem[0];
				QString after = oneItem[1];

				QStringList beforePosition = before.split("-");
				QStringList afterPosition = after.split("-");

				if (beforePosition.length() != 2 || afterPosition.length() != 2) {
					endGame("Ошибка воспроизведения - неверный формат файла");
					return;
				}

				int beforeX = beforePosition[0].toInt();
				int beforeY = beforePosition[1].toInt();
				int afterX = afterPosition[0].toInt();
				int afterY = afterPosition[1].toInt();

				QVector<int> quequeItem;
				quequeItem.append(beforeX);
				quequeItem.append(beforeY);
				quequeItem.append(afterX);
				quequeItem.append(afterY);
				chessQueque.append(quequeItem);
			}
			
			nextChess(0);

		} else {
			endGame("Ошибка воспроизведения - файл пустой");
		}

		file.close();

	} else {
		endGame("Ошибка воспроизведения - не удалось открыть файл");
	}
}

void MainWindow::moveChess() {
	if (movedX != movedEndX) {
		if (movedX > movedEndX) {
			movedX -= 1;
		} else {
			movedX += 1;
		}
	}
	if (movedY != movedEndY) {
		if (movedY > movedEndY) {
			movedY -= 1;
		} else {
			movedY += 1;
		}
	}

	if (movedX == movedEndX && movedY == movedEndY) {
		timer->stop();
		isMovedNow = false;
		board[aY][aX] = movedChess;
		nextChess(nextChessN);
	}
	
	update();
}

void MainWindow::nextChess(int n) {

	if (nextChessN >= chessQueque.size()) {
		update();
		endGame("Воспроизведение игры завершено. Количество ходов: " + QString::number(n));
		return;
	}

	QVector<int> item = chessQueque.at(n);

	isMovedNow = true;

	int beforeX = item[0];
	int beforeY = item[1];
	int afterX = item[2];
	int afterY = item[3];

	movedChess = board[beforeY][beforeX];
	board[beforeY][beforeX] = "";

	movedX = boardMarginX + squareWidth * beforeX;
	movedY = boardMarginY + squareHeight * beforeY;

	movedEndX = boardMarginX + squareWidth * afterX;
	movedEndY = boardMarginY + squareHeight * afterY;

	aX = afterX;
	aY = afterY;

	nextChessN = n + 1;
	
	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(moveChess()));
	timer->start(INTERVAL);
}

void MainWindow::endGame(QString status) {
	saveButton->setEnabled(false);
	openButton->setEnabled(true);
	startRecordButton->setEnabled(true);
	stopRecordButton->setEnabled(true);

	chessQueque.clear();
	nextChessN = -1;

	QMessageBox::information(this, "Игра завершена", status);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event) {
	mouseX = event->x();
	mouseY = event->y();

	boardX = (mouseX - boardMarginX) / squareWidth;
	boardY = (mouseY - boardMarginY) / squareHeight;
	
	if (isRecordNow) {

		if (mouseX - boardMarginX < 0 || mouseX > boardMarginX + squareWidth * boardWidth)
			boardX = -1;

		if (mouseY - boardMarginY < 0 || mouseY > boardMarginY + squareHeight * boardHeight)
			boardY = -1;

		if (isPressed) {
			update();
		}
	}
}

void MainWindow::mousePressEvent(QMouseEvent *event) {
	boardX = (mouseX - boardMarginX) / squareWidth;
	boardY = (mouseY - boardMarginY) / squareHeight;

	chessPressedX = boardX;
	chessPressedY = boardY;

	if (board[chessPressedY][chessPressedX] != "") {
		if (event->button() == Qt::LeftButton && isRecordNow) {
			isPressed = true;
			
			mouseX = event->x();
			mouseY = event->y();
		}
	} else {
		isPressed = false;
	}
}

void MainWindow::mouseReleaseEvent(QMouseEvent *) {
	isPressed = false;

	if (boardX >= 0 && boardY >= 0 && chessPressedX >= 0 && chessPressedY >= 0 && (chessPressedX != boardX || chessPressedY != boardY) && isRecordNow && board[chessPressedY][chessPressedX] != "") {
		QChar selfColor = board[chessPressedY][chessPressedX][0];
		QChar newColor = board[boardY][boardX][0];
		if (selfColor != newColor) {
			board[boardY][boardX] = board[chessPressedY][chessPressedX];
			board[chessPressedY][chessPressedX] = "";
			
			QTextStream out(&tempfile);
			out << chessPressedX << "-" << chessPressedY << ":" << boardX << "-" << boardY << "\n";
		}
	}
	update();
}

void MainWindow::paintEvent(QPaintEvent *) {
	QPainter p(this);
	int color = 1;

	for (int i = 0; i < boardWidth; i++)
	{
		for (int j = 0; j < boardHeight; j++)
		{
			if (color)
				p.setBrush(Qt::white);
			else
				p.setBrush(Qt::black);
			int sqX = boardMarginX + j * squareWidth;
			int sqY = boardMarginY + i * squareHeight;

			p.drawRect(sqX, sqY, squareWidth, squareHeight);

			if (board[i][j] != "") {
				QString image = imagesDir + board[i][j] + ".png";
				p.drawImage(sqX, sqY, QImage(image).scaled(squareWidth, squareHeight));
			}

			color = !color;
		}
		color = !color;
	}

	if (isPressed && chessPressedX >= 0 && chessPressedY >= 0) {
		QString image = imagesDir + board[chessPressedY][chessPressedX] + ".png";
		p.drawImage(mouseX - (squareWidth / 2), mouseY - (squareHeight / 2), QImage(image).scaled(squareWidth, squareHeight));
	}

	if (isMovedNow) {
		QString image = imagesDir + movedChess + ".png";
		p.drawImage(movedX, movedY, QImage(image).scaled(squareWidth, squareHeight));
	}

	QString horizontalChars[boardWidth] = {"A", "B", "C", "D", "E", "F", "G", "H"};
	QString   verticalChars[boardHeight] = {"1", "2", "3", "4", "5", "6", "7", "8"};

	for (int i = 0; i < boardWidth; i++) {
		p.save();
		p.translate(squareWidth / 2 + boardMarginX + i * squareWidth, boardMarginY - 20);
		p.rotate(180);
		p.drawText(0, 0, horizontalChars[i]);
		p.restore();
		p.drawText(squareWidth / 2 + boardMarginX + i * squareWidth, boardMarginY + squareHeight * boardHeight + 20, horizontalChars[i]);
	}

	for (int i = 0; i < boardHeight; i++) {
		p.save();
		p.drawText(boardMarginX - 20, boardMarginY + squareHeight * boardHeight - i * squareHeight - squareHeight / 2, verticalChars[i]);
		p.translate(boardMarginX + squareHeight * boardHeight + 20, boardMarginY + squareHeight * boardHeight - i * squareHeight - squareHeight / 2);
		p.rotate(180);
		p.drawText(0, 0, verticalChars[i]);
		p.restore();
	}
}

void MainWindow::resetBoard() {
	QString newboard[8][8] = {
		{"b5", "b2", "b0", "b4", "b1", "b0", "b2", "b5"},
		{"b3", "b3", "b3", "b3", "b3", "b3", "b3", "b3"},
		{"", "", "", "", "", "", "", ""},
		{"", "", "", "", "", "", "", ""},
		{"", "", "", "", "", "", "", ""},
		{"", "", "", "", "", "", "", ""},
		{"w3", "w3", "w3", "w3", "w3", "w3", "w3", "w3"},
		{"w5", "w2", "w0", "w4", "w1", "w0", "w2", "w5"}
	};

	for (int i = 0; i < boardWidth; i++) {
		for (int j = 0; j < boardHeight; j++) {
			board[i][j] = newboard[i][j];
		}
	}
	
	update();
}