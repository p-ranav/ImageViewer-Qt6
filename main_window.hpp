#pragma once
#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QPixmap>
#include <QScreen>
#include <QPushButton>
#include <QSplitter>
#include <QHBoxLayout>
#include <QFileDialog>

class MainWindow : public QWidget {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
	~MainWindow() = default;

public slots:
    void openImage();

private:
    QLabel canvasLabel;
    QPixmap fixedPixmap;
};