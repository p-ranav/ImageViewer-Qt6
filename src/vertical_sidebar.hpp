#pragma once
#include <QWidget>
#include <QPainter>
#include <QPaintEvent>

class VerticalSidebar : public QWidget {
	QLabel* fileNameValue;
	QLabel* fileSizeValue;
	QLabel* fileTypeValue;
public:
    VerticalSidebar(QWidget *parent = nullptr) : QWidget(parent) {
        setAttribute(Qt::WA_TranslucentBackground); // Enable translucent background
        setFixedSize(500, parentWidget()->height()); // Set the size of the sidebar

        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(20, 20, 20, 20); // Set margins on all sides

        QFormLayout *layout = new QFormLayout();
        layout->setLabelAlignment(Qt::AlignLeft);
        layout->setFormAlignment(Qt::AlignLeft);
		layout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow); // Align fields to the left
        mainLayout->addLayout(layout);

        // File Name
        QLabel *fileNameLabel = new QLabel("File name ");
        fileNameValue = new QLabel(""); // Default value
        layout->addRow(fileNameLabel, fileNameValue);

        // File Size
        QLabel *fileSizeLabel = new QLabel("File size ");
        fileSizeValue = new QLabel(""); // Default value
        layout->addRow(fileSizeLabel, fileSizeValue);

        // File Type
        QLabel *fileTypeLabel = new QLabel("Document type ");
        fileTypeValue = new QLabel(""); // Default value
        layout->addRow(fileTypeLabel, fileTypeValue);

        // Calculate the preferred height based on the content
        int preferredHeight = sizeHint().height();
        setFixedHeight(preferredHeight);

		move(20, 20);
    }

	void setFileName(const QString& fileName) {
		fileNameValue->setText(fileName);
	}

	void setFileSize(const QString& fileSize) {
		fileSizeValue->setText(fileSize);
	}

	void setFileType(const QString& fileType) {
		fileTypeValue->setText(fileType);
	}

protected:
    void paintEvent(QPaintEvent *) override {
        QPainter painter(this);
        painter.fillRect(rect(), QColor(0, 0, 0, 150)); // Set the background color and alpha
    }

    QSize sizeHint() const override {
        return QSize(200, QWidget::sizeHint().height()); // Set the preferred width
    }
};