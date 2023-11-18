#pragma once
#include <QWidget>
#include <QPainter>
#include <QPaintEvent>

class VerticalSidebar : public QWidget {
	QLabel* filePositionHeader;
	QLabel* fileNameValue;
	QLabel* fileSizeValue;
	QLabel* fileTypeValue;
	QLabel* imageResolutionValue;
public:
    VerticalSidebar(QWidget *parent = nullptr) : QWidget(parent) {
        setAttribute(Qt::WA_TranslucentBackground); // Enable translucent background
        setFixedSize(500, parentWidget()->height()); // Set the size of the sidebar

        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setContentsMargins(20, 20, 20, 20); // Set margins on all sides

		filePositionHeader = new QLabel("1/5");
		mainLayout->addWidget(filePositionHeader, 0, Qt::AlignCenter);

        QFormLayout *layout = new QFormLayout();
        layout->setLabelAlignment(Qt::AlignLeft);
        layout->setFormAlignment(Qt::AlignLeft);
		layout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow); // Align fields to the left
        mainLayout->addLayout(layout);

        // File Name
        QLabel *fileNameLabel = new QLabel("File name ");
        fileNameValue = new QLabel(""); // Default value
        layout->addRow(fileNameLabel, fileNameValue);
		fileNameLabel->setAttribute(Qt::WA_TranslucentBackground);
		fileNameValue->setAttribute(Qt::WA_TranslucentBackground);

        // File Size
        QLabel *fileSizeLabel = new QLabel("File size ");
        fileSizeValue = new QLabel(""); // Default value
        layout->addRow(fileSizeLabel, fileSizeValue);
		fileSizeLabel->setAttribute(Qt::WA_TranslucentBackground);
		fileSizeValue->setAttribute(Qt::WA_TranslucentBackground);

        // File Type
        QLabel *fileTypeLabel = new QLabel("Document type ");
        fileTypeValue = new QLabel(""); // Default value
        layout->addRow(fileTypeLabel, fileTypeValue);
		fileTypeLabel->setAttribute(Qt::WA_TranslucentBackground);
		fileTypeValue->setAttribute(Qt::WA_TranslucentBackground);

		// Add a horizontal line
		QFrame* horizontalLine = new QFrame();
		horizontalLine->setFrameShape(QFrame::HLine);
		horizontalLine->setFrameShadow(QFrame::Sunken);
		layout->addRow(horizontalLine);

        // Image Resolution
        QLabel *imageResolutionLabel = new QLabel("Image resolution ");
        imageResolutionValue = new QLabel(""); // Default value
        layout->addRow(imageResolutionLabel, imageResolutionValue);
		imageResolutionLabel->setAttribute(Qt::WA_TranslucentBackground);
		imageResolutionValue->setAttribute(Qt::WA_TranslucentBackground);

        // Calculate the preferred height based on the content
        int preferredHeight = sizeHint().height();
        setFixedHeight(preferredHeight);

		move(20, 20);
    }

	void setFilePosition(const QString& filePosition) {
		filePositionHeader->setText(filePosition);
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

	void setImageResolution(const QString& imageResolution) {
		imageResolutionValue->setText(imageResolution);
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