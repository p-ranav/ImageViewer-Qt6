#include "MainWindow.hpp"

MainWindow::MainWindow() : QMainWindow() {

  // Create a fixed-size QPixmap on startup
  QRect primaryScreenGeometry = QApplication::primaryScreen()->geometry();
  setGeometry(primaryScreenGeometry);

  m_preferences = new Preferences();
  connect(m_preferences, &Preferences::settingChangedSlideShowPeriod, this,
          &MainWindow::settingChangedSlideShowPeriod);
  connect(m_preferences, &Preferences::settingChangedSlideShowLoop, this,
          &MainWindow::settingChangedSlideShowLoop);

  // Create the image loader and move it to a separate thread
  imageLoader = new ImageLoader;
  imageLoaderThread = new QThread;
  imageLoader->moveToThread(imageLoaderThread);

  // Connect signals and slots for image loading
  connect(this, &MainWindow::loadImage, imageLoader, &ImageLoader::loadImage);
  connect(this, &MainWindow::goToStart, imageLoader, &ImageLoader::goToStart);
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
  openAction->setShortcut(QKeySequence("Ctrl+O"));
  connect(openAction, &QAction::triggered, this, &MainWindow::openImage);

  // Create a "Copy to clipboard" action
  QAction *copyToClipboardAction = new QAction("Copy Image", this);
  copyToClipboardAction->setShortcut(QKeySequence("Ctrl+C"));
  connect(copyToClipboardAction, &QAction::triggered, this,
          &MainWindow::copyToClipboard);

  // Create a "Copy Image Path" action
  QAction *copyImagePathAction = new QAction("Copy Image Path", this);
  connect(copyImagePathAction, &QAction::triggered, this,
          &MainWindow::copyImagePathToClipboard);

  // Create a "Copy to..." action
  QAction *copyToLocationAction = new QAction("Copy to...", this);
  connect(copyToLocationAction, &QAction::triggered, this,
          &MainWindow::copyToLocation);

  // Create a "Delete" action
  QAction *deleteAction = new QAction("Delete Image", this);
  deleteAction->setShortcut(QKeySequence("Ctrl+D"));
  connect(deleteAction, &QAction::triggered, this,
          &MainWindow::confirmAndDeleteCurrentImage);

  // Create a "Quick Export as a PNG" action
  QAction *quickExportAction = new QAction("Quick Export as a PNG", this);
  connect(quickExportAction, &QAction::triggered, this,
          &MainWindow::quickExportAsPng);

  // Create an "Zen mode" action
  QAction *preferencesAction = new QAction("Preferences", this);
  connect(preferencesAction, &QAction::triggered, this,
          &MainWindow::showPreferences);

  // Create an "Zoom In" action
  QAction *zoomInAction = new QAction("Zoom In", this);
  zoomInAction->setShortcut(QKeySequence("Ctrl++"));
  connect(zoomInAction, &QAction::triggered, this, &MainWindow::zoomIn);

  // Create an "Zoom Out" action
  QAction *zoomOutAction = new QAction("Zoom Out", this);
  zoomOutAction->setShortcut(QKeySequence("Ctrl+-"));
  connect(zoomOutAction, &QAction::triggered, this, &MainWindow::zoomOut);

  // Create an "Slideshow" action
  QAction *slideshowAction = new QAction("Start Slideshow", this);
  connect(slideshowAction, &QAction::triggered, this,
          &MainWindow::startSlideshow);

  slideshowTimer = new QTimer(this);
  slideshowTimer->setInterval(
      m_preferences->get(Preferences::SETTING_SLIDESHOW_PERIOD, 2500).toInt());
  connect(slideshowTimer, &QTimer::timeout, this,
          &MainWindow::slideshowTimerCallback);

  m_slideshowLoop =
      m_preferences->get(Preferences::SETTING_SLIDESHOW_LOOP, false).toBool();

  // Add the "Open" action to the "File" menu
  fileMenu->addAction(openAction);
  fileMenu->addSeparator();
  fileMenu->addAction(copyToClipboardAction);
  fileMenu->addAction(copyImagePathAction);
  fileMenu->addAction(copyToLocationAction);
  fileMenu->addSeparator();
  fileMenu->addAction(deleteAction);
  fileMenu->addSeparator();
  fileMenu->addAction(quickExportAction);
  fileMenu->addAction(preferencesAction);
  viewMenu->addAction(zoomInAction);
  viewMenu->addAction(zoomOutAction);
  viewMenu->addSeparator();
  viewMenu->addAction(slideshowAction);

  // Create a imageViewer to display the image
  imageViewer = new ImageViewer(this);
  connect(imageViewer, &ImageViewer::copyRequested, this,
          &MainWindow::copyToClipboard);
  connect(imageViewer, &ImageViewer::deleteRequested, this,
          &MainWindow::deleteCurrentImage);

  m_centralWidget = new QWidget(this);
  auto vstackLayout = new QVBoxLayout();
  vstackLayout->addWidget(imageViewer);
  m_centralWidget->setLayout(vstackLayout);

  m_centralWidget->setStyleSheet(
      "background-color: rgb(25, 25, 25); padding: 0px;");

  setCentralWidget(m_centralWidget);

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
  } else {
    m_firstLoad = false;
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

void MainWindow::copyImagePathToClipboard() {
  QClipboard *clipboard = QGuiApplication::clipboard();
  clipboard->setText(imageLoader->getCurrentImageFilePath());
}

QString getLastDestination() {
  QSettings settings("p-ranav", "ImageViewer");
  return settings.value("lastCopyDestination", QDir::homePath()).toString();
}

void setLastDestination(const QString &path) {
  QSettings settings("p-ranav", "ImageViewer");
  settings.setValue("lastCopyDestination", path);
}

void MainWindow::copyToLocation() {
  auto currentFilePath = imageLoader->getCurrentImageFilePath();

  QFileInfo currentFileInfo(currentFilePath);
  QFile sourceFile(currentFilePath);
  if (!sourceFile.open(QIODevice::ReadOnly)) {
    qWarning() << "Failed to open source file:" << sourceFile.errorString();
    return;
  }

  QString lastUsedDestination = getLastDestination();
  QDir candidateDir(lastUsedDestination);
  QFileInfo candidateFileInfo(candidateDir, currentFileInfo.fileName());
  auto candidateSaveLocation = candidateFileInfo.filePath();

  QString destinationFilePath = QFileDialog::getSaveFileName(
      this, tr("Save Image"), candidateSaveLocation,
      tr("Images (*.png *.jpg *.bmp);;All Files (*)"));
  if (!destinationFilePath.isEmpty()) {

    // Open the destination file for writing
    QFile destinationFile(destinationFilePath);
    if (!destinationFile.open(QIODevice::WriteOnly)) {
      qWarning() << "Failed to open destination file:"
                 << destinationFile.errorString();
      sourceFile.close(); // Close the source file before returning
      return;
    }

    // Copy the contents of the source file to the destination file
    QByteArray data = sourceFile.readAll();
    destinationFile.write(data);

    // Close both files
    sourceFile.close();
    destinationFile.close();

    setLastDestination(destinationFilePath);
  }
}

void MainWindow::onImageLoaded(const QFileInfo &fileInfo,
                               const QPixmap &imagePixmap,
                               const ImageInfo &imageInfo) {

  // Set the resized image to the QLabel
  imageViewer->setPixmap(imagePixmap, width() * getScaleFactor(),
                         height() * getScaleFactor());

  if (m_firstLoad) {

    QSize sizeHint = imageViewer->sizeHint();

    // Check if the size is smaller than the minimum size or larger than the
    // default size
    if (sizeHint.width() < m_minSize.width() ||
        sizeHint.height() < m_minSize.height()) {
      sizeHint = m_minSize;
    }

    // Get the primary screen's geometry
    QScreen *primaryScreen = QGuiApplication::primaryScreen();
    QRect screenGeometry = primaryScreen->geometry();

    // Calculate the center position
    int centerX = screenGeometry.width() / 2 - sizeHint.width() / 2;
    int centerY = screenGeometry.height() / 2 - sizeHint.height() / 2;

    setGeometry(QRect(centerX, centerY, sizeHint.width(), sizeHint.height()));

    m_firstLoad = false;
  }

  setWindowTitle(
      fileInfo.fileName() +
      QString(" (%1 x %2)").arg(imageInfo.width).arg(imageInfo.height));
}

void MainWindow::onNoMoreImagesLeft() {
  auto emptyImage = QImage(0, 0, QImage::Format_ARGB32);
  auto emptyPixmap = QPixmap::fromImage(emptyImage);
  imageViewer->setPixmap(emptyPixmap, 0, 0);
}

void MainWindow::confirmAndDeleteCurrentImage() {

  // Display a confirmation dialog
  QMessageBox::StandardButton reply = QMessageBox::question(
      this, "Delete Confirmation", "Do you really want to delete this item?",
      QMessageBox::Yes | QMessageBox::No);

  grabKeyboard();
  setFocus();
  activateWindow();

  // Check the user's response
  if (reply == QMessageBox::Yes) {
    emit deleteCurrentImage();
  } else {
    // User clicked 'No' or closed the dialog
    qDebug() << "Deletion canceled.";
  }
}

void MainWindow::closeEvent(QCloseEvent *event) {
  // Clean up the thread when the main window is closed
  imageLoaderThread->quit();
  imageLoaderThread->wait();
  event->accept();
}

void MainWindow::resizeEvent(QResizeEvent *event) {
  auto desiredWidth = width() * getScaleFactor();
  auto desiredHeight = height() * getScaleFactor();
  imageViewer->resize(desiredWidth, desiredHeight);

  QMainWindow::resizeEvent(event);
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    // Handle left mouse button double click
    QPointF scenePos = imageViewer->mapToScene(event->pos());
    qDebug() << "Double click at scene coordinates:" << scenePos;

    auto desiredWidth = width() * getScaleFactor();
    auto desiredHeight = height() * getScaleFactor();
    imageViewer->resize(desiredWidth, desiredHeight);
  }

  // Call the base class implementation
  QMainWindow::mouseDoubleClickEvent(event);
}

