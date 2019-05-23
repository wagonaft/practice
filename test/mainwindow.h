#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>

class MainWindow : public QMainWindow {
	Q_OBJECT
	public:
		MainWindow();
		
	protected:
		void mouseMoveEvent(QMouseEvent *event);
		void mousePressEvent(QMouseEvent *event);
		void mouseReleaseEvent(QMouseEvent *event);
		void paintEvent(QPaintEvent *event);
		void startGame(QString filename);
		void endGame(QString status);
		void resetBoard();
		void nextChess(int n);

	private slots:
		void openFile();
		void saveFile();
		void startRecord();
		void stopRecord();
		void moveChess();

	private:
		QLabel *position;
		QPushButton *openButton;
		QPushButton *saveButton;
		QPushButton *startRecordButton;
		QPushButton *stopRecordButton;
};
#endif
