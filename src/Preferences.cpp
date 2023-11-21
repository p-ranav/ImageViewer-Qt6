#include "Preferences.hpp"

Preferences::Preferences(QWidget *parent) : QWidget(parent) {
  m_settings = new QSettings("p-ranav", "ImageViewer");
  setupUi();
}

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
  QWidget *tab1 = new QWidget;
  tabWidget->addTab(tab1, "Tab 1");

  // Second tab ("Slideshow")
  QWidget *tab2 = setupSlideshowTab();
  tabWidget->addTab(tab2, "Slideshow");

  // Third tab
  QWidget *tab3 = new QWidget;
  tabWidget->addTab(tab3, "Tab 3");

  // Set up the layout
  QVBoxLayout *layout = new QVBoxLayout(this);
  layout->addWidget(tabWidget);
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