void MainWindow::zoomIn() { imageViewer->zoomIn(); }

void MainWindow::zoomOut() { imageViewer->zoomOut(); }

void MainWindow::slideshowTimerCallback() {
  if (imageLoader->hasNext()) {
    emit nextImage(imageViewer->pixmap());
  } else {
    /// No more images left

    /// Check if slideshow is configured to loop
    if (m_slideshowLoop) {
      /// Restart slideshow
      emit goToStart();
    }
  }
}

void MainWindow::startSlideshow() { slideshowTimer->start(); }

qreal MainWindow::getScaleFactor() const { return 1; }

void MainWindow::keyPressEvent(QKeyEvent *event) {

  if (slideshowTimer->isActive()) {
    slideshowTimer->stop();
    /// TODO: Add reachedEnd signal to stop slideshow
    /// when no more images left
    ///
    /// this can also be used to restart the slidehow from
    /// the start
  }

  switch (event->key()) {
  case Qt::Key_Up:
    /// std::cout << "UP\n";
    break;
  case Qt::Key_Down:
    /// std::cout << "DOWN\n";
    break;
  case Qt::Key_Right:
    emit nextImage(imageViewer->pixmap());
    break;
  case Qt::Key_Left:
    emit previousImage(imageViewer->pixmap());
    break;
  }
  /// QMainWindow::keyPressEvent(event);
}

void MainWindow::showPreferences() { m_preferences->show(); }

void MainWindow::settingChangedSlideShowPeriod() {
  slideshowTimer->setInterval(
      m_preferences->get(Preferences::SETTING_SLIDESHOW_PERIOD, 2500).toInt());
}

void MainWindow::settingChangedSlideShowLoop() {
  m_slideshowLoop =
      m_preferences->get(Preferences::SETTING_SLIDESHOW_LOOP, false).toBool();
}