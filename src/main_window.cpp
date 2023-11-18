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
  connect(this, &MainWindow::deleteCurrentImage, imageLoader,
          &ImageLoader::deleteCurrentImage);
  connect(imageLoader, &ImageLoader::noMoreImagesLeft, this,
          &MainWindow::onNoMoreImagesLeft);
  connect(imageLoader, &ImageLoader::imageLoaded, this,
          &MainWindow::onImageLoaded);

  // Start the thread
  imageLoaderThread->start();

  // Create a menu bar
  QMenuBar *menuBar = new QMenuBar(this);
  setMenuBar(menuBar);

  // Create a "File" menu
  QMenu *fileMenu = menuBar->addMenu("File");
  QMenu *viewMenu = menuBar->addMenu("View");

  // Create an "Open" action
  QAction *openAction = new QAction("Open...", this);
  connect(openAction, &QAction::triggered, this, &MainWindow::openImage);

  // Create a "Quick Export as a PNG" action
  QAction *quickExportAction = new QAction("Quick Export as a PNG", this);
  connect(quickExportAction, &QAction::triggered, this,
          &MainWindow::quickExportAsPng);

  // Create an "Zoom In" action
  QAction *zoomInAction = new QAction("Zoom In", this);
  connect(zoomInAction, &QAction::triggered, this, &MainWindow::zoomIn);

  // Create an "Zoom Out" action
  QAction *zoomOutAction = new QAction("Zoom Out", this);
  connect(zoomOutAction, &QAction::triggered, this, &MainWindow::zoomOut);

  // Add the "Open" action to the "File" menu
  fileMenu->addAction(openAction);
  fileMenu->addAction(quickExportAction);
  viewMenu->addAction(zoomInAction);
  viewMenu->addAction(zoomOutAction);

  // Create a imageViewer to display the image
  imageViewer = new ImageViewer(this);
  connect(imageViewer, &ImageViewer::copyRequested, this,
          &MainWindow::copyToClipboard);

  setCentralWidget(imageViewer);

  sidebar = new VerticalSidebar(this);
  sidebar->hide();

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

    QFileInfo fileInfo(imagePath);
    sidebar->setFileName(fileInfo.fileName());
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

void MainWindow::copyToClipboard() {
  const auto pixmapFullRes = imageLoader->getCurrentImageFullRes();
  QClipboard *clipboard = QGuiApplication::clipboard();
  clipboard->setPixmap(pixmapFullRes);
}

QString prettyPrintSize(qint64 size) {
  const qint64 kb = 1024;
  const qint64 mb = kb * 1024;
  const qint64 gb = mb * 1024;

  if (size < kb) {
    return QString("%1 bytes").arg(size);
  } else if (size < mb) {
    return QString("%1 KB (%2 bytes)")
        .arg(size / kb)
        .arg(QLocale().toString(size));
  } else if (size < gb) {
    return QString("%1 MB (%2 bytes)")
        .arg(size / mb)
        .arg(QLocale().toString(size));
  } else {
    return QString("%1 GB (%2 bytes)")
        .arg(size / gb)
        .arg(QLocale().toString(size));
  }
}

QString getDocumentType(const QFileInfo &fileInfo) {
  QString extension = fileInfo.suffix().toLower();

  if (extension.isEmpty()) {
    return "Unknown Type";
  } else if (extension == "png" || extension == "jpg" || extension == "jpeg" ||
             extension == "gif") {
    return "Image File";
  } else if (extension == "webp") {
    return "WebP Image";
  } else if (extension == "nef") {
    return "Nikon RAW Image";
  } else {
    return "Unknown Type";
  }
}

void MainWindow::onImageLoaded(const QFileInfo &imageFileInfo,
                               const QPixmap &imagePixmap) {
  // Set the resized image to the QLabel
  imageViewer->setPixmap(imagePixmap, width() * 0.80, height() * 0.95);

  sidebar->setFileName(imageFileInfo.fileName());

  qint64 fileSize = imageFileInfo.size();
  QString prettySize = prettyPrintSize(fileSize);
  sidebar->setFileSize(prettySize);

  const auto documentType = getDocumentType(imageFileInfo);
  sidebar->setFileType(documentType);
}

void MainWindow::onNoMoreImagesLeft() {
  auto emptyImage = QImage(0, 0, QImage::Format_ARGB32);
  auto emptyPixmap = QPixmap::fromImage(emptyImage);
  imageViewer->setPixmap(emptyPixmap, 0, 0);
  sidebar->setFileName("");
  sidebar->setFileSize("");
  sidebar->setFileType("");
}

bool MainWindow::event(QEvent *event) {
  if (event->type() == QEvent::KeyPress) {
    // Handle the key press event here
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    Qt::Key key = static_cast<Qt::Key>(keyEvent->key());
    Qt::KeyboardModifiers modifiers = keyEvent->modifiers();

    if ((key == Qt::Key_N || key == Qt::Key_Right) &&
        modifiers == Qt::NoModifier) {
      emit nextImage(imageViewer->pixmap());
      return true; // Event handled
    } else if ((key == Qt::Key_P || key == Qt::Key_Left) &&
               modifiers == Qt::NoModifier) {
      emit previousImage(imageViewer->pixmap());
      return true; // Event handled
    } else if (key == Qt::Key_I && modifiers == Qt::NoModifier) {
      if (!sidebarVisible) {
        sidebar->show();
        sidebarVisible = true;
      } else {
        sidebar->hide();
        sidebarVisible = false;
      }
      return true; // Event handled
    } else if (key == Qt::Key_D && modifiers == Qt::NoModifier) {
      // Delete current image
      emit deleteCurrentImage();
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

void MainWindow::resizeEvent(QResizeEvent *event) {
  auto desiredWidth = width() * 0.80;
  auto desiredHeight = height() * 0.95;
  imageViewer->resize(desiredWidth, desiredHeight);

  QMainWindow::resizeEvent(event);
}

void MainWindow::zoomIn() { imageViewer->zoomIn(); }

void MainWindow::zoomOut() { imageViewer->zoomOut(); }