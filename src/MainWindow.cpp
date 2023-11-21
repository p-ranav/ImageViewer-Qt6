#include "MainWindow.hpp"

#define CONNECT_TO_IMAGE_LOADER(signal_slot_name)                              \
  connect(this, &MainWindow::signal_slot_name, imageLoader,                    \
          &ImageLoader::signal_slot_name, Qt::QueuedConnection);

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
  CONNECT_TO_IMAGE_LOADER(resetImageFilePaths);
  CONNECT_TO_IMAGE_LOADER(loadImage);
  CONNECT_TO_IMAGE_LOADER(goToStart);
  CONNECT_TO_IMAGE_LOADER(goBackward);
  CONNECT_TO_IMAGE_LOADER(previousImage);
  CONNECT_TO_IMAGE_LOADER(nextImage);
  CONNECT_TO_IMAGE_LOADER(goForward);
  CONNECT_TO_IMAGE_LOADER(deleteCurrentImage);
  CONNECT_TO_IMAGE_LOADER(changeSortOrder);
  CONNECT_TO_IMAGE_LOADER(changeSortBy);
  CONNECT_TO_IMAGE_LOADER(copyCurrentImageFullResToClipboard);

  connect(imageLoader, &ImageLoader::noMoreImagesLeft, this,
          &MainWindow::onNoMoreImagesLeft, Qt::QueuedConnection);
  connect(imageLoader, &ImageLoader::imageLoaded, this,
          &MainWindow::onImageLoaded, Qt::QueuedConnection);

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

  QAction *preferencesAction = new QAction("Preferences", this);
  connect(preferencesAction, &QAction::triggered, this,
          &MainWindow::showPreferences);

  QAction *quitAction = new QAction("Quit", this);
  quitAction->setShortcut(QKeySequence("Ctrl+Q"));
  connect(quitAction, &QAction::triggered, this, &QApplication::quit);

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
  fileMenu->addAction(preferencesAction);
  fileMenu->addAction(quitAction);
  viewMenu->addAction(zoomInAction);
  viewMenu->addAction(zoomOutAction);
  viewMenu->addSeparator();
  viewMenu->addAction(slideshowAction);

  createSortByMenu(viewMenu);
  createSortOrderMenu(viewMenu);

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

void MainWindow::createSortOrderMenu(QMenu *viewMenu) {
  // Create the "Sort Order" menu
  QMenu *sortOrderMenu = viewMenu->addMenu(tr("Sort Order"));

  // Create action group
  QActionGroup *sortGroup = new QActionGroup(this);

  // Create actions for "Ascending" and "Descending"
  QAction *ascendingAction = new QAction(tr("Ascending"), this);
  QAction *descendingAction = new QAction(tr("Descending"), this);

  // Set checkable property and add actions to the group
  ascendingAction->setCheckable(true);
  descendingAction->setCheckable(true);
  sortGroup->addAction(ascendingAction);
  sortGroup->addAction(descendingAction);

  // Connect actions to slots
  connect(ascendingAction, &QAction::triggered, this,
          [this]() { emit changeSortOrder(SortOrder::ascending); });
  connect(descendingAction, &QAction::triggered, this,
          [this]() { emit changeSortOrder(SortOrder::descending); });

  // By default, set "Ascending" as checked
  ascendingAction->setChecked(true);

  // Add actions to the "Sort" menu
  sortOrderMenu->addAction(ascendingAction);
  sortOrderMenu->addAction(descendingAction);
}

void MainWindow::createSortByMenu(QMenu *viewMenu) {
  // Create the "Sort By" menu
  QMenu *sortByMenu = viewMenu->addMenu(tr("Sort By"));

  // Create action group
  QActionGroup *sortGroup = new QActionGroup(this);

  // Create actions for "Ascending" and "Descending"
  QAction *nameAction = new QAction(tr("Name"), this);
  QAction *sizeAction = new QAction(tr("Size"), this);
  QAction *dateModifiedAction = new QAction(tr("Date Modified"), this);

  // Set checkable property and add actions to the group
  nameAction->setCheckable(true);
  sizeAction->setCheckable(true);
  dateModifiedAction->setCheckable(true);
  sortGroup->addAction(nameAction);
  sortGroup->addAction(sizeAction);
  sortGroup->addAction(dateModifiedAction);

  // Connect actions to slots
  connect(nameAction, &QAction::triggered, this,
          [this]() { emit changeSortBy(SortBy::name); });
  connect(sizeAction, &QAction::triggered, this,
          [this]() { emit changeSortBy(SortBy::size); });
  connect(dateModifiedAction, &QAction::triggered, this,
          [this]() { emit changeSortBy(SortBy::date_modified); });

  // By default, set "Ascending" as checked
  nameAction->setChecked(true);

  // Add actions to the "Sort" menu
  sortByMenu->addAction(nameAction);
  sortByMenu->addAction(sizeAction);
  sortByMenu->addAction(dateModifiedAction);
}

void MainWindow::openImage() {
  // Open a file dialog to select an image
  QString fileFilter = "Images (*.png *.jpg *.jpeg *.heic *.nef "
                       "*.tiff *.webp)";

  QString previousOpenPath =
      m_preferences->get(Preferences::SETTING_PREVIOUS_OPEN_PATH, "")
          .toString();

  QString imagePath = QFileDialog::getOpenFileName(
      this, "Open Image", previousOpenPath, fileFilter);

  if (!imagePath.isEmpty()) {
    // Emit a signal to load the image in a separate thread

    emit resetImageFilePaths();
    emit loadImage(imagePath);

    QFileInfo fileInfo(imagePath);

    /// Save the open path
    m_preferences->set(Preferences::SETTING_PREVIOUS_OPEN_PATH,
                       fileInfo.dir().absolutePath());
  }
}

void MainWindow::copyToClipboard() {
  emit copyCurrentImageFullResToClipboard();
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
      tr("Images (*.png *.jpg *.jpeg *.heic *.nef *.tiff *.webp);;All Files "
         "(*)"));
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

  setWindowTitle(fileInfo.fileName() +
                 QString(" (%1 x %2) [%3]")
                     .arg(imageInfo.width)
                     .arg(imageInfo.height)
                     .arg(prettyPrintSize(fileInfo.size())));
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