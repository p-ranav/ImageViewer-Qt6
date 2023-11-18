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
  connect(this, &MainWindow::goBackward, imageLoader, &ImageLoader::goBackward);
  connect(this, &MainWindow::previousImage, imageLoader,
          &ImageLoader::previousImage);
  connect(this, &MainWindow::nextImage, imageLoader, &ImageLoader::nextImage);
  connect(this, &MainWindow::goForward, imageLoader, &ImageLoader::goForward);
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
  connect(imageViewer, &ImageViewer::deleteRequested, this,
          &MainWindow::deleteCurrentImage);

  m_infoSidebar = new VerticalSidebar(imageViewer);
  m_infoSidebar->setPalette(darkPalette);
  m_infoSidebar->hide();

  auto centralWidget = new QWidget(this);
  auto vstackLayout = new QVBoxLayout();
  vstackLayout->addWidget(imageViewer);
  centralWidget->setLayout(vstackLayout);

  // Info (Image Info button)
  m_infoButton = new QPushButton(this);
  m_infoButton->setFixedSize(40, 40);
  connect(m_infoButton, &QPushButton::pressed, [this]() {
    if (!m_sidebarVisible) {
      m_infoSidebar->show();
      m_sidebarVisible = true;
    } else {
      m_infoSidebar->hide();
      m_sidebarVisible = false;
    }
  });

  // Go Backward (Go 10 Images back)
  m_backward10Button = new QPushButton(this);
  m_backward10Button->setFixedSize(40, 40);
  connect(m_backward10Button, &QPushButton::pressed,
          [this]() { emit goBackward(); });

  // Left Arrow (Previous Image button)
  m_leftArrowButton = new QPushButton(this);
  m_leftArrowButton->setFixedSize(40, 40);
  connect(m_leftArrowButton, &QPushButton::pressed,
          [this]() { emit previousImage(imageViewer->pixmap()); });

  // Right Arrow (Next Image button)
  m_rightArrowButton = new QPushButton(this);
  m_rightArrowButton->setFixedSize(40, 40);
  connect(m_rightArrowButton, &QPushButton::pressed,
          [this]() { emit nextImage(imageViewer->pixmap()); });

  // Skip Forward (Go 10 Images forward)
  m_forward10Button = new QPushButton(this);
  m_forward10Button->setFixedSize(40, 40);
  connect(m_forward10Button, &QPushButton::pressed,
          [this]() { emit goForward(); });

  // Trash (Delete Image button)
  m_trashButton = new QPushButton(this);
  m_trashButton->setFixedSize(40, 40);
  connect(m_trashButton, &QPushButton::pressed,
          [this]() { confirmAndDeleteCurrentImage(); });

  // Create a layout and add buttons to it
  auto toolbarWidget = new QWidget(this);
  QHBoxLayout *buttonLayout = new QHBoxLayout();
  buttonLayout->addWidget(m_infoButton);
  buttonLayout->addWidget(m_backward10Button);
  buttonLayout->addWidget(m_leftArrowButton);
  buttonLayout->addWidget(m_rightArrowButton);
  buttonLayout->addWidget(m_forward10Button);
  buttonLayout->addWidget(m_trashButton);
  toolbarWidget->setLayout(buttonLayout);
  // toolbarWidget->setFixedWidth(100);

  vstackLayout->addWidget(toolbarWidget, 0, Qt::AlignCenter);

  // Set the icon with a specific color
  QColor iconColor(Qt::white); // Set your desired color
  m_infoButton->setIcon(createColorIcon(":/images/info.png", iconColor, 24));
  m_backward10Button->setIcon(
      createColorIcon(":/images/backward_10.png", iconColor, 24));
  m_leftArrowButton->setIcon(
      createColorIcon(":/images/left_arrow.png", iconColor, 24));
  m_rightArrowButton->setIcon(
      createColorIcon(":/images/right_arrow.png", iconColor, 24));
  m_forward10Button->setIcon(
      createColorIcon(":/images/forward_10.png", iconColor, 24));
  m_trashButton->setIcon(createColorIcon(":/images/trash.png", iconColor, 24));

  imageViewer->setStyleSheet("border: none;");
  centralWidget->setStyleSheet("background-color: rgb(25, 25, 25);");

  setCentralWidget(centralWidget);

  openImage();
}

