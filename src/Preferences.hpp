#pragma once
#include <QWidget>
#include <QTabWidget>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QSettings>
#include <QAnyStringView>
#include <QVariant>
#include <QCheckBox>

class Preferences : public QWidget {
    Q_OBJECT

    QSettings *m_settings;
    QLineEdit* m_slideshowPeriod;
    QCheckBox* m_slideshowLoop;

public:
    constexpr static inline char SETTING_SLIDESHOW_PERIOD[] = "slideShowTimerIntervalMs";
    constexpr static inline char SETTING_SLIDESHOW_LOOP[] = "slideShowLoopAfterEnd";

public:
    Preferences(QWidget *parent = nullptr);
    QVariant get(QAnyStringView key, const QVariant &defaultValue);
    void set(QAnyStringView key, const QVariant &value);

signals:
    void settingChangedSlideShowPeriod();
    void settingChangedSlideShowLoop();

private:
    void setupUi();
    void handleEditingFinished_slideshowPeriod();
    void handleEditingFinished_slideshowLoop(int state);
};
