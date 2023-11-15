#include "main_window.hpp"

MainWindow::MainWindow() : QMainWindow() {

  //   QWidget *mainWidget = new QWidget();

  //   // Create a QVBoxLayout for the main layout
  //   QVBoxLayout *mainLayout = new QVBoxLayout(this);

  // Set a dark color palette
  QPalette darkPalette;
  darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
  darkPalette.setColor(QPalette::WindowText, Qt::white);
  darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
  darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
  darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
  darkPalette.setColor(QPalette::ToolTipText, Qt::white);
  darkPalette.setColor(QPalette::Text, Qt::white);
  darkPalette.setColor(QPalette::Button, QColor(42, 130, 218)); // Blue color
  darkPalette.setColor(QPalette::ButtonText, Qt::white);
  darkPalette.setColor(QPalette::BrightText, Qt::red);
  darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
  darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
  darkPalette.setColor(QPalette::HighlightedText, Qt::black);

  // Apply the dark color palette to the application
  setPalette(darkPalette);

  //   // Create a QLabel with a QPixmap as its canvas
  //   canvasLabel.setAlignment(Qt::AlignCenter);

  // Create a fixed-size QPixmap on startup
  QRect primaryScreenGeometry = QApplication::primaryScreen()->geometry();
  setGeometry(primaryScreenGeometry);

  QSize canvasSize = primaryScreenGeometry.size() * 0.80;
  fixedPixmap = QPixmap(canvasSize);    // Set your desired size
  fixedPixmap.fill(QColor(25, 25, 25)); // Fill with a dark background
                                        //   canvasLabel.setPixmap(fixedPixmap);

  //   // Add the splitter to the main layout
  //   mainLayout->addWidget(&canvasLabel);

  // Create the image loader and move it to a separate thread
  imageLoader = new ImageLoader;
  imageLoaderThread = new QThread;
  imageLoader->moveToThread(imageLoaderThread);

  // Connect signals and slots for image loading
  connect(this, &MainWindow::loadImage, imageLoader, &ImageLoader::loadImage);
  connect(imageLoader, &ImageLoader::imageLoaded, this,
          &MainWindow::onImageLoaded);

  // Start the thread
  imageLoaderThread->start();

  // Create a menu bar
  QMenuBar *menuBar = new QMenuBar(this);
  setMenuBar(menuBar);

  // Create a "File" menu
  QMenu *fileMenu = menuBar->addMenu("File");

  // Create an "Open" action
  QAction *openAction = new QAction("Open", this);
  connect(openAction, &QAction::triggered, this, &MainWindow::openImage);

  // Add the "Open" action to the "File" menu
  fileMenu->addAction(openAction);

  // Create a imageViewer to display the image
  imageViewer = new ImageViewer();
  setCentralWidget(imageViewer);
}

void MainWindow::openImage() {
  // Open a file dialog to select an image
  QString imagePath = QFileDialog::getOpenFileName(
      this, "Open Image", "", "Images (*.png *.jpg *.bmp)");

  if (!imagePath.isEmpty()) {
    // Emit a signal to load the image in a separate thread
    emit loadImage(imagePath, fixedPixmap.size());
  }
}

void MainWindow::onImageLoaded(const QPixmap &imagePixmap,
                               const QPixmap &imagePixmapScaled, int width,
                               int height) {
  originalPixmap = imagePixmap;
  scaledPixmap = imagePixmapScaled;
  originalWidth = width;
  originalHeight = height;

  // Set the resized image to the QLabel
  imageViewer->setPixmap(imagePixmap);
}

void MainWindow::closeEvent(QCloseEvent *event) {
  // Clean up the thread when the main window is closed
  imageLoaderThread->quit();
  imageLoaderThread->wait();
  event->accept();
}