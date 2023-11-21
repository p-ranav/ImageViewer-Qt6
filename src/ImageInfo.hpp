#pragma once

struct ImageInfo {
  int width;
  int height;
};

static inline QString prettyPrintSize(qint64 size) {
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