QIcon MainWindow::createColorIcon(const QString &imagePath, const QColor &color,
                                  int size) {
  QPixmap pixmap(imagePath);
  pixmap =
      pixmap.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);

  QPainter painter(&pixmap);
  painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
  painter.fillRect(pixmap.rect(), color);

  return QIcon(pixmap);
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
    m_infoSidebar->setFileName(fileInfo.fileName());
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
                               const QPixmap &imagePixmap, int imageWidth,
                               int imageHeight) {
  // Set the resized image to the QLabel
  imageViewer->setPixmap(imagePixmap, width() * 0.80, height() * 0.80);

  m_infoSidebar->setFixedWidth(imageViewer->width());

  m_infoSidebar->setFilePosition(imageLoader->getHeaderLabel());

  m_infoSidebar->setFileName(imageFileInfo.fileName());

  qint64 fileSize = imageFileInfo.size();
  QString prettySize = prettyPrintSize(fileSize);
  m_infoSidebar->setFileSize(prettySize);

  const auto documentType = getDocumentType(imageFileInfo);
  m_infoSidebar->setFileType(documentType);

  const auto imageResolution =
      QString("%1 x %2").arg(imageWidth).arg(imageHeight);
  m_infoSidebar->setImageResolution(imageResolution);
}

void MainWindow::onNoMoreImagesLeft() {
  auto emptyImage = QImage(0, 0, QImage::Format_ARGB32);
  auto emptyPixmap = QPixmap::fromImage(emptyImage);
  imageViewer->setPixmap(emptyPixmap, 0, 0);
  m_infoSidebar->setFileName("");
  m_infoSidebar->setFileSize("");
  m_infoSidebar->setFileType("");
}

void MainWindow::confirmAndDeleteCurrentImage() {

  // Display a confirmation dialog
  QMessageBox::StandardButton reply = QMessageBox::question(
      this, "Delete Confirmation", "Do you really want to delete this item?",
      QMessageBox::Yes | QMessageBox::No);

  // Check the user's response
  if (reply == QMessageBox::Yes) {
    emit deleteCurrentImage();
  } else {
    // User clicked 'No' or closed the dialog
    qDebug() << "Deletion canceled.";
  }
}

bool MainWindow::event(QEvent *event) {
  if (event->type() == QEvent::KeyPress) {
    // Handle the key press event here
    QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
    Qt::Key key = static_cast<Qt::Key>(keyEvent->key());
    Qt::KeyboardModifiers modifiers = keyEvent->modifiers();

    if ((key == Qt::Key_N || key == Qt::Key_Right)/* &&
        modifiers == Qt::NoModifier*/) {
      emit nextImage(imageViewer->pixmap());
      return true; // Event handled
    } else if ((key == Qt::Key_P || key == Qt::Key_Left)/* &&
               modifiers == Qt::NoModifier*/) {
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

void MainWindow::resizeEvent(QResizeEvent *event) {
  auto desiredWidth = width() * 0.80;
  auto desiredHeight = height() * 0.80;
  imageViewer->resize(desiredWidth, desiredHeight);
  m_infoSidebar->setFixedWidth(imageViewer->width());

  QMainWindow::resizeEvent(event);
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    // Handle left mouse button double click
    QPointF scenePos = imageViewer->mapToScene(event->pos());
    qDebug() << "Double click at scene coordinates:" << scenePos;

    auto desiredWidth = width() * 0.80;
    auto desiredHeight = height() * 0.80;
    imageViewer->resize(desiredWidth, desiredHeight);
  }

  // Call the base class implementation
  QMainWindow::mouseDoubleClickEvent(event);
}

void MainWindow::zoomIn() { imageViewer->zoomIn(); }

void MainWindow::zoomOut() { imageViewer->zoomOut(); }