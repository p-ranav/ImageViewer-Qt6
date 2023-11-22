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

    static inline QSettings *m_settings = new QSettings("p-ranav", "ImageViewer");
    QLineEdit* m_slideshowPeriod;
    QCheckBox* m_slideshowLoop;

    QCheckBox* m_rawHalfSize;
    QCheckBox* m_rawAutoWb;

public:
    constexpr static inline char SETTING_PREVIOUS_OPEN_PATH[] = "openPath";
    constexpr static inline char SETTING_SLIDESHOW_PERIOD[] = "slideShowTimerIntervalMs";
    constexpr static inline char SETTING_SLIDESHOW_LOOP[] = "slideShowLoopAfterEnd";
    constexpr static inline char SETTING_RAW_HALF_SIZE[] = "rawHalfSize";
    constexpr static inline char SETTING_RAW_AUTO_WB[] = "rawAutoWb";

public:
    Preferences(QWidget *parent = nullptr);
    static QVariant get(QAnyStringView key, const QVariant &defaultValue);
    static void set(QAnyStringView key, const QVariant &value);

signals:
    void settingChangedSlideShowPeriod();
    void settingChangedSlideShowLoop();
    void rawSettingChanged();

private:
    void setupUi();
    QWidget* setupSlideshowTab();
    QWidget* setupRawTab();
    void handleEditingFinished_slideshowPeriod();
    void handleEditingFinished_slideshowLoop(int state);
    void handleEditingFinished_halfSize(int state);
    void handleEditingFinished_autoWb(int state);
};
