#include "main_window.hpp"

MainWindow::MainWindow() : QMainWindow() {

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
  openAction->setShortcut(QKeySequence("Ctrl+O"));
  connect(openAction, &QAction::triggered, this, &MainWindow::openImage);

  // Create a "Copy to clipboard" action
  QAction *copyToClipboardAction = new QAction("Copy to clipboard", this);
  copyToClipboardAction->setShortcut(QKeySequence("Ctrl+C"));
  connect(copyToClipboardAction, &QAction::triggered, this,
          &MainWindow::copyToClipboard);

  // Create a "Delete" action
  QAction *deleteAction = new QAction("Delete", this);
  deleteAction->setShortcut(QKeySequence("Ctrl+D"));
  connect(deleteAction, &QAction::triggered, this,
          &MainWindow::confirmAndDeleteCurrentImage);

  // Create a "Quick Export as a PNG" action
  QAction *quickExportAction = new QAction("Quick Export as a PNG", this);
  connect(quickExportAction, &QAction::triggered, this,
          &MainWindow::quickExportAsPng);

  // Create an "Zoom In" action
  QAction *zoomInAction = new QAction("Zoom In", this);
  zoomInAction->setShortcut(QKeySequence("Ctrl++"));
  connect(zoomInAction, &QAction::triggered, this, &MainWindow::zoomIn);

  // Create an "Zoom Out" action
  QAction *zoomOutAction = new QAction("Zoom Out", this);
  zoomOutAction->setShortcut(QKeySequence("Ctrl+-"));
  connect(zoomOutAction, &QAction::triggered, this, &MainWindow::zoomOut);

  QAction *previousImageAction = new QAction("Previous Image", this);
  previousImageAction->setShortcut(QKeySequence("Ctrl+P"));
  connect(previousImageAction, &QAction::triggered, this,
          [this]() { emit previousImage(imageViewer->pixmap()); });

  QAction *nextImageAction = new QAction("Next Image", this);
  nextImageAction->setShortcut(QKeySequence("Ctrl+N"));
  connect(nextImageAction, &QAction::triggered, this,
          [this]() { emit nextImage(imageViewer->pixmap()); });

  // Create an "Zen mode" action
  QAction *zenModeAction = new QAction("Toggle Zen Mode", this);
  zenModeAction->setShortcut(QKeySequence("Ctrl+Z"));
  connect(zenModeAction, &QAction::triggered, this,
          &MainWindow::toggleFullScreen);

  // Add the "Open" action to the "File" menu
  fileMenu->addAction(openAction);
  fileMenu->addAction(copyToClipboardAction);
  fileMenu->addAction(deleteAction);
  fileMenu->addSeparator();
  fileMenu->addAction(quickExportAction);
  viewMenu->addAction(zoomInAction);
  viewMenu->addAction(zoomOutAction);
  viewMenu->addSeparator();
  viewMenu->addAction(previousImageAction);
  viewMenu->addAction(nextImageAction);
  viewMenu->addSeparator();
  viewMenu->addAction(zenModeAction);

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

  // Left Arrow (Previous Image button)
  m_leftArrowButton = new QPushButton(this);
  m_leftArrowButton->setFixedSize(40, 40);
  connect(m_leftArrowButton, &QPushButton::pressed,
          [this]() { emit previousImage(imageViewer->pixmap()); });
  m_leftArrowButton->setStyleSheet("background: transparent;");

  // Right Arrow (Next Image button)
  m_rightArrowButton = new QPushButton(this);
  m_rightArrowButton->setFixedSize(40, 40);
  connect(m_rightArrowButton, &QPushButton::pressed,
          [this]() { emit nextImage(imageViewer->pixmap()); });
  m_rightArrowButton->setStyleSheet("background: transparent;");

  // Create a layout and add buttons to it
  m_toolbarWidget = new QWidget(this);
  QHBoxLayout *buttonLayout = new QHBoxLayout();
  buttonLayout->addWidget(m_leftArrowButton);
  buttonLayout->addWidget(m_rightArrowButton);
  m_toolbarWidget->setLayout(buttonLayout);

  vstackLayout->addWidget(m_toolbarWidget, 0, Qt::AlignCenter);

  // Set the icon with a specific color
  QColor iconColor(Qt::white); // Set your desired color
  m_leftArrowButton->setIcon(
      createColorIcon(":/images/left_arrow.png", iconColor, 24));
  m_rightArrowButton->setIcon(
      createColorIcon(":/images/right_arrow.png", iconColor, 24));

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

void MainWindow::onImageLoaded(const QFileInfo &, const QPixmap &imagePixmap,
                               const ImageInfo &) {
  // Set the resized image to the QLabel
  imageViewer->setPixmap(imagePixmap, width() * getScaleFactor(),
                         height() * getScaleFactor());
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

void MainWindow::toggleFullScreen() {

  if (m_fullScreen) {
    m_fullScreen = false;

    imageViewer->setPixmap(imageViewer->pixmap(), width() * getScaleFactor(),
                           height() * getScaleFactor());

    showNormal();
    m_toolbarWidget->show();

  } else {
    m_fullScreen = true;
    imageViewer->setPixmap(imageViewer->pixmap(), width() * getScaleFactor(),
                           height() * getScaleFactor());

    m_toolbarWidget->hide();
    showFullScreen();
  }
}

qreal MainWindow::getScaleFactor() const {
  if (m_fullScreen) {
    return 1;
  } else {
    return SCALE_FACTOR;
  }
}