#include "Preferences.hpp"

Preferences::Preferences(QWidget *parent) : QWidget(parent) { setupUi(); }

QVariant Preferences::get(QAnyStringView key, const QVariant &defaultValue) {
  return m_settings->value(key, defaultValue);
}

void Preferences::set(QAnyStringView key, const QVariant &value) {
  m_settings->setValue(key, value);
}

void Preferences::setupUi() {
  // Create a tab widget
  QTabWidget *tabWidget = new QTabWidget(this);

  // First tab
  QWidget *tab1 = setupViewTab();
  tabWidget->addTab(tab1, "General");

  // Second tab ("Slideshow")
  QWidget *tab2 = setupSlideshowTab();
  tabWidget->addTab(tab2, "Slideshow");

  // Third tab ("RAW")
  QWidget *tab3 = setupRawTab();
  tabWidget->addTab(tab3, "RAW");

  // Set up the layout
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->addWidget(tabWidget);
}

QWidget *Preferences::setupViewTab() {
  // Second tab ("Slideshow")
  QWidget *tab1 = new QWidget;
  QFormLayout *formLayout = new QFormLayout(tab1);

  QLabel *backgroundLabel = new QLabel("Background", this);
  m_colorPickerButton = new QPushButton(this);
  m_colorPickerButton->setFixedSize(30, 30);
  auto savedColor =
      get(SETTING_BACKGROUND_COLOR, QRect(25, 25, 25, 255)).toRect();
  auto r = savedColor.x();
  auto g = savedColor.y();
  auto b = savedColor.width();
  auto a = savedColor.height();
  QString styleSheet =
      QString("background-color: %1").arg(QColor(r, g, b, a).name());
  m_colorPickerButton->setStyleSheet(styleSheet);

  connect(m_colorPickerButton, &QPushButton::clicked, this, [this]() {
    auto savedColor =
        get(SETTING_BACKGROUND_COLOR, QRect(25, 25, 25, 255)).toRect();
    auto r = savedColor.x();
    auto g = savedColor.y();
    auto b = savedColor.width();
    auto a = savedColor.height();

    // Open color dialog and get selected color
    QColor color =
        QColorDialog::getColor(QColor(r, g, b, a), this, "Select Color",
                               QColorDialog::ShowAlphaChannel);

    // Set the background color of the main window
    if (color.isValid()) {
      /// do work
      QString styleSheet = QString("background-color: %1").arg(color.name());
      m_colorPickerButton->setStyleSheet(styleSheet);

      set(SETTING_BACKGROUND_COLOR,
          QRect(color.red(), color.green(), color.blue(), color.alpha()));

      /// Let the mainWindow know
      emit settingChangedBackgroundColor(color);
    }
  });

  formLayout->addRow(backgroundLabel, m_colorPickerButton);

  return tab1;
}

QWidget *Preferences::setupSlideshowTab() {
  // Second tab ("Slideshow")
  QWidget *tab2 = new QWidget;
  QFormLayout *formLayout = new QFormLayout(tab2);

  // Slideshow Period
  QLabel *slideshowLabel = new QLabel("Period (s)");
  m_slideshowPeriod = new QLineEdit;
  connect(m_slideshowPeriod, &QLineEdit::editingFinished, this,
          &Preferences::handleEditingFinished_slideshowPeriod);
  m_slideshowPeriod->setText(
      QString("%1").arg(get(SETTING_SLIDESHOW_PERIOD, 2500).toInt() / 1000.0));
  formLayout->addRow(slideshowLabel, m_slideshowPeriod);

  // Slideshow Loop after end checkbox
  m_slideshowLoop = new QCheckBox("Loop");
  if (get(SETTING_SLIDESHOW_LOOP, true).toBool()) {
    m_slideshowLoop->setChecked(true);
  } else {
    m_slideshowLoop->setChecked(false);
  }
  connect(m_slideshowLoop, &QCheckBox::stateChanged, this,
          &Preferences::handleEditingFinished_slideshowLoop);
  formLayout->addRow(m_slideshowLoop);

  return tab2;
}

QWidget *Preferences::setupRawTab() {
  QWidget *tab3 = new QWidget;
  QFormLayout *formLayout = new QFormLayout(tab3);

  // RAW half-size setting
  m_rawHalfSize = new QCheckBox("Load RAW images half size");
  if (get(SETTING_RAW_HALF_SIZE, true).toBool()) {
    m_rawHalfSize->setChecked(true);
  } else {
    m_rawHalfSize->setChecked(false);
  }
  connect(m_rawHalfSize, &QCheckBox::stateChanged, this,
          &Preferences::handleEditingFinished_halfSize);
  formLayout->addRow(m_rawHalfSize);

  // RAW auto-wb setting
  m_rawAutoWb = new QCheckBox("Use automatic white balance");
  if (get(SETTING_RAW_AUTO_WB, false).toBool()) {
    m_rawAutoWb->setChecked(true);
  } else {
    m_rawAutoWb->setChecked(false);
  }
  connect(m_rawAutoWb, &QCheckBox::stateChanged, this,
          &Preferences::handleEditingFinished_autoWb);
  formLayout->addRow(m_rawAutoWb);

  return tab3;
}

void Preferences::handleEditingFinished_slideshowPeriod() {
  m_slideshowPeriod->clearFocus();

  QString text = m_slideshowPeriod->text();
  bool ok;
  float newSlideshowPeriod = text.toFloat(&ok);

  if (newSlideshowPeriod < 1) {
    newSlideshowPeriod = 1; // at least 1s
  }

  m_slideshowPeriod->setText(QString("%1").arg(newSlideshowPeriod));

  if (ok) {
    // If the conversion to float was successful
    newSlideshowPeriod *= 1000;
    set(SETTING_SLIDESHOW_PERIOD, newSlideshowPeriod);
    emit settingChangedSlideShowPeriod();
    qDebug() << "Preferences::Set slideshow period to " << text << "s";
  } else {
    qDebug() << "Preferences::Invalid slideshow period";
  }
}

void Preferences::handleEditingFinished_slideshowLoop(int state) {
  if (state == Qt::Checked) {
    set(SETTING_SLIDESHOW_LOOP, true);
    qDebug() << "Preferences::Slideshow Loop: True";
  } else {
    set(SETTING_SLIDESHOW_LOOP, false);
    qDebug() << "Preferences::Slideshow Loop: False";
  }
}

void Preferences::handleEditingFinished_halfSize(int state) {
  if (state == Qt::Checked) {
    set(SETTING_RAW_HALF_SIZE, true);
    qDebug() << "Preferences::RAW Half size: True";
  } else {
    set(SETTING_RAW_HALF_SIZE, false);
    qDebug() << "Preferences::RAW Half size: False";
  }

  emit rawSettingChanged();
}

void Preferences::handleEditingFinished_autoWb(int state) {
  if (state == Qt::Checked) {
    set(SETTING_RAW_AUTO_WB, true);
    qDebug() << "Preferences::RAW auto wb: True";
  } else {
    set(SETTING_RAW_AUTO_WB, false);
    qDebug() << "Preferences::RAW auto wb: False";
  }

  emit rawSettingChanged();
}