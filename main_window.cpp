#include "main_window.hpp"

    MainWindow::MainWindow(QWidget *parent) : QWidget(parent) {
        // Set a dark color palette
        QPalette darkPalette;
        darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::WindowText, Qt::white);
        darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
        darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
        darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
        darkPalette.setColor(QPalette::ToolTipText, Qt::white);
        darkPalette.setColor(QPalette::Text, Qt::white);
        darkPalette.setColor(QPalette::Button, QColor(42, 130, 218));  // Blue color
        darkPalette.setColor(QPalette::ButtonText, Qt::white);
        darkPalette.setColor(QPalette::BrightText, Qt::red);
        darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
        darkPalette.setColor(QPalette::HighlightedText, Qt::black);

        // Apply the dark color palette to the application
        setPalette(darkPalette);

        // Create a QVBoxLayout for the main layout
        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        // Create a QSplitter for the sidebar and central widget
        QSplitter *splitter = new QSplitter(Qt::Horizontal);

        // Create a QPushButton for the sidebar
        QPushButton *addPhotosButton = new QPushButton("Add Photos");

        // Set a larger size for the button
        addPhotosButton->setFixedSize(150, 50);

        // Set the style to have square edges and handle hover and click events
        addPhotosButton->setStyleSheet(
            "QPushButton {"
            "   border-radius: 0;"
            "   background-color: #2A82DA;"  // Initial background color (blue)
            "   color: white;"
            "}"
            "QPushButton:hover {"
            "   background-color: #3682DA;"  // Hover background color (a slightly lighter shade of blue)
            "}"
            "QPushButton:pressed {"
            "   background-color: #1D6EB7;"  // Clicked background color (a slightly darker shade of blue)
            "}"
        );

        // Connect the button's clicked signal to a slot for handling image loading
        connect(addPhotosButton, &QPushButton::clicked, this, &MainWindow::openImage);

        // Add the button to the sidebar
        splitter->addWidget(addPhotosButton);

        // Create a QLabel with a QPixmap as its canvas
        canvasLabel.setAlignment(Qt::AlignCenter);

        // Create a fixed-size QPixmap on startup
		QRect primaryScreenGeometry = QApplication::primaryScreen()->geometry();
		setGeometry(primaryScreenGeometry);

		QSize canvasSize = primaryScreenGeometry.size() * 0.75;
        fixedPixmap = QPixmap(canvasSize);  // Set your desired size
        fixedPixmap.fill(QColor(25, 25, 25));  // Fill with a dark background
        canvasLabel.setPixmap(fixedPixmap);

        // Add the canvas to the central layout
        splitter->addWidget(&canvasLabel);

        // Set the size policy for the central layout
        splitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        // Set the sizes for the sidebar and central layout
        splitter->setSizes({150, 400});

        // Add the splitter to the main layout
        mainLayout->addWidget(splitter);
    }

	void MainWindow::openImage() {
        // Open a file dialog to select an image
        QString imagePath = QFileDialog::getOpenFileName(this, "Open Image", "", "Images (*.png *.jpg *.bmp)");

        if (!imagePath.isEmpty()) {
            // Load the selected image into the QPixmap and resize it
            QPixmap imagePixmap(imagePath);
            imagePixmap = imagePixmap.scaled(fixedPixmap.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

            // Set the resized image to the QLabel
            canvasLabel.setPixmap(imagePixmap);
        }
	}