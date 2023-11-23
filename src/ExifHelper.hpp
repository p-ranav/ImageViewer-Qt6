#pragma once
#include "libexif/exif-data.h"
/// #include "libexif/exif-system.h"
#include "libexif/exif-loader.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static unsigned entry_count;

/** Callback function handling an ExifEntry. */
static inline void content_foreach_func(ExifEntry *entry, void *)
{
  char buf[2000];
  exif_entry_get_value(entry, buf, sizeof(buf));
  printf("    Entry %u: %s (%s) %s\n", 
	 entry_count,
	 exif_tag_get_name(entry->tag),
	 exif_format_get_name(entry->format),
	 exif_entry_get_value(entry, buf, sizeof(buf)));
  ++entry_count;
}


/** Callback function handling an ExifContent (corresponds 1:1 to an IFD). */
static inline void data_foreach_func(ExifContent *content, void *callback_data)
{
  static unsigned content_count;
  entry_count = 0;
  printf("  Content %u: ifd=%d\n", content_count, exif_content_get_ifd(content));
  exif_content_foreach_entry(content, content_foreach_func, callback_data);
  ++content_count;
}

static inline void dump_makernote(ExifData *d) {
  ExifMnoteData *mn = exif_data_get_mnote_data(d);
  if (mn) {
    char buf[2000];
    int i;
    int num = exif_mnote_data_count(mn);
    printf("  MakerNote\n");
    for (i=0; i < num; ++i) {
      if (exif_mnote_data_get_value(mn, i, buf, sizeof(buf))) {
	const char *name = exif_mnote_data_get_name(mn, i);
	/// unsigned int id = exif_mnote_data_get_id(mn, i);
	if (!name)
	    name = "(unknown)";
	printf("      %s - %s\n", name, buf);
      }
    }
  }
}

static inline void readExifData(const char *filePath) {
  ExifData *d;
  unsigned char *buf;
  unsigned int len;

  const char *fn = filePath;
  d = exif_data_new_from_file(fn);
  if (!d) {
    fprintf(stderr, "Could not load data from '%s'!\n", fn);
    return;
  }

  exif_data_save_data(d, &buf, &len);

  if (!buf) {
    fprintf(stderr, "Could not extract EXIF data!\n");
    return;
  }

  exif_data_foreach_content(d, data_foreach_func, nullptr);

  exif_data_unref(d);
}