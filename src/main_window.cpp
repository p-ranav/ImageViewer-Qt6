#include "main_window.hpp"

MainWindow::MainWindow() : QMainWindow() {

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

  // Create a fixed-size QPixmap on startup
  QRect primaryScreenGeometry = QApplication::primaryScreen()->geometry();
  setGeometry(primaryScreenGeometry);

  // Create the image loader and move it to a separate thread
  imageLoader = new ImageLoader;
  imageLoaderThread = new QThread;
  imageLoader->moveToThread(imageLoaderThread);

  // Connect signals and slots for image loading
  connect(this, &MainWindow::loadImage, imageLoader, &ImageLoader::loadImage);
  connect(this, &MainWindow::nextImage, imageLoader, &ImageLoader::nextImage);
  connect(this, &MainWindow::previousImage, imageLoader,
          &ImageLoader::previousImage);
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
  QAction *openAction = new QAction("Open...", this);
  connect(openAction, &QAction::triggered, this, &MainWindow::openImage);

  // Create a "Quick Export as a PNG" action
  QAction *quickExportAction = new QAction("Quick Export as a PNG", this);
  connect(quickExportAction, &QAction::triggered, this,
          &MainWindow::quickExportAsPng);

  // Add the "Open" action to the "File" menu
  fileMenu->addAction(openAction);
  fileMenu->addAction(quickExportAction);

  // Create a imageViewer to display the image
  imageViewer = new ImageViewer();
  setCentralWidget(imageViewer);

  openImage();
}

void MainWindow::openImage() {
  // Open a file dialog to select an image
  QString fileFilter = "Images (*.png *.jpg *.jpeg *.nef "
                       "*.tiff *.webp)";

  QString imagePath = QFileDialog::getOpenFileName(
      this, "Open Image", /*QDir::homePath()*/ "", fileFilter);

  if (!imagePath.isEmpty()) {
    // Emit a signal to load the image in a separate thread

    imageLoader->resetImageFilePaths();
    emit loadImage(imagePath);
  }
}

void MainWindow::quickExportAsPng() {

  QString currentFilePath = imageLoader->getCurrentImageFilePath();
  QFileInfo fileInfo(currentFilePath);
  QString fileName = fileInfo.baseName();

  // Get a file name for saving the PNG
  QString saveFileName = QFileDialog::getSaveFileName(
      nullptr, "Save PNG File", fileName + ".png", "PNG Files (*.png)");

  if (saveFileName.isEmpty()) {
    // User canceled the operation or didn't provide a file name
    return;
  }

  // Ensure the file extension is .png
  if (!saveFileName.toLower().endsWith(".png")) {
    saveFileName += ".png";
  }

  // Save the image as a PNG file
  auto pixmapFullRes = imageLoader->getCurrentImageFullRes();
  if (pixmapFullRes.save(saveFileName, "PNG")) {
    qDebug() << "Image saved successfully as" << saveFileName;
  } else {
    qDebug() << "Error saving image";
  }
}

void MainWindow::onImageLoaded(const QPixmap &imagePixmap) {
  // Set the resized image to the QLabel
  imageViewer->setPixmap(imagePixmap, height() * 0.95);
}

bool MainWindow::event(QEvent *event) {
  if (event->type() == QEvent::KeyPress) {
    // Handle the key press event here
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    Qt::Key key = static_cast<Qt::Key>(keyEvent->key());
    Qt::KeyboardModifiers modifiers = keyEvent->modifiers();

    if (key == Qt::Key_N && modifiers == Qt::NoModifier) {
      emit nextImage(imageViewer->pixmap());
      return true; // Event handled
    } else if (key == Qt::Key_P && modifiers == Qt::NoModifier) {
      emit previousImage(imageViewer->pixmap());
      return true; // Event handled
    }
  }

  // Call the base class implementation for other events
  return QMainWindow::event(event);
}

void MainWindow::closeEvent(QCloseEvent *event) {
  // Clean up the thread when the main window is closed
  imageLoaderThread->quit();
  imageLoaderThread->wait();
  event->accept();
}